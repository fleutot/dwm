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

struct ll_node {
    void *data;
    struct ll_node *next;
};

struct linkedlist_s {
    int size;
    struct ll_node *head;
};

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

//******************************************************************************
// Function definitions
//******************************************************************************

//  ----------------------------------------------------------------------------
/// @brief  Add a new node  at the end of a linked list, with content
/// data.
/// @attention  The data object must be dynamically allocated since the list's
/// destroy function uses free() on all data objects.
//  ----------------------------------------------------------------------------
struct ll_node *linkedlist_add(struct ll_node *dst, void *data)
{
    struct ll_node *node= malloc(sizeof (struct ll_node));

    *node = (struct ll_node) {
        .data = data,
        .next = NULL
    };

    if (dst == NULL) {
        // NULL means this list was empty.
        dst = node;
    } else {
        // Walk to the last node.
        struct ll_node *n = (struct ll_node *) dst;
        while (n->next != NULL) {
            n = n->next;
        }
        n->next = node;
    }
    return dst;
}


//  ----------------------------------------------------------------------------
/// \brief  Free all nodes of the list passed as parameter, then the list object
/// itself.
//  ----------------------------------------------------------------------------
void linkedlist_destroy(struct ll_node *list)
{
    if (list == NULL) {
        return;
    } else {
        linkedlist_destroy(list->next);
        if (list->data != NULL) {
            //free((void *) list->data);
        }
        free(list);
    }
}


//  ----------------------------------------------------------------------------
/// \brief  Run the callback function on all nodes' data member.
//  ----------------------------------------------------------------------------
void linkedlist_run_for_all(struct ll_node *list,
                            void (*callback) (void *data))
{
    if (list == NULL) {
        return;
    }
    callback(list->data);
    linkedlist_run_for_all(list->next, callback);
}

//  ----------------------------------------------------------------------------
/// \brief  Walk to the node at position, and get a pointer to the data at that
/// position.
/// \param  list
/// \param  position
/// \return Pointer to the data at position in list.
//  ----------------------------------------------------------------------------
void *linkedlist_data_handle_get(struct ll_node * const list,
                                 unsigned int const position)
{
    struct ll_node *walker = nodes_walker(list, position);

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
