/*----------------------------------------------------------------------------
Copyright (c) 2013 Gauthier Fleutot Ostervall
----------------------------------------------------------------------------*/
#include "linkedlist.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//******************************************************************************
// Module constants
//******************************************************************************

//******************************************************************************
// Module variables
//******************************************************************************

//******************************************************************************
// Function prototypes
//******************************************************************************
static struct ll_node *nodes_walker(struct ll_node *start, int pos);
static void nodes_run_for_all(struct ll_node *n, void (*callback)(void *data));

//******************************************************************************
// Function definitions
//******************************************************************************

//  ----------------------------------------------------------------------------
/// @brief  Add a new node  at the end of a linked list, with content
/// data.
/// @attention  The data object must be dynamically allocated since the list's
/// remove functions use free() on data objects.
//  ----------------------------------------------------------------------------
void linkedlist_add(struct linkedlist *list, void *data)
{
    linkedlist_add_before(list, NULL, data);
}

void linkedlist_prepend(struct linkedlist *list, void *data)
{
    linkedlist_add_before(list, list->head, data);
}

void linkedlist_add_before(struct linkedlist *list, void *at, void *data)
{
    struct ll_node *node= malloc(sizeof (struct ll_node));

    *node = (struct ll_node) {
        .data = data,
        .next = NULL
    };

    if (list->head == NULL || list->size == 0) {
        list->head = node;
        list->size = 1;
    } else {
        // Walk to the last node, or before node `at`.
        struct ll_node *n = list->head;
        while (n->next != NULL && n->next->data != at) {
            n = n->next;
        }
        node->next = n->next;
        n->next = node;
        list->size++;
    }
}

void linkedlist_rm(struct linkedlist *l, void *data)
{
    struct ll_node *n;
    struct ll_node *to_remove;

    if (l->head->data == data) {
        to_remove = l->head;
        l->head = l->head->next;
    } else {
        for (n = l->head;
	     n != NULL && n->next != NULL && n->next->data != data;
	     n = n->next) {
        }
        if (!n || !n->next)
            return;
        to_remove = n->next;
        n->next = n->next->next;
    }

    free(to_remove);
    l->size--;
}

//  ----------------------------------------------------------------------------
/// \brief  Free all nodes of the list passed as parameter.
//  ----------------------------------------------------------------------------
void linkedlist_destroy(struct linkedlist *list)
{
    if (list == NULL) {
        return;
    } else {
        struct ll_node *next;
        for (struct ll_node *n = list->head; n != NULL; n = next) {
            next = n->next;
            free(n->data);
            free(n);
        }
    }
    list->head = NULL;
    list->size = 0;
}


//  ----------------------------------------------------------------------------
/// \brief  Run the callback function on all nodes' data member.
//  ----------------------------------------------------------------------------
void linkedlist_run_for_all(struct linkedlist *list,
                            void (*callback) (void *data))
{
    if (list == NULL || list->head == NULL) {
        return;
    }
    nodes_run_for_all(list->head, callback);
}


void linkedlist_next_select(struct linkedlist *list)
{
    if ((list->selected == NULL) || (list->selected->next == NULL)) {
        list->selected = list->head;
    } else {
        list->selected = list->selected->next;
    }
}

void *linkedlist_selected_data_get(struct linkedlist *list)
{
    if (list == NULL || list->selected == NULL) {
        return NULL;
    }
    return list->selected->data;
}

//  ----------------------------------------------------------------------------
/// \brief  Walk to the node at position, and get a pointer to the data at that
/// position.
/// \param  list
/// \param  position
/// \return Pointer to the data at position in list.
//  ----------------------------------------------------------------------------
void *linkedlist_data_handle_get(struct linkedlist * const list,
                                 unsigned int const position)
{
    struct ll_node *walker = nodes_walker(list->head, position);

    return walker->data;
}

// ----------------------------------------------------------------------------
/// \brief Walk pos number of nodes from start. If the tail of a list is
/// reached, go on from head (wrap around).
/// \param  start   The first node.
/// \param  pos     Number of steps to take. May be negative.
/// \return Pointer to the target node.
//  ----------------------------------------------------------------------------
static struct ll_node *nodes_walker(struct ll_node *start, int pos)
{
    struct ll_node *walker = start;
    for (int i = 0; i < pos; i++) {
        if (walker->next != NULL) {
            walker = walker->next;
        } else {
            // wrap around.
            walker = start;
        }
    }
    return walker;
}

static void nodes_run_for_all(struct ll_node *n, void (*callback)(void *data))
{
    callback(n->data);
    if (n->next != NULL) {
        nodes_run_for_all(n->next, callback);
    }
}
