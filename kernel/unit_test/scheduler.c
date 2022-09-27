#include <include/scheduler.h>

#include <test.h>

/* Given a single scheduled process, check that
 * scheduler_next() always returns the same value.
 */
int test_scheduled_single()
{
    scheduler_init();

    scheduler_add(0);

    for (int i = 0; i < 100; i++)
    {
        int n = scheduler_next();
        ASSERT_EQUAL_INT(0, n);
    }

    return 0;
}

/* Given multiple scheduled tasks, check that the
 * scheduler schedules all of them equally.
 */
int test_scheduled_multiple()
{
    scheduler_init();

    scheduler_add(6);
    scheduler_add(9);
    scheduler_add(2);

    for (int i = 0; i < 100; i++)
    {
        int n = scheduler_next();
        ASSERT_EQUAL_INT(TASK_RUNNING, scheduler_state(n));

        if (i % 3 == 0) ASSERT_EQUAL_INT(6, n);
        else if (i % 3 == 1) ASSERT_EQUAL_INT(9, n);
        else if (i % 3 == 2) ASSERT_EQUAL_INT(2, n);
    }

    return 0;
}

/* Given multiple scheduled tasks, check that the
 * scheduler schedules all of them equally.
 */
int test_scheduled_multiple_2()
{
    scheduler_init();
    scheduler_add(0);

    int n = scheduler_next();
    ASSERT_EQUAL_INT(0, n);

    n = scheduler_next();
    ASSERT_EQUAL_INT(0, n);

    n = scheduler_next();
    ASSERT_EQUAL_INT(0, n);

    scheduler_add(1);

    n = scheduler_next();
    ASSERT_EQUAL_INT(1, n);

    n = scheduler_next();
    ASSERT_EQUAL_INT(0, n);

    n = scheduler_next();
    ASSERT_EQUAL_INT(1, n);

    n = scheduler_next();
    ASSERT_EQUAL_INT(0, n);

    return 0;
}

/* Check that a task which has finished is not scheduled.
 */
int test_scheduled_exit()
{
    scheduler_init();

    scheduler_add(6);
    scheduler_add(9);
    scheduler_add(2);

    for (int i = 0; i < 100; i++)
    {
        int n = scheduler_next();
        if (i % 3 == 0) ASSERT_EQUAL_INT(6, n);
        else if (i % 3 == 1) ASSERT_EQUAL_INT(9, n);
        else if (i % 3 == 2) ASSERT_EQUAL_INT(2, n);
    }

    scheduler_exit(6, 0);

    for (int i = 0; i < 100; i++)
    {
        int n = scheduler_next();
        if (i % 2 == 0) ASSERT_EQUAL_INT(9, n);
        else if (i % 2 == 1) ASSERT_EQUAL_INT(2, n);
    }

    return 0;
}

/* Tests that attempting to add too many tasks
 * to the scheduler results in an error code.
 */
int test_schedule_too_many()
{
    scheduler_init();

    for (int i = 0; i < 16; i++)
    {
        int e = scheduler_add(i);
        ASSERT_EQUAL_INT(0, e);
    }

    int e2 = scheduler_add(16);
    ASSERT_EQUAL_INT(E_TOO_MANY_TASKS, e2);

    return 0;
}

/* Tests that the return status of a task can be
 * accessed.
 */
int test_schedule_return_status()
{
    scheduler_init();

    scheduler_add(9);
    
    scheduler_exit(9, 42);

    int ret = scheduler_exitcode(9);

    ASSERT_EQUAL_INT(42, ret);

    return 0;
}

/* Tests that waiting on an event sets the status
 * of a task correctly.
 */
int test_schedule_wait_process()
{
    scheduler_init();

    scheduler_add(9);

    ASSERT_EQUAL_INT(TASK_READY, scheduler_state(9));
    ASSERT_EQUAL_INT(EVENT_NO_EVENT, scheduler_event(9));

    scheduler_block(9, EVENT_PROCESS_FINISHED);

    ASSERT_EQUAL_INT(TASK_BLOCKED, scheduler_state(9));
    ASSERT_EQUAL_INT(EVENT_PROCESS_FINISHED, scheduler_event(9));

    return 0;
}

/* Tests that the right event occurring transitions
 * a task from WAITING to READY when waiting on EVENT_PROCESS_FINISHED
 * and a process finishes.
 */
int test_schedule_transition_from_waiting_process_finished()
{
    scheduler_init();

    scheduler_add(5);
    scheduler_add(6);

    ASSERT_EQUAL_INT(TASK_READY, scheduler_state(5));
    ASSERT_EQUAL_INT(TASK_READY, scheduler_state(6));

    scheduler_tick();

    /* PID 5 should now be executing, 6 ready. */
    ASSERT_EQUAL_INT(TASK_RUNNING, scheduler_state(5));
    ASSERT_EQUAL_INT(TASK_READY, scheduler_state(6));

    scheduler_block(5, EVENT_PROCESS_FINISHED);

    /* PID 5 should now be waiting, 6 ready. */
    ASSERT_EQUAL_INT(TASK_BLOCKED, scheduler_state(5));
    ASSERT_EQUAL_INT(TASK_READY, scheduler_state(6));

    /* PID 6 should now be executing. */
    scheduler_tick();

    /* PID 5 should now be waiting, 6 running. */
    ASSERT_EQUAL_INT(TASK_BLOCKED, scheduler_state(5));
    ASSERT_EQUAL_INT(TASK_RUNNING, scheduler_state(6));

    /* PID 6 finishes, should broadcast event. */
    scheduler_exit(6, 42);

    /* PID 5 should now be ready, 6 finished. */
    ASSERT_EQUAL_INT(TASK_READY, scheduler_state(5));
    ASSERT_EQUAL_INT(TASK_FINISHED, scheduler_state(6));

    return 0;
}

/* Tests that blocked tasks do not get scheduled.
 * Also tests that a task does get scheduled once it is no longer waiting.
 */
int test_schedule_task_not_scheduled_when_waiting(void)
{
    scheduler_init();

    scheduler_add(14);
    scheduler_add(52);

    ASSERT_EQUAL_INT(TASK_READY, scheduler_state(14));
    ASSERT_EQUAL_INT(TASK_READY, scheduler_state(52));

    scheduler_tick();

    /* PID 14 should now be executing, 52 ready. */
    ASSERT_EQUAL_INT(TASK_RUNNING, scheduler_state(14));
    ASSERT_EQUAL_INT(TASK_READY, scheduler_state(52));

    scheduler_block(14, EVENT_PROCESS_FINISHED);

    /* PID 14 should now be waiting, 52 ready. */
    ASSERT_EQUAL_INT(TASK_BLOCKED, scheduler_state(14));
    ASSERT_EQUAL_INT(TASK_READY, scheduler_state(52));

    /* PID 52 should now be executing. */
    scheduler_tick();

    /* PID 14 should now be waiting, 52 running. */
    ASSERT_EQUAL_INT(TASK_BLOCKED, scheduler_state(14));
    ASSERT_EQUAL_INT(TASK_RUNNING, scheduler_state(52));

    /* Run scheduler for 10 ticks and ensure 14 is never scheduled. */
    for (int i = 0; i < 10; i++)
    {
        scheduler_tick();

        /* PID 14 should now be waiting, 52 running. */
        ASSERT_EQUAL_INT(TASK_BLOCKED, scheduler_state(14));
        ASSERT_EQUAL_INT(TASK_RUNNING, scheduler_state(52));
    }

    /* PID 52 finishes, should broadcast event. */
    scheduler_exit(52, 42);

    /* PID 14 should now be ready, 52 finished. */
    ASSERT_EQUAL_INT(TASK_READY, scheduler_state(14));
    ASSERT_EQUAL_INT(TASK_FINISHED, scheduler_state(52));

    scheduler_tick();

    /* PID 14 should now be running, 52 finished. */
    ASSERT_EQUAL_INT(TASK_RUNNING, scheduler_state(14));
    ASSERT_EQUAL_INT(TASK_FINISHED, scheduler_state(52));

    return 0;
}
