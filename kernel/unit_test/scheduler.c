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

    scheduler_wait(9, EVENT_PROCESS_FINISHED);

    ASSERT_EQUAL_INT(TASK_WAITING, scheduler_state(9));
    ASSERT_EQUAL_INT(EVENT_PROCESS_FINISHED, scheduler_event(9));

    return 0;
}
