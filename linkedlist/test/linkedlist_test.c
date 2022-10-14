/*----------------------------------------------------------------------------
Copyright (c) 2013 Gauthier Fleutot Ostervall
----------------------------------------------------------------------------*/

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
#define NB_ELEMENTS(x) (sizeof (x) / sizeof (x[0]))

#define TEST_START_PRINT()    do {              \
        printf("Running %s...", __func__);      \
    } while (0)

#define TEST_END_PRINT()  do {                  \
        printf("OK.\n");                        \
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
static void list_populate(struct linkedlist *list,
                          int *data_objects[],
                          int number_of_elements);
static void list_read_to_array_reset(void);
static void list_read_to_array(void *a);
static bool int_arrays_equal(int const * const a,
                             int const * const b,
                             const int size);

// Test functions.
static void test_linkedlist_run_for_all(void);
static void test_linkedlist_data_handle_get(void);
static void test_linkedlist_add_before(void);
static void test_linkedlist_rm(void);

//******************************************************************************
// Function definitions
//******************************************************************************
int main(void)
{
    test_linkedlist_run_for_all();
    test_linkedlist_data_handle_get();
    test_linkedlist_add_before();
    test_linkedlist_rm();
    printf("All tests passed.\n");
}


//******************************************************************************
// Internal functions
//******************************************************************************
static void test_linkedlist_run_for_all(void)
{
    TEST_START_PRINT();
    int data[] = {1, 2, 3, 4, 5};
    int *data_objects[NB_ELEMENTS(data)];
    // linkedlists requires dynamic allocation for their objects.
    for (int i = 0; i < NB_ELEMENTS(data); i++) {
        data_objects[i] = malloc(sizeof(data[0]));
        *data_objects[i] = data[i];
    }

    struct linkedlist list = {
        .head = NULL,
        .size = 0
    };

    list_populate(&list, data_objects, NB_ELEMENTS(data));

    list_read_to_array_reset();
    linkedlist_run_for_all(&list, list_read_to_array);

    assert(int_arrays_equal(data, read_array, NB_ELEMENTS(data)));

    linkedlist_destroy(&list);
    TEST_END_PRINT();
}

static void test_linkedlist_data_handle_get(void)
{
    TEST_START_PRINT();
    int data[] = {11, 12, 13, 14, 15};
    int *data_objects[NB_ELEMENTS(data)];
    // linkedlists requires dynamic allocation for their objects.
    for (int i = 0; i < NB_ELEMENTS(data); i++) {
        data_objects[i] = malloc(sizeof(data[0]));
        *data_objects[i] = data[i];
    }

    unsigned int pos = 3;

    struct linkedlist list = {
        .head = NULL,
        .size = 0
    };

    list_populate(&list, data_objects, NB_ELEMENTS(data));

    int *result_ptr = (int *) linkedlist_data_handle_get(&list, pos);

    assert(result_ptr != NULL);
    assert(*result_ptr == data[pos]);

    // Test the wrap around functionality.
    const unsigned int other_pos = 2;
    result_ptr = (int *) linkedlist_data_handle_get(
        &list,
        other_pos + NB_ELEMENTS(data)
        );

    assert(result_ptr != NULL);
    assert(*result_ptr == data[other_pos]);

    linkedlist_destroy(&list);
    TEST_END_PRINT();
}

static void test_linkedlist_add_before(void)
{
    TEST_START_PRINT();
    int data[] = {1, 2, 3, 4, 5};
    int *data_objects[NB_ELEMENTS(data)];
    // linkedlists requires dynamic allocation for their objects.
    for (int i = 0; i < NB_ELEMENTS(data); i++) {
        data_objects[i] = malloc(sizeof(data[0]));
        *data_objects[i] = data[i];
    }

    struct linkedlist list = {
        .head = NULL,
        .size = 0
    };
    list_populate(&list, data_objects, NB_ELEMENTS(data));

    int extra_data = 42;
    int *extra_data_object = malloc(sizeof(extra_data));
    *extra_data_object = extra_data;

    linkedlist_add_before(&list, data_objects[2], extra_data_object);

    list_read_to_array_reset();
    linkedlist_run_for_all(&list, list_read_to_array);

    assert(int_arrays_equal((int[]) {1, 2, 42, 3, 4, 5}, read_array, NB_ELEMENTS(data) + 1));

    linkedlist_destroy(&list);
    TEST_END_PRINT();
}

static void test_linkedlist_rm(void)
{
    TEST_START_PRINT();
    int data[] = {1, 2, 3, 4, 5};
    int *data_objects[NB_ELEMENTS(data)];
    // linkedlists requires dynamic allocation for their objects.
    for (int i = 0; i < NB_ELEMENTS(data); i++) {
        data_objects[i] = malloc(sizeof(data[0]));
        *data_objects[i] = data[i];
    }

    struct linkedlist list = {
        .head = NULL,
        .size = 0
    };
    list_populate(&list, data_objects, NB_ELEMENTS(data));

    // Remove first
    linkedlist_rm(&list, data_objects[0]);
    list_read_to_array_reset();
    linkedlist_run_for_all(&list, list_read_to_array);
    assert(int_arrays_equal((int[]) {2, 3, 4, 5}, read_array, list.size));

    // Remove in the middle
    linkedlist_rm(&list, data_objects[2]);
    list_read_to_array_reset();
    linkedlist_run_for_all(&list, list_read_to_array);
    assert(int_arrays_equal((int[]) {2, 4, 5}, read_array, list.size));

    // Remove last
    linkedlist_rm(&list, data_objects[4]);
    list_read_to_array_reset();
    linkedlist_run_for_all(&list, list_read_to_array);
    assert(int_arrays_equal((int[]) {2, 4}, read_array, list.size));

    // Try to remove data not in list
    int *external_data = malloc(sizeof(*external_data));
    linkedlist_rm(&list, external_data);
    list_read_to_array_reset();
    linkedlist_run_for_all(&list, list_read_to_array);
    assert(int_arrays_equal((int[]) {2, 4}, read_array, list.size));

    TEST_END_PRINT();
}


//------------------------------------------------------------------------------
// Helper functions
//------------------------------------------------------------------------------

//  ----------------------------------------------------------------------------
/// \brief  Reset the function that puts the content of a list to an array.
//  ----------------------------------------------------------------------------
static void list_read_to_array_reset(void)
{
    for (unsigned int i = 0; i < NB_ELEMENTS(read_array); i++) {
        read_array[i] = 0;
    }
    read_array_current_index = 0;
}

// ----------------------------------------------------------------------------
/// \brief Put the content of a list to an array. Needs resetting first (see
/// list_read_to_array_reset()). This function is meant to be used as a
/// parameter of linkedlist_run_for_all, hence the parameter format.
/// \param  Pointer to the data of a node.
//  ----------------------------------------------------------------------------
static void list_read_to_array(void *a)
{
    read_array[read_array_current_index] = * (int *) a;
    read_array_current_index++;

    if (read_array_current_index >= NB_ELEMENTS(read_array)) {
        fprintf(stderr, "list_read_to_array: index out of range.\n");
        read_array_current_index = 0;
    }
}

// ----------------------------------------------------------------------------
/// \brief Display the content of the data of a node. This function is meant to
/// be used as a parameter of / linkedlist_run_for_all, hence the parameter
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
static void list_populate(struct linkedlist *list,
                          int *data_objects[],
                          int number_of_elements)
{
    for (int i = 0; i < number_of_elements; i++) {
        linkedlist_add(list, data_objects[i]);
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
