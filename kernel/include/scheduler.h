#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include <stdint.h>

#define E_TOO_MANY_TASKS -1

typedef int8_t TaskState_T;

#define TASK_RUNNING  ((TaskState_T)0)
#define TASK_READY    ((TaskState_T)1)
#define TASK_FINISHED ((TaskState_T)2)
#define TASK_FREE     ((TaskState_T)3)
#define TASK_BLOCKED  ((TaskState_T)4)

typedef int EventType_T;

#define EVENT_NO_EVENT ((EventType_T)0)
#define EVENT_PROCESS_FINISHED ((EventType_T)1)

/* scheduler_init
 *
 * Purpose:
 *     Initialize the scheduler.
 * 
 * Parameters:
 *     None.
 * 
 * Returns:
 *     Nothing.
 */
void scheduler_init(void);

/* scheduler_add
 *
 * Purpose:
 *     Add a process to the scheduler.
 * 
 * Parameters:
 *     Process ID.
 * 
 * Returns:
 *     Error code, or 0 on success.
 */
int scheduler_add(int pid);

/* scheduler_next
 *
 * Purpose:
 *     Return the PID of the next scheduled
 *     process.
 * 
 * Parameters:
 *     None.
 * 
 * Returns:
 *     PID (int).
 */
int scheduler_next(void);

/* scheduler_tick
 *
 * Purpose:
 *     Performs one tick of the scheduler, advancing
 *     to the next available task.
 * 
 * Parameters:
 *     None.
 * 
 * Returns:
 *     Memory bank to be selected for new process.
 */
uint8_t scheduler_tick(void);

/* scheduler_exit
 *
 * Purpose:
 *     Ends the task with the given PID.
 *     Sets the exit code of the task.
 * 
 * Parameters:
 *     pid:      Process ID.
 *     exitcode: Exit code.
 * 
 * Returns:
 *     Nothing.
 */
void scheduler_exit(int pid, int exitcode);

/* scheduler_exitcode
 *
 * Purpose:
 *     Gets the exitcode of the task
 *     with the given PID.
 * 
 * Parameters:
 *     pid:      Process ID.
 * 
 * Returns:
 *     Exit code.
 */
int scheduler_exitcode(int pid);

/* scheduler_state
 *
 * Purpose:
 *     Gets the state of the task with given PID.
 * 
 * Parameters:
 *     Process ID.
 * 
 * Returns:
 *     Task state.
 */
TaskState_T scheduler_state(int pid);

/* scheduler_current
 *
 * Purpose:
 *     Gets the currently-executing task ID.
 * 
 * Parameters:
 *     None.
 * 
 * Returns:
 *     Task ID.
 */
int scheduler_current(void);

/* scheduler_block
 *
 * Purpose:
 *     Indicates that the task with given Process ID should
 *     be blocked on a particular event.
 * 
 * Parameters:
 *     Process ID
 *     Event type
 * 
 * Returns:
 *     Nothing.
 */
void scheduler_block(int pid, EventType_T event);

/* scheduler_event
 *
 * Purpose:
 *     Returns the event a given task is waiting on.
 * 
 * Parameters:
 *     Process ID of task
 * 
 * Returns:
 *     Event type.
 */
EventType_T scheduler_event(int pid);

#endif
