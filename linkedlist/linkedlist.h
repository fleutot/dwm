/*----------------------------------------------------------------------------
Copyright (c) 2013 Gauthier Fleutot Ostervall
----------------------------------------------------------------------------*/

#ifndef LINKEDLIST_H_INCLUDED
#define LINKEDLIST_H_INCLUDED

#include <stdbool.h>
#include <stddef.h>

// Sizes are saved as int, make sure this MAX_SIZE is under INT_MAX.
#define LINKEDLIST_MAX_SIZE (5000U)

// Do not defin your own ll_node variables, use the function linkedlist_add().
struct ll_node;

//  ----------------------------------------------------------------------------
/// \brief  Link a new element at the end of the destination list.
/// \param  dst Destination list.
/// \param  data Pointer to the data content of the new node.
/// \return The updated list. This is useful if dst was empty before calling
/// this function.
/// \attention  The data object pointed to by data must be allocated
/// dynamically. Addresses to auto or global variables may not be used.
//  ----------------------------------------------------------------------------
struct ll_node *linkedlist_add(struct ll_node *dst, void *data);

// ----------------------------------------------------------------------------
/// \brief Destroy the list passed as parameter. Only the list and its nodes are
/// destroyed, the data pointed to by each node needs to be destroyed separately
/// (probably before destroying the list itself).
/// \param list The list to destroy.
// ----------------------------------------------------------------------------
void linkedlist_destroy(struct ll_node *list);

//  ----------------------------------------------------------------------------
/// \brief  Run the callback function passed as parameter on the data of all
/// nodes in the list passed as parameter. The callback may modify the data,
/// since the data itself is held by the client module.
/// \param  list The list to run the callback on.
/// \param  callback Function pointer to the function to run on data.
//  ----------------------------------------------------------------------------
void linkedlist_run_for_all(struct ll_node *list,
                            void (*callback)(void *data));

//  ----------------------------------------------------------------------------
/// \brief  Get the pointer to the data of the node at position. If position
/// goes beyond the number of elemnts of list, wrap around (go on form head
/// after reaching tail).
/// \param  list The list to explore.
/// \param  position The index to the node of interest.
//  ----------------------------------------------------------------------------
void *linkedlist_data_handle_get(struct ll_node * const list,
                                 unsigned int const position);


//  ----------------------------------------------------------------------------
/// \brief Get the size of the list passed as parameter.
//  ----------------------------------------------------------------------------
int linkedlist_size_get(struct ll_node * const list);

#endif // LINKEDLIST_H_INCLUDED
