// Module under test.
#include "../linkedlist.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <malloc.h>

//******************************************************************************
// Module macros
//******************************************************************************
// Number of elements in array x.
#define NB_ELEMENTS(x) (sizeof(x) / sizeof(x[0]))

#define TEST_START_PRINT()    do {                      \
		printf("Running %s...", __func__);      \
} while (0)

#define TEST_END_PRINT()  do {                  \
		printf("OK.\n");                \
} while (0)


//******************************************************************************
// Module constants
//******************************************************************************

//******************************************************************************
// Module variables
//******************************************************************************
// read_array for reading back list data.
static int read_array[20];
static unsigned int read_array_current_index;

//******************************************************************************
// Function prototypes
//******************************************************************************
// Helper functions.
static void display(void const * const a);
static void populate(
	struct list *list,
	int *data_objects[],
	int number_of_elements);
static void read_to_array_reset(void);
static void read_to_array(void *data, void *storage);
static bool int_arrays_equal(
	int const * const a,
	int const * const b,
	const int size);

// Test functions.
static void test_list_run_for_all(void);
static void test_list_data_handle_get(void);
static void test_list_add_before(void);
static void test_list_rm(void);
static void test_list_next_prev(void);
static void test_list_pop(void);

//******************************************************************************
// Function definitions
//******************************************************************************
int main(void)
{
	test_list_run_for_all();
	test_list_data_handle_get();
	test_list_add_before();
	test_list_rm();
	test_list_next_prev();
	test_list_pop();
	printf("All tests passed.\n");
}


//******************************************************************************
// Internal functions
//******************************************************************************
static void test_list_run_for_all(void)
{
	TEST_START_PRINT();
	int data[] = { 1, 2, 3, 4, 5 };
	int *data_objects[NB_ELEMENTS(data)];
	// lists requires dynamic allocation for their objects.
	for (int i = 0; i < NB_ELEMENTS(data); i++) {
		data_objects[i] = malloc(sizeof(data[0]));
		*data_objects[i] = data[i];
	}

	struct list list = {
		.head = NULL,
		.size = 0
	};

	populate(&list, data_objects, NB_ELEMENTS(data));

	read_to_array_reset();
	list_run_for_all(&list, read_to_array, NULL);

	assert(int_arrays_equal(data, read_array, NB_ELEMENTS(data)));

	list_destroy(&list);
	TEST_END_PRINT();
}

static void test_list_data_handle_get(void)
{
	TEST_START_PRINT();
	int data[] = { 11, 12, 13, 14, 15 };
	int *data_objects[NB_ELEMENTS(data)];
	// lists requires dynamic allocation for their objects.
	for (int i = 0; i < NB_ELEMENTS(data); i++) {
		data_objects[i] = malloc(sizeof(data[0]));
		*data_objects[i] = data[i];
	}

	unsigned int pos = 3;

	struct list list = {
		.head = NULL,
		.size = 0
	};

	populate(&list, data_objects, NB_ELEMENTS(data));

	int *result_ptr = (int *) list_data_handle_get(&list, pos);

	assert(result_ptr != NULL);
	assert(*result_ptr == data[pos]);

	// Test the wrap around functionality.
	const unsigned int other_pos = 2;
	result_ptr = (int *) list_data_handle_get(
		&list,
		other_pos + NB_ELEMENTS(data)
		);

	assert(result_ptr != NULL);
	assert(*result_ptr == data[other_pos]);

	list_destroy(&list);
	TEST_END_PRINT();
}

static void test_list_add_before(void)
{
	TEST_START_PRINT();
	int data[] = { 1, 2, 3, 4, 5 };
	int *data_objects[NB_ELEMENTS(data)];
	// lists requires dynamic allocation for their objects.
	for (int i = 0; i < NB_ELEMENTS(data); i++) {
		data_objects[i] = malloc(sizeof(data[0]));
		*data_objects[i] = data[i];
	}

	struct list list = {
		.head = NULL,
		.size = 0
	};
	populate(&list, data_objects, NB_ELEMENTS(data));

	int extra_data = 42;
	int *extra_data_object = malloc(sizeof(extra_data));
	*extra_data_object = extra_data;

	list_add_before(&list, data_objects[2], extra_data_object);

	read_to_array_reset();
	list_run_for_all(&list, read_to_array, NULL);

	assert(int_arrays_equal((int[]) { 1, 2, 42, 3, 4, 5 }, read_array, NB_ELEMENTS(data) + 1));

	list_destroy(&list);
	TEST_END_PRINT();
}

static void test_list_rm(void)
{
	TEST_START_PRINT();
	int data[] = { 1, 2, 3, 4, 5 };
	int *data_objects[NB_ELEMENTS(data)];
	// lists requires dynamic allocation for their objects.
	for (int i = 0; i < NB_ELEMENTS(data); i++) {
		data_objects[i] = malloc(sizeof(data[0]));
		*data_objects[i] = data[i];
	}

	struct list list = {
		.head = NULL,
		.size = 0
	};
	populate(&list, data_objects, NB_ELEMENTS(data));

	// Remove first
	list_rm(&list, data_objects[0]);
	read_to_array_reset();
	list_run_for_all(&list, read_to_array, NULL);
	assert(int_arrays_equal((int[]) { 2, 3, 4, 5 }, read_array, list.size));

	// Remove in the middle
	list_rm(&list, data_objects[2]);
	read_to_array_reset();
	list_run_for_all(&list, read_to_array, NULL);
	assert(int_arrays_equal((int[]) { 2, 4, 5 }, read_array, list.size));

	// Remove last
	list_rm(&list, data_objects[4]);
	read_to_array_reset();
	list_run_for_all(&list, read_to_array, NULL);
	assert(int_arrays_equal((int[]) { 2, 4 }, read_array, list.size));

	// Try to remove data not in list
	int *external_data = malloc(sizeof(*external_data));
	list_rm(&list, external_data);
	read_to_array_reset();
	list_run_for_all(&list, read_to_array, NULL);
	assert(int_arrays_equal((int[]) { 2, 4 }, read_array, list.size));

	// Remove all
	list_rm(&list, data_objects[1]);
	list_rm(&list, data_objects[3]);
	assert(list_size_get(&list) == 0);
	assert(list.head == NULL);
	assert(list.selected == NULL);

	TEST_END_PRINT();
}

static void test_list_next_prev(void)
{
	TEST_START_PRINT();
	int data[] = { 1, 2, 3 };

	struct list list = LIST_EMPTY;

	for (int i = 0; i < NB_ELEMENTS(data); i++) {
		list_add(&list, &data[i]);
	}

	int *read_back;

	read_back = list_next_select(&list);
	assert(*read_back == 1);

	read_back = list_next_select(&list);
	assert(*read_back == 2);

	read_back = list_next_select(&list);
	assert(*read_back == 3);

	read_back = list_next_select(&list);
	assert(read_back == NULL);
	assert(*(int *) (list_selected_data_get(&list)) == 3);

	read_back = list_prev_select(&list);
	assert(*read_back == 2);

	read_back = list_prev_select(&list);
	assert(*read_back == 1);

	struct list empty_list = LIST_EMPTY;
	read_back = list_next_select(&empty_list);
	assert(read_back == NULL);

	struct list empty_list_2 = LIST_EMPTY;
	read_back = list_prev_select(&empty_list_2);
	assert(read_back == NULL);

	struct list short_list = LIST_EMPTY;
	list_add(&short_list, &data[0]);
	read_back = list_next_select(&short_list);
	assert(*read_back == 1);

	struct list short_list_2 = LIST_EMPTY;
	list_add(&short_list_2, &data[0]);
	read_back = list_prev_select(&short_list_2);
	assert(*read_back == 1);

	TEST_END_PRINT();
}

static void test_list_pop(void)
{
	TEST_START_PRINT();
	int data[] = { 1, 2, 3, 4, 5 };

	struct list list = LIST_EMPTY;

	for (int i = 0; i < NB_ELEMENTS(data); i++) {
		list_add(&list, &data[i]);
	}

	assert(list.size == NB_ELEMENTS(data));

	int *popped = list_pop(&list);
	assert(list.size == NB_ELEMENTS(data) - 1);
	assert(popped == &data[NB_ELEMENTS(data) - 1]);

	read_to_array_reset(); /* sets all elements to 0 */
	list_run_for_all(&list, read_to_array, NULL);
	assert(int_arrays_equal(
		       (int[]) { 1, 2, 3, 4, 0 },
		       read_array,
		       NB_ELEMENTS(data)));

	popped = list_pop(&list);
	assert(*popped == 4);
	assert(list.size == 3);

	popped = list_pop(&list);
	assert(*popped == 3);
	assert(list.size == 2);

	popped = list_pop(&list);
	assert(*popped == 2);
	assert(list.size == 1);

	popped = list_pop(&list);
	assert(*popped == 1);
	assert(list.size == 0);
	assert(list.head == NULL);

	TEST_END_PRINT();
}

//------------------------------------------------------------------------------
// Helper functions
//------------------------------------------------------------------------------

//  ----------------------------------------------------------------------------
/// \brief  Reset the function that puts the content of a list to an array.
//  ----------------------------------------------------------------------------
static void read_to_array_reset(void)
{
	for (unsigned int i = 0; i < NB_ELEMENTS(read_array); i++) {
		read_array[i] = 0;
	}
	read_array_current_index = 0;
}

// ----------------------------------------------------------------------------
/// \brief Put the content of a list to an array. Needs resetting first (see
/// read_to_array_reset()). This function is meant to be used as a
/// parameter of list_run_for_all, hence the parameter format.
/// \param  data Pointer to the data of a node.
/// \param  storage Pointer to extra data, passed directly from the caller.
//  ----------------------------------------------------------------------------
static void read_to_array(void *data, void *storage)
{
	read_array[read_array_current_index] = *(int *) data;
	read_array_current_index++;

	if (read_array_current_index >= NB_ELEMENTS(read_array)) {
		fprintf(stderr, "%s: index out of range.\n", __func__);
		read_array_current_index = 0;
	}
}

// ----------------------------------------------------------------------------
/// \brief Display the content of the data of a node. This function is meant to
/// be used as a parameter of / list_run_for_all, hence the parameter
/// format.
/// \param  Pointer to the data of a node.
//  ----------------------------------------------------------------------------
static void display(void const * const a)
{
	printf("d: %d\n", *(int *) a);
}

//  ----------------------------------------------------------------------------
/// \brief  Populate an existing list with the content of an array.
/// \param  list The list to populate. Normally empty at the beginning.
/// \param  data The actual data array to fill the list with.
/// \param  number_of_elements The size of the data array.
//  ----------------------------------------------------------------------------
static void populate(struct list *list,
		     int *data_objects[],
		     int number_of_elements)
{
	for (int i = 0; i < number_of_elements; i++) {
		list_add(list, data_objects[i]);
	}
}

//  ----------------------------------------------------------------------------
/// \brief  Check if two arrays have the same content.
/// \param  a   Pointer to one of the array to compare.
/// \param  b   Pointer to one of the array to compare.
/// \param  size    Number of elements in the arrays to compare.
/// \return True if the arrays had the same content.
//  ----------------------------------------------------------------------------
static bool int_arrays_equal(int const * const a,
			     int const * const b,
			     const int size)
{
	for (int i = 0; i < size; i++) {
		if (a[i] != b[i]) {
			return false;
		}
	}
	return true;
}
