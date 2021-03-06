#include <include/ecs_benchmark.h>

double iterate(EcsWorld *world, int n_iter) {
    double result = 0;

    for (int i = 0; i < N_ITERATIONS; i ++) {
        struct timespec start; timespec_gettime(&start);
        
        /* Pass constant time, so reflecs won't request time */
        ecs_progress(world, 1); 
        
        double t = timespec_measure(&start);
        if (!result || t < result) {
            result = t;
        }
    }

    return result;
}


double bench_create_empty_reflecs(int n) {
    EcsWorld *world = ecs_init();

    struct timespec start; timespec_gettime(&start);
    for (int i = 0; i < n; i ++) {
        ecs_new(world, 0);
    }
    double result = timespec_measure(&start);

    ecs_fini(world);

    return result;
}

double bench_create_empty_reflecs_batch(int n) {
    EcsWorld *world = ecs_init();

    struct timespec start; timespec_gettime(&start);
    ecs_new_w_count(world, 0, n, NULL);
    double result = timespec_measure(&start);

    ecs_fini(world);

    return result;
}

double bench_create_1component_reflecs_batch(int n) {
    EcsWorld *world = ecs_init();

    ECS_COMPONENT(world, Position);

    struct timespec start; timespec_gettime(&start);
    ecs_new_w_count(world, Position, n, NULL);
    double result = timespec_measure(&start);

    ecs_fini(world);

    return result;
}

double bench_create_2component_reflecs_batch(int n) {
    EcsWorld *world = ecs_init();

    ECS_COMPONENT(world, Position);
    ECS_COMPONENT(world, Velocity);
    ECS_TYPE(world, Movable, Position, Velocity);

    struct timespec start; timespec_gettime(&start);
    ecs_new_w_count(world, Movable, n, NULL);
    double result = timespec_measure(&start);

    ecs_fini(world);

    return result;
}

double bench_create_3component_reflecs_batch(int n) {
    EcsWorld *world = ecs_init();

    ECS_COMPONENT(world, Position);
    ECS_COMPONENT(world, Velocity);
    ECS_COMPONENT(world, Mass);
    ECS_TYPE(world, Movable, Position, Velocity, Mass);

    struct timespec start; timespec_gettime(&start);
    ecs_new_w_count(world, Movable, n, NULL);
    double result = timespec_measure(&start);

    ecs_fini(world);

    return result;
}

double bench_delete_1component_reflecs(int n) {
    EcsWorld *world = ecs_init();

    ECS_COMPONENT(world, Position);

    EcsEntity *entities = new EcsEntity[n];
    ecs_new_w_count(world, Position, n, entities);

    struct timespec start; timespec_gettime(&start);
    for (int i = 0; i < n; i ++) {
        ecs_delete(world, entities[i]);
    }
    double result = timespec_measure(&start);

    delete[] entities;

    ecs_fini(world);

    return result;
}

double bench_add_one_reflecs(int n) {
    EcsWorld *world = ecs_init();

    ECS_COMPONENT(world, Position);

    EcsEntity *entities = new EcsEntity[n];
    ecs_new_w_count(world, 0, n, entities);

    struct timespec start; timespec_gettime(&start);
    for (int i = 0; i < n; i ++) {
        ecs_add(world, entities[i], Position);
    }
    double result = timespec_measure(&start);

    delete[] entities;

    ecs_fini(world);

    return result;
}

double bench_add_one_reflecs_new(int n) {
    EcsWorld *world = ecs_init();

    ECS_COMPONENT(world, Position);

    struct timespec start; timespec_gettime(&start);
    for (int i = 0; i < n; i ++) {
        ecs_new(world, Position);
    }
    double result = timespec_measure(&start);

    ecs_fini(world);

    return result;
}

double bench_add_two_reflecs(int n) {
    EcsWorld *world = ecs_init();

    ECS_COMPONENT(world, Position);
    ECS_COMPONENT(world, Velocity);
    ECS_TYPE(world, Type, Position, Velocity);
    ecs_dim(world, n);
    ecs_dim_type(world, Type, n);

    EcsEntity *entities = new EcsEntity[n];
    ecs_new_w_count(world, 0, n, entities);

    struct timespec start; timespec_gettime(&start);
    for (int i = 0; i < n; i ++) {
        ecs_add(world, entities[i], Position);
        ecs_add(world, entities[i], Velocity);
    }
    double result = timespec_measure(&start);

    ecs_fini(world);

    return result;
}

double bench_add_two_reflecs_family(int n) {
    EcsWorld *world = ecs_init();

    ECS_COMPONENT(world, Position);
    ECS_COMPONENT(world, Velocity);
    ECS_TYPE(world, Type, Position, Velocity);

    EcsEntity *entities = new EcsEntity[n];
    ecs_new_w_count(world, 0, n, entities);

    struct timespec start; timespec_gettime(&start);
    for (int i = 0; i < n; i ++) {
        ecs_add(world, entities[i], Type);
    }
    double result = timespec_measure(&start);

    delete[] entities;

    ecs_fini(world);

    return result;
}

double bench_add_three_reflecs(int n) {
    EcsWorld *world = ecs_init();

    ECS_COMPONENT(world, Position);
    ECS_COMPONENT(world, Velocity);
    ECS_COMPONENT(world, Mass);

    EcsEntity *entities = new EcsEntity[n];
    ecs_new_w_count(world, 0, n, entities);

    struct timespec start; timespec_gettime(&start);
    for (int i = 0; i < n; i ++) {
        ecs_add(world, entities[i], Position);
        ecs_add(world, entities[i], Velocity);
        ecs_add(world, entities[i], Mass);
    }
    double result = timespec_measure(&start);

    delete[] entities;

    ecs_fini(world);

    return result;
}

double bench_add_three_reflecs_family(int n) {
    EcsWorld *world = ecs_init();

    ECS_COMPONENT(world, Position);
    ECS_COMPONENT(world, Velocity);
    ECS_COMPONENT(world, Mass);
    ECS_TYPE(world, Type, Position, Velocity, Mass);

    EcsEntity *entities = new EcsEntity[n];
    ecs_new_w_count(world, 0, n, entities);

    struct timespec start; timespec_gettime(&start);
    for (int i = 0; i < n; i ++) {
        ecs_add(world, entities[i], Type);
    }
    double result = timespec_measure(&start);

    delete[] entities;

    ecs_fini(world);

    return result;
}

double bench_add_four_reflecs(int n) {
    EcsWorld *world = ecs_init();

    ECS_COMPONENT(world, Position);
    ECS_COMPONENT(world, Velocity);
    ECS_COMPONENT(world, Mass);
    ECS_COMPONENT(world, Color);

    EcsEntity *entities = new EcsEntity[n];
    ecs_new_w_count(world, 0, n, entities);

    struct timespec start; timespec_gettime(&start);
    for (int i = 0; i < n; i ++) {
        ecs_add(world, entities[i], Position);
        ecs_add(world, entities[i], Velocity);
        ecs_add(world, entities[i], Mass);
        ecs_add(world, entities[i], Color);
    }
    double result = timespec_measure(&start);

    ecs_fini(world);

    return result;
}

double bench_add_four_reflecs_family(int n) {
    EcsWorld *world = ecs_init();

    ECS_COMPONENT(world, Position);
    ECS_COMPONENT(world, Velocity);
    ECS_COMPONENT(world, Mass);
    ECS_COMPONENT(world, Color);
    ECS_TYPE(world, Type, Position, Velocity, Mass, Color);

    EcsEntity *entities = new EcsEntity[n];
    ecs_new_w_count(world, 0, n, entities);

    struct timespec start; timespec_gettime(&start);
    for (int i = 0; i < n; i ++) {
        ecs_add(world, entities[i], Type);
    }
    double result = timespec_measure(&start);

    delete[] entities;

    ecs_fini(world);

    return result;
}


double bench_remove_one_reflecs(int n) {
    EcsWorld *world = ecs_init();

    ECS_COMPONENT(world, Position);

    EcsEntity *entities = new EcsEntity[n];
    ecs_new_w_count(world, Position, n, entities);

    struct timespec start; timespec_gettime(&start);
    for (int i = 0; i < n; i ++) {
        ecs_remove(world, entities[i], Position);
    }
    double result = timespec_measure(&start);

    ecs_fini(world);

    return result;
}

double bench_remove_two_reflecs(int n) {
    EcsWorld *world = ecs_init();

    ECS_COMPONENT(world, Position);
    ECS_COMPONENT(world, Velocity);
    ECS_TYPE(world, Type, Position, Velocity);

    EcsEntity *entities = new EcsEntity[n];
    ecs_new_w_count(world, Type, n, entities);

    struct timespec start; timespec_gettime(&start);
    for (int i = 0; i < n; i ++) {
        ecs_remove(world, entities[i], Position);
        ecs_remove(world, entities[i], Velocity);
    }
    double result = timespec_measure(&start);

    ecs_fini(world);

    return result;
}

double bench_remove_two_reflecs_family(int n) {
    EcsWorld *world = ecs_init();

    ECS_COMPONENT(world, Position);
    ECS_COMPONENT(world, Velocity);
    ECS_TYPE(world, Type, Position, Velocity);

    EcsEntity *entities = new EcsEntity[n];
    ecs_new_w_count(world, Type, n, entities);

    struct timespec start; timespec_gettime(&start);
    for (int i = 0; i < n; i ++) {
        ecs_remove(world, entities[i], Type);
    }
    double result = timespec_measure(&start);

    ecs_fini(world);

    return result;
}

double bench_remove_three_reflecs(int n) {
    EcsWorld *world = ecs_init();

    ECS_COMPONENT(world, Position);
    ECS_COMPONENT(world, Velocity);
    ECS_COMPONENT(world, Mass);
    ECS_TYPE(world, Type, Position, Velocity, Mass);

    EcsEntity *entities = new EcsEntity[n];
    ecs_new_w_count(world, Type, n, entities);

    struct timespec start; timespec_gettime(&start);
    for (int i = 0; i < n; i ++) {
        ecs_remove(world, entities[i], Position);
        ecs_remove(world, entities[i], Velocity);
        ecs_remove(world, entities[i], Mass);
    }
    double result = timespec_measure(&start);

    ecs_fini(world);

    return result;
}

double bench_remove_three_reflecs_family(int n) {
    EcsWorld *world = ecs_init();

    ECS_COMPONENT(world, Position);
    ECS_COMPONENT(world, Velocity);
    ECS_COMPONENT(world, Mass);
    ECS_TYPE(world, Type, Position, Velocity, Mass);

    EcsEntity *entities = new EcsEntity[n];
    ecs_new_w_count(world, Type, n, entities);

    struct timespec start; timespec_gettime(&start);
    for (int i = 0; i < n; i ++) {
        ecs_remove(world, entities[i], Type);
    }
    double result = timespec_measure(&start);

    ecs_fini(world);

    return result;
}


/* -- ITERATION -- */

void IterOne(EcsRows *rows) {
    int count = rows->limit;
    Position *p = ecs_column(rows, Position, 1);

    for (int i = 0; i < count; i ++) {
        p[i].x ++;
        p[i].y ++;
    }
}

void IterTwo(EcsRows *rows) {
    int count = rows->limit;
    Position *p = ecs_column(rows, Position, 1);
    Velocity *v = ecs_column(rows, Velocity, 2);

    for (int i = 0; i < count; i ++) {
        p[i].x += v[i].x;
        p[i].y += v[i].y;
    }
}

void IterThree(EcsRows *rows) {
    int count = rows->limit;
    Position *p = ecs_column(rows, Position, 1);
    Velocity *v = ecs_column(rows, Velocity, 2);
    Mass *m = ecs_column(rows, Mass, 3);

    for (int i = 0; i < count; i ++) {
        p[i].x += v[i].x / m[i].value;
        p[i].y += v[i].y / m[i].value;
    }
}

void IterFour(EcsRows *rows) {
    int count = rows->limit;
    Position *p = ecs_column(rows, Position, 1);
    Velocity *v = ecs_column(rows, Velocity, 2);
    Mass *m = ecs_column(rows, Mass, 3);
    Damage *d = ecs_column(rows, Damage, 4);

    for (int i = 0; i < count; i ++) {
        p[i].x += v[i].x / m[i].value / d[i].value;
        p[i].y += v[i].y / m[i].value / d[i].value;
    }
}

double bench_iter_one_reflecs(int n, int n_iter) {
    EcsWorld *world = ecs_init();

    ECS_COMPONENT(world, Position);
    ECS_SYSTEM(world, IterOne, EcsOnFrame, Position);

    ecs_new_w_count(world, Position, n, NULL);

    double result = iterate(world, n_iter);

    ecs_fini(world);

    return result;
}

double bench_iter_two_reflecs(int n, int n_iter) {
    EcsWorld *world = ecs_init();

    ECS_COMPONENT(world, Position);
    ECS_COMPONENT(world, Velocity);
    ECS_TYPE(world, Movable, Position, Velocity);
    ECS_SYSTEM(world, IterTwo, EcsOnFrame, Position, Velocity);

    ecs_new_w_count(world, Movable, n, NULL);

    double result = iterate(world, n_iter);

    ecs_fini(world);

    return result;
}

double bench_iter_three_two_types_reflecs(int n, int n_iter) {
    EcsWorld *world = ecs_init();

    ECS_COMPONENT(world, Position);
    ECS_COMPONENT(world, Velocity);
    ECS_COMPONENT(world, Rotation);
    ECS_COMPONENT(world, Mass);
    ECS_TYPE(world, TypeOne, Position, Velocity, Mass);
    ECS_TYPE(world, TypeTwo, Position, Velocity, Mass, Rotation);
    ECS_SYSTEM(world, IterThree, EcsOnFrame, Position, Velocity, Mass);

    ecs_new_w_count(world, TypeOne, n / 2, NULL);
    ecs_new_w_count(world, TypeTwo, n / 2, NULL);

    double result = iterate(world, n_iter);

    ecs_fini(world);

    return result;
}

double bench_iter_three_reflecs(int n, int n_iter) {
    EcsWorld *world = ecs_init();

    ECS_COMPONENT(world, Position);
    ECS_COMPONENT(world, Velocity);
    ECS_COMPONENT(world, Mass);
    ECS_TYPE(world, Movable, Position, Velocity, Mass);
    ECS_SYSTEM(world, IterThree, EcsOnFrame, Position, Velocity, Mass);

    ecs_new_w_count(world, Movable, n, NULL);

    double result = iterate(world, n_iter);

    ecs_fini(world);

    return result;
}

void create_eight_types(EcsWorld *world, int count, bool match_all) {
    ECS_COMPONENT(world, Position);
    ECS_COMPONENT(world, Velocity);
    ECS_COMPONENT(world, Rotation);
    ECS_COMPONENT(world, Mass);
    ECS_COMPONENT(world, Color);
    ECS_COMPONENT(world, Battery);
    ECS_COMPONENT(world, Damage);

    for(uint64_t i = 0; i < count; i++) {
        EcsEntity e = ecs_new(world, 0);
        ecs_add(world, e, Position);
        ecs_add(world, e, Mass);
        ecs_add(world, e, Damage);
        if ((i % 8) == 1) {
            ecs_add(world, e, Rotation);
        } else 
        if ((i % 8) == 2) {
            ecs_add(world, e, Color);
        } else 
        if ((i % 8) == 3) {
            ecs_add(world, e, Battery);
        } else 
        if ((i % 8) == 4) {
            ecs_add(world, e, Rotation);
            ecs_add(world, e, Color);
        } else 
        if ((i % 8) == 5) {
            ecs_add(world, e, Rotation);
            ecs_add(world, e, Battery);
        } else 
        if ((i % 8) == 6) {
            ecs_add(world, e, Battery);
            ecs_add(world, e, Color);
        } else 
        if ((i % 8) == 7) {
            ecs_add(world, e, Rotation);
            ecs_add(world, e, Battery);
            ecs_add(world, e, Color);
        }

        if (match_all || i % 2) {
            ecs_add(world, e, Velocity);
        }
    }
}

double bench_iter_two_eight_types_reflecs(int n, int n_iter, bool match_all) {
    EcsWorld *world = ecs_init();

    ECS_COMPONENT(world, Position);
    ECS_COMPONENT(world, Velocity);
    ECS_COMPONENT(world, Rotation);
    ECS_COMPONENT(world, Mass);
    ECS_COMPONENT(world, Color);
    ECS_COMPONENT(world, Battery);

    int count = n * (2 - match_all);

    create_eight_types(world, count, match_all);

    ECS_SYSTEM(world, IterTwo, EcsOnFrame, Position, Velocity);

    double result = iterate(world, n_iter);

    ecs_fini(world);

    return result;
}

double bench_iter_three_eight_types_reflecs(int n, int n_iter, bool match_all) {
    EcsWorld *world = ecs_init();

    ECS_COMPONENT(world, Position);
    ECS_COMPONENT(world, Velocity);
    ECS_COMPONENT(world, Rotation);
    ECS_COMPONENT(world, Mass);
    ECS_COMPONENT(world, Color);
    ECS_COMPONENT(world, Battery);

    int count = n * (2 - match_all);

    create_eight_types(world, count, match_all);

    ECS_SYSTEM(world, IterThree, EcsOnFrame, Position, Velocity, Mass);

    double result = iterate(world, n_iter);

    ecs_fini(world);

    return result;
}

double bench_iter_four_eight_types_reflecs(int n, int n_iter, bool match_all) {
    EcsWorld *world = ecs_init();

    ECS_COMPONENT(world, Position);
    ECS_COMPONENT(world, Velocity);
    ECS_COMPONENT(world, Rotation);
    ECS_COMPONENT(world, Mass);
    ECS_COMPONENT(world, Color);
    ECS_COMPONENT(world, Battery);
    ECS_COMPONENT(world, Damage);

    int count = n * (2 - match_all);

    create_eight_types(world, count, match_all);

    ECS_SYSTEM(world, IterFour, EcsOnFrame, Position, Velocity, Mass, Damage);

    double result = iterate(world, n_iter);

    ecs_fini(world);

    return result;
}

void create_pathological(EcsWorld *world, std::vector< std::vector <int>> entity_list) {
    ECS_COMPONENT(world, Position);
    ECS_COMPONENT(world, Velocity);
    ECS_COMPONENT(world, Rotation);
    ECS_COMPONENT(world, Mass);
    ECS_COMPONENT(world, Color);
    ECS_COMPONENT(world, Battery);
    ECS_COMPONENT(world, Damage);
    ECS_COMPONENT(world, Health);
    ECS_COMPONENT(world, Attack);
    ECS_COMPONENT(world, Defense);
    ECS_COMPONENT(world, Stamina);
    ECS_COMPONENT(world, Strength);
    ECS_COMPONENT(world, Agility);
    ECS_COMPONENT(world, Intelligence);

    for (std::vector<int> &component_list: entity_list) {
        EcsEntity e = ecs_new(world, 0);
        ecs_add(world, e, Position);
        ecs_add(world, e, Velocity);
        ecs_add(world, e, Mass);
        ecs_add(world, e, Damage);

        for (int c: component_list) {
            switch(c) {
            case 0:
                ecs_add(world, e, Stamina);
                break;
            case 1:
                ecs_add(world, e, Strength);
                break;
            case 2:
                ecs_add(world, e, Agility);
                break;
            case 3:
                ecs_add(world, e, Intelligence);
                break;
            case 4:
                ecs_add(world, e, Color);
                break;
            case 5:
                ecs_add(world, e, Battery);
                break;
            case 6:
                ecs_add(world, e, Rotation);
                break;
            case 7:
                ecs_add(world, e, Health);
                break;
            case 8:
                ecs_add(world, e, Attack);
                break;
            case 9:
                ecs_add(world, e, Defense);
                break;
            }
        }
    }
}

double bench_iter_one_pathological_reflecs(int n_iter, std::vector< std::vector<int>> entity_list) {
    EcsWorld *world = ecs_init();

    create_pathological(world, entity_list);

    ECS_SYSTEM(world, IterOne, EcsOnFrame, Position);

    double result = iterate(world, n_iter);

    ecs_fini(world);

    return result;
}

double bench_iter_two_pathological_reflecs(int n_iter, std::vector< std::vector<int>> entity_list) {
    EcsWorld *world = ecs_init();

    create_pathological(world, entity_list);

    ECS_SYSTEM(world, IterTwo, EcsOnFrame, Position, Velocity);

    double result = iterate(world, n_iter);

    ecs_fini(world);

    return result;
}

double bench_iter_three_pathological_reflecs(int n_iter, std::vector< std::vector<int>> entity_list) {
    EcsWorld *world = ecs_init();

    create_pathological(world, entity_list);

    ECS_SYSTEM(world, IterThree, EcsOnFrame, Position, Velocity, Mass);

    double result = iterate(world, n_iter);

    ecs_fini(world);

    return result;
}

double bench_iter_four_pathological_reflecs(int n_iter, std::vector< std::vector<int>> entity_list) {
    EcsWorld *world = ecs_init();

    create_pathological(world, entity_list);

    ECS_SYSTEM(world, IterFour, EcsOnFrame, Position, Velocity, Mass, Damage);

    double result = iterate(world, n_iter);

    ecs_fini(world);

    return result;
}
