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
enum placement {
	BEFORE,
	AFTER
};

//******************************************************************************
// Module variables
//******************************************************************************

//******************************************************************************
// Function prototypes
//******************************************************************************
static struct ll_node *nodes_walker(struct ll_node *start, int pos);

static void list_insert_node(
	struct list *list,
	enum placement placement,
	struct ll_node *at,
	void *data);

//******************************************************************************
// Function definitions
//******************************************************************************

//  ----------------------------------------------------------------------------
/// @brief  Add a new node  at the end of a linked list, with content
/// data.
/// @attention  The data object must be dynamically allocated since the list's
/// remove functions use free() on data objects.
//  ----------------------------------------------------------------------------

void list_add(struct list *list, void *data)
{
	list_insert_node(list, BEFORE, NULL, data);
}

void list_prepend(struct list *list, void *data)
{
	list_insert_node(list, BEFORE, list->head, data);
}

void list_add_before(struct list *list, void *at, void *data)
{
	struct ll_node *n;

	for (
		n = list->head;
		n != NULL && n->next != NULL && n->next->data != at;
		n = n->next) {
	}
	list_insert_node(list, AFTER, n, data);
}

static void list_insert_node(
	struct list *list,
	enum placement placement,
	struct ll_node *at,
	void *data)
{
	struct ll_node *node = malloc(sizeof(struct ll_node));

	assert(node);
	*node = (struct ll_node) {
		.data = data,
		.next = NULL
	};

	if (list->head == NULL || list->size == 0) {
		list->head = node;
		list->size = 1;
	} else if ((at == list->head) && (placement == BEFORE)) {
		node->next = list->head;
		list->head = node;
		list->size++;
	} else {
		struct ll_node *n;
		if (placement == BEFORE) {
			// Walk to the last node, or before node `at`.
			for (n = list->head;
			     n->next != NULL && n->next != at;
			     n = n->next) {
			}
		} else {
			// placement == AFTER
			n = at;
		}
		node->next = n->next;
		n->next = node;
		list->size++;
	}
}

void list_rm(struct list *l, void *data)
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
		if (!n || !n->next) {
			// Not found.
			printf("%s: not found\n", __func__);
			return;
		}
		to_remove = n->next;
		n->next = n->next->next;
	}

	printf("%s: removing\n", __func__);

	if (l->selected == to_remove) {
		l->selected = NULL;
	}

	free(to_remove);
	l->size--;
}

void *list_pop(struct list *l)
{
	if (l->head == NULL) {
		return NULL;
	}

	if (l->head->next == NULL) {
		/* Only one element */
		void *data = l->head->data;
		free(l->head);
		l->head = NULL;
		l->size--;
		return data;
	}

	struct ll_node *new_last;
	for (
		new_last = l->head;
		new_last->next->next != NULL;
		new_last = new_last->next) {
	}

	void *data = new_last->next->data;
	free(new_last->next);
	new_last->next = NULL;
	l->size--;
	return data;
}

//  ----------------------------------------------------------------------------
/// \brief  Free all nodes of the list passed as parameter.
//  ----------------------------------------------------------------------------
void list_destroy(struct list *list)
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
void list_run_for_all(
	struct list *list,
	void (*callback) (void *data, void *storage),
	void *storage)
{
	if (list == NULL || list->head == NULL) {
		return;
	}
	for (struct ll_node *n = list->head; n != NULL; n = n->next) {
		callback(n->data, storage);
	}
}

void *list_find(
	struct list *list,
	bool (*callback)(void *data, void *storage),
	void *storage)
{
	struct ll_node *n = list->head;
	void *found_data = NULL;

	while (n != NULL && found_data == NULL) {
		if (callback(n->data, storage))
			found_data = n->data;
		n = n->next;
	}
	return found_data;
}

void list_select(struct list *list, const void *data)
{
	struct ll_node *n;

	for (n = list->head;
	     n != NULL && n->data != data;
	     n = n->next) {
	}

	if (n == NULL) {
		printf("%s: data not found\n", __func__);
		return;
	}
	printf("%s: data found\n", __func__);
	list->selected = n;
}

void *list_head_select(struct list *list)
{
	list->selected = list->head;
	if (list->head == NULL) {
		return NULL;
	}
	return list->head->data;
}

void *list_tail_select(struct list *l)
{
	struct ll_node *n;

	for (n = l->head;
	     n != NULL && n->next != NULL;
	     n = n->next) {
	}

	l->selected = n;
	return l->selected == NULL ? NULL : l->selected->data;
}

void *list_next_select(struct list *list)
{
	if (list->selected == NULL) {
		list->selected = list->head;
	} else if (list->selected->next != NULL) {
		list->selected = list->selected->next;
	}

	return list->selected == NULL ? NULL : list->selected->data;
}

void *list_prev_select(struct list *l)
{
	if (l->head == NULL
	    || l->head->next == NULL
	    || l->selected == NULL) {
		l->selected = l->head;
	} else if (l->head == l->selected) {
		// Don't wrap around if the head is selected. Then
		// there is no previous.
	} else {
		struct ll_node *n;
		for (n = l->head;
		     n->next != l->selected && n->next != NULL;
		     n = n->next) {
		}
		l->selected = n;
	}

	return l->selected == NULL ? NULL : l->selected->data;
}

void *list_next_wrap_select(struct list *l)
{
	if (l->selected && l->selected->next == NULL) {
		return list_head_select(l);
	} else {
		return list_next_select(l);
	}
}

void *list_prev_wrap_select(struct list *l)
{
	if (l->selected && l->selected == l->head) {
		return list_tail_select(l);
	} else {
		return list_prev_select(l);
	}
}

void *list_selected_data_get(struct list *list)
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
void *list_data_handle_get(
	struct list * const list,
	unsigned int const position)
{
	// TODO: this does not really need a walker function, can be
	// done directly here.
	struct ll_node *walker = nodes_walker(list->head, position);

	return walker->data;
}

int list_size_get(const struct list *l)
{
	return l->size;
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
