#include <include/scheduler.h>

#include <include/process.h>
#include <include/ram.h>

#include <stdint.h>

typedef struct _ScheduleTableEntry_T
{
    TaskState_T state;
    EventType_T waiting_event;
    int pid;
    int exitcode;
} ScheduleTableEntry_T;

#define MAX_SCHEDULED 16

int current_scheduled;
int num_scheduled;
ScheduleTableEntry_T schedule_table[MAX_SCHEDULED];

void scheduler_init(void)
{
    for (int i = 0; i < MAX_SCHEDULED; i++)
    {
        schedule_table[i].state = TASK_FREE;
    }
    current_scheduled = -1;
    num_scheduled = 0;
#ifdef DEBUG
    schedule_table[0].state = TASK_READY;
    schedule_table[0].pid = 0;
    num_scheduled = 1;
#endif
}

/* Helper function to allocate a place in the schedule table. */
int scheduler_allocate(void)
{
    for (int i = 0; i < MAX_SCHEDULED; i++)
    {
        if (schedule_table[i].state == TASK_FREE) return i;
    }

    return -1;
}

/* Helper function to get the scheduler entry with given PID. */
int scheduler_entry(int pid)
{
    for (int i = 0; i < MAX_SCHEDULED; i++)
    {
        if (schedule_table[i].pid == pid) return i;
    }

    return -1;
}

int scheduler_add(int pid)
{
    int s = scheduler_allocate();
    if (s < 0) return E_TOO_MANY_TASKS;

    schedule_table[s].state = TASK_READY;
    schedule_table[s].waiting_event = EVENT_NO_EVENT;
    schedule_table[s].pid = pid;

    num_scheduled++;

    return 0;
}

TaskState_T scheduler_state(int pid)
{
    int s = scheduler_entry(pid);
    return schedule_table[s].state;
}

void scheduler_exit(int pid, int exitcode)
{
    int s = scheduler_entry(pid);
    schedule_table[s].state = TASK_FINISHED;
    schedule_table[s].exitcode = exitcode;
}

int scheduler_exitcode(int pid)
{
    int s = scheduler_entry(pid);
    return schedule_table[s].exitcode;
}

int scheduler_current(void)
{
    return current_scheduled;
}

int scheduler_next(void)
{
    if (current_scheduled >= 0 && schedule_table[current_scheduled].state == TASK_RUNNING)
    {
        schedule_table[current_scheduled].state = TASK_READY;
    }

    /* Find the next READY task. */
    while (1)
    {
        current_scheduled++;
        if (current_scheduled >= num_scheduled) current_scheduled = 0;

        if (schedule_table[current_scheduled].state == TASK_READY) break;
    }

    schedule_table[current_scheduled].state = TASK_RUNNING;
    return schedule_table[current_scheduled].pid;
}

uint8_t scheduler_tick()
{
    int pid = scheduler_next();

    const ProcessDescriptor_T * p = process_info(pid);

    return p->bank;
}

/* scheduler_wait
 *
 * Purpose:
 *     Indicates that the task with given Process ID should
 *     wait on a particular event.
 * 
 * Parameters:
 *     Process ID
 *     Event type
 * 
 * Returns:
 *     Nothing.
 */
void scheduler_wait(int pid, EventType_T event)
{
    int s = scheduler_entry(pid);
    schedule_table[s].waiting_event = event;
    schedule_table[s].state = TASK_WAITING;
}

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
EventType_T scheduler_event(int pid)
{
    int s = scheduler_entry(pid);
    return schedule_table[s].waiting_event;
}
