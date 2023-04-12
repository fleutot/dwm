#ifndef LINKEDLIST_H_INCLUDED
#define LINKEDLIST_H_INCLUDED

#include <stdbool.h>
#include <stddef.h>

// Sizes are saved as int, make sure this MAX_SIZE is under INT_MAX.
#define LIST_MAX_SIZE (5000U)

struct ll_node;

// TODO: this struct should not be exposed, but kept private in the c file, and
// only anonymous here. Exposing it now for hacking other functions
// (e.g. drawbar), because the need should disappear when tagviews are
// implemented. When that occurs, the struct ll_node definition can move back to
// the c file.
struct ll_node {
	void *data;
	struct ll_node *next;
};

struct list {
	int size;
	struct ll_node *head;
	// An ll_node* is maybe not very interesting, would pointer to the
	// selected *data be more interesting? A pointer to the ll_node is
	// nice, to add a new client to the list.
	struct ll_node *selected;
};

#define LIST_EMPTY (struct list) {  \
		.size = 0,                              \
		.head = NULL,                       \
		.selected = NULL                    \
}

//  ----------------------------------------------------------------------------
/// \brief  Link a new element at the end of the destination list.
/// \param  l  Destination list.
/// \param  data  Pointer to the data content of the new node.
/// \attention  The data object pointed to by data must be allocated
/// dynamically. Addresses to auto or global variables may not be used.
//  ----------------------------------------------------------------------------
void list_add(struct list *l, void *data);

/// \brief  Link a new element at the start of the destination list.
/// \param  list  Destination list.
/// \param  data  Pointer to the data content of the new node.
/// \attention  The data object pointed to by data must be allocated
/// dynamically. Addresses to auto or global variables may not be used.
//  ----------------------------------------------------------------------------
void list_prepend(struct list *list, void *data);

//  ----------------------------------------------------------------------------
/// \brief  Link a new element at the place of the `at` element.
/// \param  l  Destination list.
/// \param  at  Data pointer to add at. Comparison done on pointer value, and
/// not for the node itself.
/// \param  data Pointer to the data content of the new node.
/// \attention  The data object pointed to by data must be allocated
/// dynamically. Addresses to auto or global variables may not be used.
//  ----------------------------------------------------------------------------
void list_add_before(struct list *l, void *at, void *data);
void list_add_before_selected(struct list *l, void *data);

void list_rm(struct list *l, void *data);

/// ----------------------------------------------------------------------------
/// \brief Pop the last element of the list. The list node gets freed,
/// but the caller is responsible for freeing the data itself.
/// \param  list The list to pop from.
/// \return A pointer to the data, so that the caller can free it.
// ----------------------------------------------------------------------------
void *list_pop(struct list *l);

// ----------------------------------------------------------------------------
/// \brief Destroy the list passed as parameter. Only the list and its nodes are
/// destroyed, the data pointed to by each node needs to be destroyed separately
/// (probably before destroying the list itself).
/// \param list The list to destroy.
// ----------------------------------------------------------------------------
void list_destroy(struct list *l);

//  ----------------------------------------------------------------------------
/// \brief  Run the callback function passed as parameter on the data of all
/// nodes in the list passed as parameter. The callback may modify the data,
/// since the data itself is held by the client module.
/// \param  list The list to run the callback on.
/// \param  callback Function pointer to the function to run on data
/// in each list element.
/// \param  storage Extra data to give the callback.
//  ----------------------------------------------------------------------------
void list_run_for_all(
	struct list *l,
	void (*callback)(void *data, void *storage),
	void *storage);

//  ----------------------------------------------------------------------------
/// \brief Find the data for which the callback is true.
/// \param  list The list to traverse.
/// \param  callback The function that returns true if found
/// \param  storage Extra data passed to callback
/// \return Pointer to the data found. NULL if not found.
//  ----------------------------------------------------------------------------
void *list_find(
	struct list *l,
	bool (*callback)(void *data, void *storage),
	void *storage);

void list_select(struct list *list, const void *data);
void *list_head_select(struct list *list);
void *list_tail_select(struct list *l);
void *list_next_select(struct list *l);
void *list_prev_select(struct list *l);
void *list_next_wrap_select(struct list *l);
void *list_prev_wrap_select(struct list *l);
void *list_selected_data_get(struct list *l);

//  ----------------------------------------------------------------------------
/// \brief swap the data pointers between to elements of the list.
//  ----------------------------------------------------------------------------
void list_data_swap(struct list *list, void *a, void *b);

//  ----------------------------------------------------------------------------
/// \brief  Get the pointer to the data of the node at position.
/// \param  list The list to explore.
/// \param  position The index to the node of interest.
/// \return NULL if invalid position, pointer to the data otherwise.
//  ----------------------------------------------------------------------------
void *list_data_handle_get(
	struct list *l,
	unsigned int const position);

//  ----------------------------------------------------------------------------
/// \brief Get the size of the list passed as parameter.
//  ----------------------------------------------------------------------------
int list_size_get(const struct list *l);

#endif // LINKEDLIST_H_INCLUDED
