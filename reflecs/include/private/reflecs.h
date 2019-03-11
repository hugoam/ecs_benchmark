#ifndef REFLECS_PRIVATE_H
#define REFLECS_PRIVATE_H

/* This file contains declarations to private reflecs functions */

#include "types.h"

/* -- Entity API -- */

/* Merge entity with stage */
void ecs_merge_entity(
    EcsWorld *world,
    EcsStage *stage,
    EcsEntity entity,
    EcsRow *staged_row);


/* Notify row system of entity (identified by row_index) */
bool ecs_notify(
    EcsWorld *world,
    EcsStage *stage,
    EcsMap *systems,
    EcsType type_id,
    EcsTable *table,
    EcsTableColumn *table_columns,
    int32_t offset,
    int32_t limit);

/* -- World API -- */

/* Get (or create) table from type */
EcsTable* ecs_world_get_table(
    EcsWorld *world,
    EcsStage *stage,
    EcsType type_id);

/* Activate system (move from inactive array to on_frame array or vice versa) */
void ecs_world_activate_system(
    EcsWorld *world,
    EcsEntity system,
    EcsSystemKind kind,
    bool active);

/* Get current thread-specific stage */
EcsStage *ecs_get_stage(
    EcsWorld **world_ptr);

/* -- Stage API -- */

/* Initialize stage data structures */
void ecs_stage_init(
    EcsWorld *world,
    EcsStage *stage);

/* Deinitialize stage */
void ecs_stage_deinit(
    EcsWorld *world,
    EcsStage *stage);

/* Merge stage with main stage */
void ecs_stage_merge(
    EcsWorld *world,
    EcsStage *stage);

/* -- Type utility API -- */

/* Get type from entity handle (component, type, prefab) */
EcsType ecs_type_from_handle(
    EcsWorld *world,
    EcsStage *stage,
    EcsEntity entity,
    EcsEntityInfo *info);

/* Merge add/remove families */
EcsType ecs_type_merge(
    EcsWorld *world,
    EcsStage *stage,
    EcsType cur_id,
    EcsType to_add_id,
    EcsType to_remove_id);

/* Merge add/remove families using arrays */
EcsType ecs_type_merge_arr(
    EcsWorld *world,
    EcsStage *stage,
    EcsArray *arr_cur,
    EcsArray *to_add,
    EcsArray *to_del);

/* Test if type_id_1 contains type_id_2 */
EcsEntity ecs_type_contains(
    EcsWorld *world,
    EcsStage *stage,
    EcsType type_id_1,
    EcsType type_id_2,
    bool match_all,
    bool match_prefab);

/* Test if type contains component */
bool ecs_type_contains_component(
    EcsWorld *world,
    EcsStage *stage,
    EcsType type,
    EcsEntity component,
    bool match_prefab);

/* Register new type from either a single component, an array of component
 * handles, or a combination */
EcsType ecs_type_register(
    EcsWorld *world,
    EcsStage *stage,
    EcsEntity to_add,
    EcsArray *set);

/* Add component to type */
EcsType ecs_type_add(
    EcsWorld *world,
    EcsStage *stage,
    EcsType type,
    EcsEntity component);

/* Get array with component handles from type */
EcsArray* ecs_type_get(
    EcsWorld *world,
    EcsStage *stage,
    EcsType type_id);

/* Convert type to string */
char* ecs_type_tostr(
    EcsWorld *world,
    EcsStage *stage,
    EcsType type_id);

/* Get index for entity in type */
int16_t ecs_type_index_of(
    EcsArray *type,
    EcsEntity component);

/* -- Table API -- */

/* Initialize table */
EcsResult ecs_table_init(
    EcsWorld *world,
    EcsStage *stage,
    EcsTable *table);

/* Initialize table with component size (used during bootstrap) */
EcsResult ecs_table_init_w_size(
    EcsWorld *world,
    EcsTable *table,
    EcsArray *type,
    uint32_t size);

/* Insert row into table (or stage) */
uint32_t ecs_table_insert(
    EcsWorld *world,
    EcsTable *table,
    EcsTableColumn *columns,
    EcsEntity entity);

/* Insert multiple rows into table (or stage) */
uint32_t ecs_table_grow(
    EcsWorld *world,
    EcsTable *table,
    EcsTableColumn *columns,
    uint32_t count,
    EcsEntity first_entity);

/* Dimension array to have n rows (doesn't add entities) */
int16_t ecs_table_dim(
    EcsTable *table,
    uint32_t count);

/* Return number of entities in table */
uint64_t ecs_table_count(
    EcsTable *table);

/* Return size of table row */
uint32_t ecs_table_row_size(
    EcsTable *table);

/* Return size of table row */
uint32_t ecs_table_rows_dimensioned(
    EcsTable *table);    

/* Delete row from table */
void ecs_table_delete(
    EcsWorld *world,
    EcsTable *table,
    uint32_t index);

/* Get row from table (or stage) */
void* ecs_table_get(
    EcsTable *table,
    EcsArray *rows,
    uint32_t index);

/* Test if table has component */
bool ecs_table_has_components(
    EcsTable *table,
    EcsArray *components);

/* Deinitialize table. This invokes all matching on_remove systems */
void ecs_table_deinit(
    EcsWorld *world,
    EcsTable *table);

/* Free table */
void ecs_table_free(
    EcsWorld *world,
    EcsTable *table);

/* -- System API -- */

/* Compute the AND type from the system columns */
void ecs_system_compute_and_families(
    EcsWorld *world,
    EcsEntity system,
    EcsSystem *system_data);

/* Create new table system */
EcsEntity ecs_new_col_system(
    EcsWorld *world,
    const char *id,
    EcsSystemKind kind,
    const char *sig,
    EcsSystemAction action);

/* Notify column system of a new table, which initiates system-table matching */
void ecs_col_system_notify_of_table(
    EcsWorld *world,
    EcsEntity system,
    EcsTable *table);

/* Notify row system of a new type, which initiates system-type matching */
void ecs_row_system_notify_of_type(
    EcsWorld *world,
    EcsStage *stage,
    EcsEntity system,
    EcsType type);

/* Activate table for system (happens if table goes from empty to not empty) */
void ecs_system_activate_table(
    EcsWorld *world,
    EcsEntity system,
    EcsTable *table,
    bool active);

/* Run a job (from a worker thread) */
void ecs_run_job(
    EcsWorld *world,
    EcsThread *thread,
    EcsJob *job);

/* Run a task (periodic system that is not matched against any tables) */
void ecs_run_task(
    EcsWorld *world,
    EcsEntity system,
    float delta_time);

/* Invoke row system */
void ecs_row_notify(
    EcsWorld *world,
    EcsEntity system,
    EcsRowSystem *system_data,
    int32_t *columns,
    EcsTableColumn *table_columns,
    uint32_t offset,
    uint32_t limit);

/* Callback for parse_component_expr that stores result as EcsSystemColumn's */
EcsResult ecs_parse_component_action(
    EcsWorld *world,
    EcsSystemExprElemKind elem_kind,
    EcsSystemExprOperKind oper_kind,
    const char *component_id,
    void *data);

/* -- Worker API -- */

/* Compute schedule based on current number of entities matching system */
void ecs_schedule_jobs(
    EcsWorld *world,
    EcsEntity system);

/* Prepare jobs */
void ecs_prepare_jobs(
    EcsWorld *world,
    EcsEntity system);

/* Run jobs */
void ecs_run_jobs(
    EcsWorld *world);

/* -- Private utilities -- */

/* Compute hash */
void ecs_hash(
    const void *key,
    size_t length,
    uint32_t *result);

/* Convert 64bit value to EcsRow type. EcsRow is stored as 64bit int in the
 * entity index */
EcsRow ecs_to_row(
    uint64_t value);

/* Get 64bit integer from EcsRow */
uint64_t ecs_from_row(
    EcsRow row);

/* Utility that parses system signature */
EcsResult ecs_parse_component_expr(
    EcsWorld *world,
    const char *sig,
    ecs_parse_action action,
    void *ctx);

/* Test whether signature has columns that must be retrieved from a table */
bool ecs_needs_tables(
    EcsWorld *world,
    const char *signature);

/* Count number of columns signature */
uint32_t ecs_columns_count(
    const char *sig);

#define assert_func(cond) _assert_func(cond, #cond, __FILE__, __LINE__, __func__)
void _assert_func(
    bool cond,
    const char *cond_str,
    const char *file,
    uint32_t line,
    const char *func);

#endif
