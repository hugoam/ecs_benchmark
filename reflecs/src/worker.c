#include "include/private/reflecs.h"
#include <assert.h>
#include <math.h>

const EcsArrayParams thread_arr_params = {
    .element_size = sizeof(EcsThread)
};

const EcsArrayParams job_arr_params = {
    .element_size = sizeof(EcsJob)
};

/** Worker thread code. Processes a job for one system */
static
void* ecs_worker(void *arg) {
    EcsThread *thread = arg;
    EcsWorld *world = thread->world;
    int i;

    ut_mutex_lock(world->thread_mutex);
    world->threads_running ++;

    while (!world->quit_workers) {
        ut_cond_wait(world->thread_cond, world->thread_mutex);
        if (world->quit_workers) {
            break;
        }

        EcsJob **jobs = thread->jobs;
        uint32_t job_count = thread->job_count;
        ut_mutex_unlock(world->thread_mutex);

        for (i = 0; i < job_count; i ++) {
            ecs_run_job(world, thread, jobs[i]);
        }

        ut_mutex_lock(world->thread_mutex);
        thread->job_count = 0;

        ut_mutex_lock(world->job_mutex);
        world->jobs_finished ++;
        if (world->jobs_finished == world->threads_running) {
            ut_cond_signal(world->job_cond);
        }
        ut_mutex_unlock(world->job_mutex);
    }

    ut_mutex_unlock(world->thread_mutex);

    return NULL;
}

/** Wait until threads have started (busy loop) */
static
void wait_for_threads(
    EcsWorld *world)
{
    uint32_t thread_count = ecs_array_count(world->worker_threads) - 1;
    bool wait = true;

    do {
        ut_mutex_lock(world->thread_mutex);
        if (world->threads_running == thread_count) {
            wait = false;
        }
        ut_mutex_unlock(world->thread_mutex);
    } while (wait);
}

/** Wait until threads have finished processing their jobs */
static
void wait_for_jobs(
    EcsWorld *world)
{
    uint32_t thread_count = ecs_array_count(world->worker_threads) - 1;

    ut_mutex_lock(world->job_mutex);
    if (world->jobs_finished != thread_count) {
        do {
            ut_cond_wait(world->job_cond, world->job_mutex);
        } while (world->jobs_finished != thread_count);
    }
    ut_mutex_unlock(world->job_mutex);
}

/** Stop worker threads */
static
void ecs_stop_threads(
    EcsWorld *world)
{
    ut_mutex_lock(world->thread_mutex);
    world->quit_workers = true;
    ut_cond_signal(world->thread_cond);
    ut_mutex_unlock(world->thread_mutex);

    EcsThread *buffer = ecs_array_buffer(world->worker_threads);
    uint32_t i, count = ecs_array_count(world->worker_threads);
    for (i = 1; i < count; i ++) {
        ut_thread_join(buffer[i].thread, NULL);
        ecs_stage_deinit(world, buffer[i].stage);
    }

    ecs_array_free(world->worker_threads);
    ecs_array_free(world->worker_stages);
    world->worker_stages = NULL;
    world->worker_threads = NULL;
    world->quit_workers = false;
    world->threads_running = 0;
}

/** Start worker threads, wait until they are running */
static
EcsResult start_threads(
    EcsWorld *world,
    uint32_t threads)
{
    if (world->worker_threads) {
        return EcsError;
    }

    world->worker_threads = ecs_array_new(&thread_arr_params, threads);
    world->worker_stages = ecs_array_new(&stage_arr_params, threads - 1);

    int i;
    for (i = 0; i < threads; i ++) {
        EcsThread *thread =
            ecs_array_add(&world->worker_threads, &thread_arr_params);

        thread->magic = ECS_THREAD_MAGIC;
        thread->world = world;
		//thread->thread = { NULL };
        thread->job_count = 0;

        if (i != 0) {
            thread->stage = ecs_array_add(&world->worker_stages, &stage_arr_params);
            ecs_stage_init(world, thread->stage);
            if (thread->thread = ut_thread_new(ecs_worker, thread)) {
                goto error;
            }
        } else {
            thread->stage = NULL;
        }
    }

    return EcsOk;
error:
    ecs_stop_threads(world);
    return EcsError;
}

/** Create jobs for system */
static
void create_jobs(
    EcsColSystem *system_data,
    uint32_t thread_count)
{
    if (system_data->jobs) {
        ecs_array_free(system_data->jobs);
    }

    system_data->jobs = ecs_array_new(&job_arr_params, thread_count);

    int i;
    for (i = 0; i < thread_count; i ++) {
        ecs_array_add(&system_data->jobs, &job_arr_params);
    }
}


/* -- Private functions -- */

/** Create a job per available thread for system */
void ecs_schedule_jobs(
    EcsWorld *world,
    EcsEntity system)
{
    EcsColSystem *system_data = ecs_get_ptr(world, system, EcsColSystem);
    uint32_t thread_count = ecs_array_count(world->worker_threads);
    uint32_t total_rows = 0;

    EcsIter table_it = ecs_array_iter(
        system_data->tables, &system_data->table_params);

    while (ecs_iter_hasnext(&table_it)) {
        uint32_t table_index = *(uint32_t*)ecs_iter_next(&table_it);
        EcsTable *table = ecs_array_get(
            world->main_stage.tables, &table_arr_params, table_index);
        total_rows += ecs_array_count(table->columns[0].data);
    }

    if (total_rows < thread_count) {
        thread_count = total_rows;
    }

    if (ecs_array_count(system_data->jobs) != thread_count) {
        create_jobs(system_data, thread_count);
    }

    float rows_per_thread = (float)total_rows / (float)thread_count;
    float residual = 0;
    int32_t rows_per_thread_i = rows_per_thread;

    table_it = ecs_array_iter(system_data->tables, &system_data->table_params);

    uint32_t sys_table_index = 0;
    uint32_t table_index = *(uint32_t*)ecs_array_get(
        system_data->tables, &system_data->table_params, 0);
    EcsTable *table = ecs_array_get(
        world->main_stage.tables, &table_arr_params, table_index);
    uint32_t table_row_count = ecs_array_count(table->columns[0].data);
    uint32_t start_index = 0;

    EcsJob *job = NULL;
    uint32_t i;
    for (i = 0; i < thread_count; i ++) {
        job = ecs_array_get(system_data->jobs, &job_arr_params, i);
        int32_t rows_per_job = rows_per_thread_i;
        residual += rows_per_thread - rows_per_job;
        if (residual > 1) {
            rows_per_job ++;
            residual --;
        }

        job->system = system;
        job->system_data = system_data;
        job->table_index = sys_table_index;
        job->start_index = start_index;
        job->row_count = rows_per_job;

        start_index += rows_per_job;

        while (start_index > table_row_count) {
            sys_table_index ++;
            table_index = *(uint32_t*)ecs_array_get(
                system_data->tables, &system_data->table_params, sys_table_index);
            table = ecs_array_get(
                world->main_stage.tables, &table_arr_params, table_index);
            table_row_count = ecs_array_count(table->columns[0].data);
            if (start_index > table_row_count) {
                start_index -= table_row_count;
            } else {
                start_index = 0;
            }
        }
    }

    if (residual >= 0.9) {
        job->row_count ++;
    }
}

/** Assign jobs to worker threads, signal workers */
void ecs_prepare_jobs(
    EcsWorld *world,
    EcsEntity system)
{
    EcsColSystem *system_data = ecs_get_ptr(world, system, EcsColSystem);
    EcsArray *threads = world->worker_threads;
    EcsArray *jobs = system_data->jobs;
    int i;

    uint32_t thread_count = ecs_array_count(jobs);

    for (i = 0; i < thread_count; i++) {
        EcsThread *thr = ecs_array_get(threads, &thread_arr_params, i);
        uint32_t job_count = thr->job_count;
        thr->jobs[job_count] = ecs_array_get(jobs, &job_arr_params, i);
        thr->job_count = job_count + 1;
    }
}

void ecs_run_jobs(
    EcsWorld *world)
{
    /* Make sure threads are ready to accept jobs */
    wait_for_threads(world);

    ut_mutex_lock(world->thread_mutex);
    world->jobs_finished = 0;
    ut_cond_signal(world->thread_cond);
    ut_mutex_unlock(world->thread_mutex);

    /* Run job for thread 0 in main thread */
    EcsThread *thread = ecs_array_buffer(world->worker_threads);
    EcsJob **jobs = thread->jobs;
    uint32_t i, job_count = thread->job_count;

    for (i = 0; i < job_count; i ++) {
        ecs_run_job(world, NULL, jobs[i]);
    }
    thread->job_count = 0;

    if (world->jobs_finished != ecs_array_count(world->worker_threads) - 1) {
        wait_for_jobs(world);
    }
}


/* -- Public functions -- */

EcsResult ecs_set_threads(
    EcsWorld *world,
    uint32_t threads)
{
    if (ecs_array_count(world->worker_threads)) {
        ecs_stop_threads(world);
        ut_cond_free(world->thread_cond);
        ut_mutex_free(world->thread_mutex);
        ut_cond_free(world->job_cond);
        ut_mutex_free(world->job_mutex);
    }

    if (threads > 1) {
        ut_cond_new(world->thread_cond);
        ut_mutex_new(world->thread_mutex);
        ut_cond_new(world->job_cond);
        ut_mutex_new(world->job_mutex);
        if (start_threads(world, threads) != EcsOk) {
            return EcsError;
        }
    }

    world->valid_schedule = false;

    return EcsOk;
}
