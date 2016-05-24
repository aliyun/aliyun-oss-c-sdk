#ifndef LIBAOS_LIST_H
#define LIBAOS_LIST_H

#include <apr_general.h>

// from kernel list
typedef struct aos_list_s aos_list_t;

struct aos_list_s {
    aos_list_t *next, *prev;
};

#define aos_list_head_init(name) {&(name), &(name)}

#define aos_list_init(ptr) do {                  \
        (ptr)->next = (ptr);                    \
        (ptr)->prev = (ptr);                    \
    } while (0)

static APR_INLINE void __aos_list_add(aos_list_t *list, aos_list_t *prev, aos_list_t *next)
{
    next->prev = list;
    list->next = next;
    list->prev = prev;
    prev->next = list;
}

// list head to add it before
static APR_INLINE void aos_list_add_tail(aos_list_t *list, aos_list_t *head)
{
    __aos_list_add(list, head->prev, head);
}

static APR_INLINE void __aos_list_del(aos_list_t *prev, aos_list_t *next)
{
    next->prev = prev;
    prev->next = next;
}

// deletes entry from list
static APR_INLINE void aos_list_del(aos_list_t *entry)
{
    __aos_list_del(entry->prev, entry->next);
    aos_list_init(entry);
}

// tests whether a list is empty
static APR_INLINE int aos_list_empty(const aos_list_t *head)
{
    return (head->next == head);
}

// move list to new_list
static APR_INLINE void aos_list_movelist(aos_list_t *list, aos_list_t *new_list)
{
    if (!aos_list_empty(list)) {
        new_list->prev = list->prev;
        new_list->next = list->next;
        new_list->prev->next = new_list;
        new_list->next->prev = new_list;
        aos_list_init(list);
    } else {
        aos_list_init(new_list);
    }
}

// get last
#define aos_list_get_last(list, type, member)                           \
    aos_list_empty(list) ? NULL : aos_list_entry((list)->prev, type, member)

// get first
#define aos_list_get_first(list, type, member)                          \
    aos_list_empty(list) ? NULL : aos_list_entry((list)->next, type, member)

#define aos_list_entry(ptr, type, member) \
    (type *)( (char *)ptr - APR_OFFSETOF(type, member) )

// traversing
#define aos_list_for_each_entry(postp, pos, head, member)                      \
    for (pos = aos_list_entry((head)->next, postp, member);      \
         &pos->member != (head);                                        \
         pos = aos_list_entry(pos->member.next, postp, member))

#define aos_list_for_each_entry_reverse(postp, pos, head, member)              \
    for (pos = aos_list_entry((head)->prev, postp, member);      \
         &pos->member != (head);                                        \
         pos = aos_list_entry(pos->member.prev, postp, member))

#define aos_list_for_each_entry_safe(postp, pos, n, head, member)              \
    for (pos = aos_list_entry((head)->next, postp, member),      \
                 n = aos_list_entry(pos->member.next, postp, member); \
         &pos->member != (head);                                        \
         pos = n, n = aos_list_entry(n->member.next, postp, member))

#define aos_list_for_each_entry_safe_reverse(postp, pos, n, head, member)      \
    for (pos = aos_list_entry((head)->prev, postp, member),      \
                 n = aos_list_entry(pos->member.prev, postp, member); \
         &pos->member != (head);                                        \
         pos = n, n = aos_list_entry(n->member.prev, postp, member))

#endif
