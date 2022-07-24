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

typedef struct node_s {
    void const *data;       // Generic data type pointer.
    struct node_s *next;
} node_t;

struct linkedlist_s {
    int size;
    node_t *head;
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
static void nodes_recursive_destroy(node_t *node_p);
static void nodes_run_for_all(node_t *node_p,
                              void (*callback)(void const * const data));
static node_t *nodes_recursive_copy(node_t *src,
                                    size_t const data_size);
static bool nodes_recursive_compare(node_t * const a, node_t * const b,
                                    size_t const data_size);
static node_t *nodes_walker(node_t * const start, int const pos);
static void length_limit(linkedlist_t * const list, int const limit);

//******************************************************************************
// Function definitions
//******************************************************************************
//  ----------------------------------------------------------------------------
/// \brief  Create a new empty list, with node pointer pointing to NULL.
/// \return Pointer to the new list.
//  ----------------------------------------------------------------------------
linkedlist_t *linkedlist_create(void)
{
    linkedlist_t *new_list_p = malloc(sizeof (linkedlist_t));
    if (new_list_p == NULL) {
        fprintf(stderr, "%s: new_list_p is NULL.\n", __func__);
        return NULL;
    }
    new_list_p->head = NULL;
    new_list_p->size = 0;
    return new_list_p;
}


//  ----------------------------------------------------------------------------
/// \brief  Add a new node at the end of a linked list, with content
/// data. Allocate memory for the new node, walk through the list to its end,
/// and link to the new node. If the list was empty before being added to, the
/// new start of list is the new node itseld.
/// \attention  The data object must be dynamically allocated since the list's
/// destroy function uses free() on all data objects.
//  ----------------------------------------------------------------------------
void linkedlist_add(linkedlist_t *dst, void const * const data)
{
    if (dst == NULL) {
        fprintf(stderr, "%s: the list needs to be created first.\n", __func__);
        return;
    }

    if (dst->size >= LINKEDLIST_MAX_SIZE) {
        // Max reached, which is not an error. Do nothing.
        return;
    }

    // Create a whole new node.
    node_t *new_node_p = malloc(sizeof (node_t));
    if (new_node_p == NULL) {
        fprintf(stderr, "%s: new_node_p is NULL.\n", __func__);
        return;
    }
    *new_node_p = (node_t) {
        .data = data,
        .next = NULL
    };

    if (dst->head == NULL) {
        // NULL head means this list was empty.
        dst->head = new_node_p;
    } else {
        // Walk to the last node.
        node_t *current_node = (node_t *) dst->head;
        while (current_node->next != NULL) {
            current_node = current_node->next;
        }
        // current_node is now the last node of dst.
        current_node->next = new_node_p;
    }
    dst->size++;
}


//  ----------------------------------------------------------------------------
/// \brief  Free all nodes of the list passed as parameter, then the list object
/// itself.
//  ----------------------------------------------------------------------------
void linkedlist_destroy(linkedlist_t *list)
{
    nodes_recursive_destroy(list->head);
    free(list);
}


//  ----------------------------------------------------------------------------
/// \brief  Run the callback function on all nodes' data member.
//  ----------------------------------------------------------------------------
void linkedlist_run_for_all(linkedlist_t *list,
                            void (*callback) (void const * const data))
{
    if (list != NULL) {
        nodes_run_for_all(list->head, callback);
    } else {
        fprintf(stderr, "%s: list is NULL.\n", __func__);
    }
}


//  ----------------------------------------------------------------------------
/// \brief  Destroy a possibly non-empty dst list and fill it with a copy of
/// src. Both nodes and data are copied to new locations, no sharing of memory
/// between src and dst.
//  ----------------------------------------------------------------------------
void linkedlist_copy(linkedlist_t *dst, linkedlist_t *src,
                     size_t const data_size)
{
    if (dst->head != NULL) {
        // Do not destroy the dst object, only the genes.
        nodes_recursive_destroy(dst->head);
    }
    dst->head = nodes_recursive_copy(src->head, data_size);
    dst->size = src->size;
}


//  ----------------------------------------------------------------------------
/// \brief  Destroy a possibly non-empty sublist and fill it with a copy of list
/// from position to its end.
//  ----------------------------------------------------------------------------
void linkedlist_sublist_copy(linkedlist_t * const dst,
                             linkedlist_t * const list,
                             unsigned int const position,
                             size_t const data_size)
{
    if (list == NULL || dst == NULL) {
        fprintf(stderr, "%s: list or dst is NULL.\n", __func__);
        return;
    }

    if (dst->head != NULL) {
        linkedlist_destroy(dst);
    }

    node_t *walker = nodes_walker(list->head, position);
    dst->head = nodes_recursive_copy(walker, data_size);
    dst->size = list->size - position;
}


//  ----------------------------------------------------------------------------
/// \brief  Compare the content of two lists (values of data, not pointers).
//  ----------------------------------------------------------------------------
bool linkedlist_compare(linkedlist_t * const list_a,
                        linkedlist_t * const list_b,
                        size_t const data_size)
{
    assert(list_a);
    assert(list_b);

    return nodes_recursive_compare(list_a->head, list_b->head, data_size);
}


//  ----------------------------------------------------------------------------
/// \brief  Cross lists by changing the .next pointer of the node before
/// position. Special case when trying to cross from index 0 is taken care of.
//  ----------------------------------------------------------------------------
void linkedlist_cross(linkedlist_t * const list_a, int const pos_a,
                      linkedlist_t * const list_b, int const pos_b)
{
    if (list_a == NULL || list_b == NULL) {
        fprintf(stderr, "%s: one of the input lists is NULL.\n", __func__);
        return;
    }

    node_t *walker_a = nodes_walker(list_a->head, pos_a - 1);
    node_t *walker_b = nodes_walker(list_b->head, pos_b - 1);

    node_t *start_of_new_part_for_list_b = walker_a->next;

    if (pos_a == 0) {
        start_of_new_part_for_list_b = list_a->head;
        list_a->head = walker_b->next;
    } else {
        walker_a->next = walker_b->next;
    }

    if (pos_b == 0) {
        list_b->head = walker_a->next;
    } else {
        walker_b->next = start_of_new_part_for_list_b;
    }

    // Update the sizes and truncate if a list grows above max.
    int old_list_a_size = list_a->size;
    long new_size_a = (long) pos_a + list_b->size - pos_b;
    if (new_size_a > LINKEDLIST_MAX_SIZE) {
        length_limit(list_a, LINKEDLIST_MAX_SIZE);
    } else {
        list_a->size = new_size_a;
    }

    long new_size_b = (long) pos_b + old_list_a_size - pos_a;
    if (new_size_b > LINKEDLIST_MAX_SIZE) {
        length_limit(list_b, LINKEDLIST_MAX_SIZE);
    } else {
        list_b->size = new_size_b;
    }
}


//  ----------------------------------------------------------------------------
/// \brief  Walk to the node at position, and get a pointer to the data at that
/// position.
/// \param  list
/// \param  position
/// \return Pointer to the data at position in list.
//  ----------------------------------------------------------------------------
void *linkedlist_data_handle_get(linkedlist_t * const list,
                                 unsigned int const position)
{
    node_t *walker = nodes_walker(list->head, position);

    return (void *) walker->data;
}


//  ----------------------------------------------------------------------------
/// \brief  Get the list size from the structs data. Not actually going through
/// the list to its end.
/// \param  list The list of which the size is to be returned.
/// \return The size of the list.
//  ----------------------------------------------------------------------------
int linkedlist_size_get(linkedlist_t * const list)
{
    if (list == NULL) {
        fprintf(stderr, "%s: list is NULL.\n", __func__);
        return 0;
    } else {
        return list->size;
    }
}


//******************************************************************************
// Internal functions
//******************************************************************************
//  ----------------------------------------------------------------------------
/// \brief  Free the node passed as parameter, and run itself on the next node
/// in the list.
/// \param  Pointer to the node to free.
//  ----------------------------------------------------------------------------
static void nodes_recursive_destroy(node_t *node_p)
{
    if (node_p == NULL) {
        return;
    } else {
        node_t *rest_of_nodes = node_p->next;
        if (node_p->data != NULL) {
            free((void *) node_p->data);
        }
        free(node_p);
        nodes_recursive_destroy(rest_of_nodes);
    }
}


//  ----------------------------------------------------------------------------
/// \brief  Run the callback on the data field of the node passed as parameter,
/// and run itself on the next node in the list.
/// \param  node_p Pointer to the node that has data to run the callback on.
/// \param  callback The function to run on the node's data.
//  ----------------------------------------------------------------------------
static void nodes_run_for_all(node_t *node_p,
                              void (*callback)(void const * const data))
{
    if (node_p == NULL) {
        return;
    } else {
        if (node_p->data == NULL) {
            fprintf(stderr, "%s: data pointer is NULL.\n", __func__);
        }
        callback(node_p->data);
        nodes_run_for_all(node_p->next, callback);
    }
}


//  ----------------------------------------------------------------------------
/// \brief  Allocate memory for new data (content copied from src) and a new
/// node. Call itself to copy the next node of src (and populate the current
/// .next with the returned value).
/// \param  src Node to copy from.
/// \param  data_size Data block size.
/// \return Pointer to the newly created node.
//  ----------------------------------------------------------------------------
static node_t *nodes_recursive_copy(node_t * const src,
                                    size_t const data_size)
{
    if (src == NULL) {
        return NULL;
    }

    // Allocate a new data object and copy.
    void *new_data = malloc(data_size);
    if (new_data != NULL) {
        memcpy(new_data, src->data, data_size);
    }

    // Create a whole new node.
    node_t *new_node_p = malloc(sizeof (node_t));
    *new_node_p = (node_t) {
        .data = new_data,
        .next = nodes_recursive_copy(src->next, data_size)
    };

    return new_node_p;
}


//  ----------------------------------------------------------------------------
/// \brief  Compare the data of the current node, quit and return false if they
/// differ, otherwise keep on compare the next node until reaching the end.
/// \param  a   First node to compare.
/// \param  b   Second node to compare.
/// \param  data_size   The size of the data being pointed to by each node.
/// \return True if the rest of the lists are equal.
//  ----------------------------------------------------------------------------
static bool nodes_recursive_compare(node_t * const a, node_t * const b,
                                    size_t const data_size)
{
    if (a == NULL) {
        if (b != NULL) {
            return false;
        }
        return true;
    }

    if (memcmp(a->data, b->data, data_size) != 0) {
        return false;
    } else {
        return nodes_recursive_compare(a->next, b->next, data_size);
    }
}


// ----------------------------------------------------------------------------
/// \brief Walk pos number of nodes from start. If the tail of a list is
/// reached, go on from head (wrap around).
/// \param  start   The first node.
/// \param  pos     Number of steps to take. May be negative.
/// \return Pointer to the target node.
//  ----------------------------------------------------------------------------
static node_t *nodes_walker(node_t * const start, int const pos)
{
    node_t *walker = start;
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


//  ----------------------------------------------------------------------------
/// \brief  Truncate the list after position limit.
/// \param  list    The list to truncate.
/// \param  limit   The max size of the resulting list.
//  ----------------------------------------------------------------------------
static void length_limit(linkedlist_t * const list, int const limit)
{
    if (limit <= list->size) {
        return;
    }

    node_t *walker = nodes_walker(list->head, limit - 1);
    nodes_recursive_destroy(walker->next);
    walker->next = NULL;
    list->size = limit;
}
