/*----------------------------------------------------------------------------
Copyright (c) 2013 Gauthier Fleutot Ostervall
----------------------------------------------------------------------------*/

#ifndef LINKEDLIST_H_INCLUDED
#define LINKEDLIST_H_INCLUDED

#include <stdbool.h>
#include <stddef.h>

// Sizes are saved as int, make sure this MAX_SIZE is under INT_MAX.
#define LINKEDLIST_MAX_SIZE (5000U)

struct ll_node;

struct linkedlist {
    int size;
    struct ll_node *head;
};

//  ----------------------------------------------------------------------------
/// \brief  Link a new element at the end of the destination list.
/// \param  l  Destination list.
/// \param  data  Pointer to the data content of the new node.
/// \attention  The data object pointed to by data must be allocated
/// dynamically. Addresses to auto or global variables may not be used.
//  ----------------------------------------------------------------------------
void linkedlist_add(struct linkedlist *l, void *data);

//  ----------------------------------------------------------------------------
/// \brief  Link a new element at the place of the `at` element.
/// \param  l  Destination list.
/// \param  at  Data pointer to add at. Comparison done on pointer value, and
/// not for the node itself.
/// \param  data Pointer to the data content of the new node.
/// \attention  The data object pointed to by data must be allocated
/// dynamically. Addresses to auto or global variables may not be used.
//  ----------------------------------------------------------------------------
void linkedlist_add_before(struct linkedlist *l, void *at, void *data);

// ----------------------------------------------------------------------------
/// \brief Destroy the list passed as parameter. Only the list and its nodes are
/// destroyed, the data pointed to by each node needs to be destroyed separately
/// (probably before destroying the list itself).
/// \param list The list to destroy.
// ----------------------------------------------------------------------------
void linkedlist_destroy(struct linkedlist *l);

//  ----------------------------------------------------------------------------
/// \brief  Run the callback function passed as parameter on the data of all
/// nodes in the list passed as parameter. The callback may modify the data,
/// since the data itself is held by the client module.
/// \param  list The list to run the callback on.
/// \param  callback Function pointer to the function to run on data.
//  ----------------------------------------------------------------------------
void linkedlist_run_for_all(struct linkedlist *l,
                            void (*callback)(void *data));

//  ----------------------------------------------------------------------------
/// \brief  Get the pointer to the data of the node at position. If position
/// goes beyond the number of elemnts of list, wrap around (go on form head
/// after reaching tail).
/// \param  list The list to explore.
/// \param  position The index to the node of interest.
//  ----------------------------------------------------------------------------
void *linkedlist_data_handle_get(struct linkedlist *l,
                                 unsigned int const position);


//  ----------------------------------------------------------------------------
/// \brief Get the size of the list passed as parameter.
//  ----------------------------------------------------------------------------
int linkedlist_size_get(struct linkedlist *l);

#endif // LINKEDLIST_H_INCLUDED
