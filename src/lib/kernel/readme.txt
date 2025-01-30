Please delete this text file as this is not part of our project

add only the contents below from here to src/lib/kernel/list.c. I am adding the file as well:



##Headers - add this to what is already there###

#include "threads/thread.h"


add this in/ from  line # 315:

/* Comparator function for sorting threads by tid. */
static bool tid_less(const struct list_elem *a, const struct list_elem *b, void *aux UNUSED) {
    struct thread *thread_a = list_entry(a, struct thread, allelem);
    struct thread *thread_b = list_entry(b, struct thread, allelem);
    return thread_a->tid < thread_b->tid;
}

int get_sorted_index(struct list *threads, struct thread *target) {
    if (threads == NULL || target == NULL) {
        return -1;
    }

    /* Sort the list based on tid */
/* this is from the requirement:  It is required to use the given list_sort() function for sorting the input list.   */
/*list_sort method is from line # 405 after we add these code - removing this comment line and the line above*/
    list_sort(threads, tid_less, NULL);

    /* Find the index of the target thread in the sorted list */
    int index = 0;
    struct list_elem *e;
    for (e = list_begin(threads); e != list_end(threads); e = list_next(e), index++) {
        struct thread *t = list_entry(e, struct thread, allelem);
        if (t == target) {
            return index;
        }
    }

    return -1; /* Target thread not found in the list */
}
