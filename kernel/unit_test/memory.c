#include <include/memory.h>

#include <test.h>

/* Given an initial state where all memory pages are free,
 * checks that allocating a page returns the first one.
 */
int test_alloc_first()
{
    /* Initialize with 16 pages. */
    memory_init(16);

    int p = memory_allocate();
    ASSERT_EQUAL_INT(0, p);

    return 0;
}

/* Given an initial state where all memory pages are free,
 * checks that allocating all pages works correctly.
 */
int test_alloc_all()
{
    /* Initialize with 16 pages. */
    memory_init(16);

    for (int i = 0; i < 16; i++)
    {
        int p = memory_allocate();
        ASSERT_EQUAL_INT(i, p);
    }

    return 0;
}

/* Tests that allocating too many pages results in an error.
 */
int test_alloc_too_many()
{
    /* Initialize with 16 pages. */
    memory_init(16);

    for (int i = 0; i < 16; i++)
    {
        int p = memory_allocate();
    }

    int p2 = memory_allocate();
    ASSERT_EQUAL_INT(E_NOPAGES, p2);

    int p3 = memory_allocate();
    ASSERT_EQUAL_INT(E_NOPAGES, p3);

    return 0;
}

/* Tests that allocated memory can be freed.
 */
int test_alloc_then_free()
{
    /* Initialize with 16 pages. */
    memory_init(16);

    int p1 = memory_allocate();
    ASSERT_EQUAL_INT(0, p1);

    int p2 = memory_allocate();
    ASSERT_EQUAL_INT(1, p2);

    int p3 = memory_allocate();
    ASSERT_EQUAL_INT(2, p3);

    memory_free(p2);

    int p4 = memory_allocate();
    ASSERT_EQUAL_INT(1, p4);

    return 0;
}

/* Tests that disabled memory stays disabled when freed.
 */
int test_free_disabled()
{
    /* Initialize with 16 pages. */
    memory_init(16);

    for (int i = 0; i < 16; i++)
    {
        int p = memory_allocate();
    }

    /* Free a disabled page. */
    memory_free(22);

    /* Allocate another page. We shouldn't be able to
     * because all enabled pages are in use. */
    int p2 = memory_allocate();
    ASSERT_EQUAL_INT(E_NOPAGES, p2);

    return 0;
}

/* Tests that we can't initialize with too many pages.
 */
int test_init_too_many()
{
    /* Initialize with 257 pages. */
    int e = memory_init(257);
    ASSERT_EQUAL_INT(E_INIT, e);

    return 0;
}
