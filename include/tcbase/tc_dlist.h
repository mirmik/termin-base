#ifndef TC_DLIST_H
#define TC_DLIST_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tc_dlist_node {
    struct tc_dlist_node* next;
    struct tc_dlist_node* prev;
} tc_dlist_node;

typedef tc_dlist_node tc_dlist_head;

#define TC_DLIST_HEAD_INIT(name) { &(name), &(name) }
#define TC_DLIST_HEAD(name) tc_dlist_head name = TC_DLIST_HEAD_INIT(name)

#define tc_dlist_entry(ptr, type, member) \
    ((type*)((char*)(ptr) - offsetof(type, member)))

static inline void tc_dlist_init_head(tc_dlist_head* head) {
    head->next = head;
    head->prev = head;
}

static inline void tc_dlist_init_node(tc_dlist_node* node) {
    node->next = node;
    node->prev = node;
}

static inline bool tc_dlist_is_linked(const tc_dlist_node* node) {
    return node->next != node;
}

static inline bool tc_dlist_empty(const tc_dlist_head* head) {
    return head->next == head;
}

static inline void tc_dlist__add(tc_dlist_node* node, tc_dlist_node* prev, tc_dlist_node* next) {
    node->prev = prev;
    node->next = next;
    next->prev = node;
    prev->next = node;
}

static inline void tc_dlist_add(tc_dlist_node* node, tc_dlist_head* head) {
    tc_dlist__add(node, head, head->next);
}

static inline void tc_dlist_add_tail(tc_dlist_node* node, tc_dlist_head* head) {
    tc_dlist__add(node, head->prev, head);
}

static inline void tc_dlist__del(tc_dlist_node* prev, tc_dlist_node* next) {
    next->prev = prev;
    prev->next = next;
}

static inline void tc_dlist_del(tc_dlist_node* node) {
    if (node->next != node) {
        tc_dlist__del(node->prev, node->next);
        tc_dlist_init_node(node);
    }
}

static inline void tc_dlist_move(tc_dlist_node* node, tc_dlist_head* head) {
    tc_dlist__del(node->prev, node->next);
    tc_dlist_add(node, head);
}

static inline void tc_dlist_move_tail(tc_dlist_node* node, tc_dlist_head* head) {
    tc_dlist__del(node->prev, node->next);
    tc_dlist_add_tail(node, head);
}

#define tc_dlist_first_entry(head, type, member) \
    (tc_dlist_empty(head) ? NULL : tc_dlist_entry((head)->next, type, member))

#define tc_dlist_last_entry(head, type, member) \
    (tc_dlist_empty(head) ? NULL : tc_dlist_entry((head)->prev, type, member))

#define tc_dlist_next_entry(pos, type, member) tc_dlist_entry((pos)->member.next, type, member)
#define tc_dlist_prev_entry(pos, type, member) tc_dlist_entry((pos)->member.prev, type, member)

#define tc_dlist_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

#define tc_dlist_for_each_safe(pos, tmp, head) \
    for (pos = (head)->next, tmp = pos->next; pos != (head); pos = tmp, tmp = pos->next)

#define tc_dlist_for_each_entry(pos, head, type, member) \
    for (pos = tc_dlist_entry((head)->next, type, member); \
         &pos->member != (head); \
         pos = tc_dlist_entry(pos->member.next, type, member))

#define tc_dlist_for_each_entry_reverse(pos, head, type, member) \
    for (pos = tc_dlist_entry((head)->prev, type, member); \
         &pos->member != (head); \
         pos = tc_dlist_entry(pos->member.prev, type, member))

#define tc_dlist_for_each_entry_safe(pos, tmp, head, type, member) \
    for (pos = tc_dlist_entry((head)->next, type, member), \
         tmp = tc_dlist_entry(pos->member.next, type, member); \
         &pos->member != (head); \
         pos = tmp, tmp = tc_dlist_entry(tmp->member.next, type, member))

static inline size_t tc_dlist_size(const tc_dlist_head* head) {
    size_t count = 0;
    tc_dlist_node* pos;
    tc_dlist_for_each(pos, head) {
        count++;
    }
    return count;
}

static inline bool tc_dlist_contains(const tc_dlist_head* head, const tc_dlist_node* node) {
    tc_dlist_node* pos;
    tc_dlist_for_each(pos, head) {
        if (pos == node) return true;
    }
    return false;
}

#ifdef __cplusplus
}
#endif

#endif // TC_DLIST_H
