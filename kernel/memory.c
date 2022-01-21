#include <include/memory.h>

#define PAGES_MAX 256

/* To keep track of allocated memory pages. */
int num_pages;
uint8_t page_table[PAGES_MAX];

/* Routines for memory management. */

/* Initializes the memory manager.
 *
 * Parameters:
 *     pages: Number of pages available in memory.
 * 
 * Returns:
 *     0 if initialization successful, error code otherwise.
 */
int memory_init(int pages)
{
    if (pages > PAGES_MAX) return E_INIT;

    num_pages = pages;

    /* Mark all available pages as free. */
    for (int i = 0; i < pages; i++)
    {
        page_table[i] = PAGE_FREE;
    }

    /* Mark all the rest as disabled. */
    for (int i = pages; i < PAGES_MAX; i++)
    {
        page_table[i] = PAGE_DISABLED;
    }

#ifdef DEBUG
    page_table[0] = PAGE_USED;
#endif
}


/* Allocates a page of memory.
 *
 * Paramters:
 *     None.
 * 
 * Returns:
 *     Index in page table of the next free
 *     page, or <0 if error.
 */
int memory_allocate(void)
{
    for (int i = 0; i < num_pages; i++)
    {
        /* If we've found a free page, mark it as used
         * and return the index. */
        if (page_table[i] == PAGE_FREE)
        {
            page_table[i] = PAGE_USED;
            return i;
        }
    }

    /* No free pages, return error. */
    return E_NOPAGES;
}

/* Frees a page.
 *
 * Parameters:
 *     page: Page to be freed.
 */
void memory_free(int page)
{
    /* Free the page. */
    page_table[page] = PAGE_FREE;
}
