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
static void list_populate(struct ll_node **list,
                          int data[],
                          int number_of_elements);
static void list_read_to_array_reset(void);
static void list_read_to_array(void *a);
static bool int_arrays_equal(int const * const a,
                             int const * const b,
                             const int size);

// Test functions.
static void test_linkedlist_run_for_all(void);
static void test_linkedlist_data_handle_get(void);

//******************************************************************************
// Function definitions
//******************************************************************************
int main(void)
{
    test_linkedlist_run_for_all();
    test_linkedlist_data_handle_get();
    printf("All tests passed.\n");
}


//******************************************************************************
// Internal functions
//******************************************************************************
static void test_linkedlist_run_for_all(void)
{
    TEST_START_PRINT();
    int data[] = {1, 2, 3, 4, 5};

    struct ll_node *list = NULL;
    list_populate(&list, data, NB_ELEMENTS(data));

    list_read_to_array_reset();
    linkedlist_run_for_all(list, list_read_to_array);

    assert(int_arrays_equal(data, read_array, NB_ELEMENTS(data)));

    linkedlist_destroy(list);
    TEST_END_PRINT();
}

static void test_linkedlist_data_handle_get(void)
{
    TEST_START_PRINT();
    int data[] = {11, 12, 13, 14, 15};
    unsigned int pos = 3;

    struct ll_node *list = NULL;
    list_populate(&list, data, NB_ELEMENTS(data));

    int *result_ptr = (int *) linkedlist_data_handle_get(list, pos);

    assert(result_ptr != NULL);
    assert(*result_ptr == data[pos]);

    // Test the wrap around functionality.
    const unsigned int other_pos = 2;
    result_ptr = (int *) linkedlist_data_handle_get(
        list,
        other_pos + NB_ELEMENTS(data)
        );

    assert(result_ptr != NULL);
    assert(*result_ptr == data[other_pos]);

    linkedlist_destroy(list);
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
static void list_populate(struct ll_node **list,
                          int data[],
                          int number_of_elements)
{
    for (int i = 0; i < number_of_elements; i++) {
        int *new_data_object = malloc(sizeof(int));

        *new_data_object = data[i];
        *list = linkedlist_add(*list, new_data_object);
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
