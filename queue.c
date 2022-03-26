#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (head)
        INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;
    element_t *entry = NULL, *safe = NULL;
    list_for_each_entry_safe (entry, safe, l, list) {
        list_del(&entry->list);
        free(entry->value);
        free(entry);
    }
    free(l);
}

bool malloc_element(element_t **new, char *s)
{
    *new = malloc(sizeof(element_t));
    if (!*new)
        return false;
    size_t size = strlen(s) + 1;
    (*new)->value = malloc(size);
    if (!(*new)->value) {
        free(*new);
        return false;
    }
    strncpy((*new)->value, s, size);
    return true;
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *new_e = NULL;
    if (!malloc_element(&new_e, s))
        return false;
    list_add(&new_e->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *new_e = NULL;
    if (!malloc_element(&new_e, s))
        return false;
    list_add_tail(&new_e->list, head);
    return true;
}

element_t *remove_node(struct list_head *node, char *sp, size_t bufsize)
{
    element_t *entry = list_entry(node, element_t, list);
    size_t size = strlen(entry->value) + 1;
    size = (bufsize < size) ? bufsize : size;
    strncpy(sp, entry->value, size);
    sp[size - 1] = '\0';
    node->prev->next = node->next;
    node->next->prev = node->prev;
    return list_entry(node, element_t, list);
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head) || !sp)
        return NULL;
    return remove_node(head->next, sp, bufsize);
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head) || !sp)
        return NULL;
    return remove_node(head->prev, sp, bufsize);
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;
    int len = 0;
    struct list_head *li;
    list_for_each (li, head)
        len++;
    return len;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;
    struct list_head *slow = head->next, *fast = head->next->next;
    for (; fast != head && fast->prev != head;
         slow = slow->next, fast = fast->next->next)
        ;
    element_t *entry = list_entry(slow, element_t, list);
    list_del(&entry->list);
    free(entry->value);
    free(entry);
    return true;
}

/* Delete partial list (not including begin & end) */
void q_delete_partial_list(struct list_head *begin, struct list_head *end)
{
    for (struct list_head *rm = begin->next; rm != end;) {
        element_t *rm_ele = list_entry(rm, element_t, list);
        rm = rm->next;
        q_release_element(rm_ele);
    }
    begin->next = end;
    end->prev = begin;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head)
        return false;
    struct list_head *src = head->next, *tgt = head->next->next;
    for (; tgt != head; tgt = tgt->next) {
        if (strcmp(list_entry(src, element_t, list)->value,
                   list_entry(tgt, element_t, list)->value)) {
            if (src->next != tgt) {
                src = src->prev;
                q_delete_partial_list(src, tgt);
            }
            src = src->next;
        }
    }
    if (src->next != tgt) {
        src = src->prev;
        q_delete_partial_list(src, tgt);
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head || list_empty(head))
        return;
    for (struct list_head *fast = head->next->next;
         fast != head && fast->prev != head; fast = fast->next->next->next) {
        fast->next->prev = fast->prev;
        fast->prev->next = fast->next;
        fast->prev = fast->prev->prev;
        fast->prev->next = fast;
        fast->next = fast->next->prev;
        fast->next->prev = fast;
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head)
        return;
    struct list_head *node = NULL, *safe = NULL;
    list_for_each_safe (node, safe, head) {
        node->next = node->prev;
        node->prev = safe;
    }
    node->next = node->prev;
    node->prev = safe;
}

void q_sort_list(struct list_head *head)
{
    // Cut the list in half
    if (list_empty(head) || list_is_singular(head))
        return;
    struct list_head *slow = head->next;
    for (struct list_head *fast = head->next->next;
         fast != head && fast->next != head;
         slow = slow->next, fast = fast->next->next)
        ;
    struct list_head half;
    list_cut_position(&half, head, slow);
    q_sort_list(head);
    q_sort_list(&half);

    // Integrate two sorted lists into a single list
    struct list_head *p_head = head->next, *p_list = half.next, *p_pos = head;
    while (p_head != head && p_list != &half) {
        if (strcmp(list_entry(p_head, element_t, list)->value,
                   list_entry(p_list, element_t, list)->value) <= 0) {
            p_pos->next = p_head;
            p_head->prev = p_pos;
            p_head = p_head->next;
        } else {
            p_pos->next = p_list;
            p_list->prev = p_pos;
            p_list = p_list->next;
        }
        p_pos = p_pos->next;
    }
    // Tail-to-head update
    if (p_list == &half) {
        p_pos->next = p_head;
        p_head->prev = p_pos;
        // Skip the update since head's tail already links to head's head
    } else {
        p_pos->next = p_list;
        p_list->prev = p_pos;
        head->prev = half.prev;
        head->prev->next = head;
    }
}

/* Sort elements of queue in ascending order */
void q_sort(struct list_head *head)
{
    if (!head)
        return;
    q_sort_list(head);
}
