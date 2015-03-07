#ifndef _SORTED_LIST_H_
#define _SORTED_LIST_H_

#include <stdbool.h>

/*
 * Compare function for two list objects. Implemented by the caller.
 */
typedef int compare_function_t(void *, void *);

/*
 * Destroy function for a list object. Implemented by the caller.
 */
typedef void destroy_function_t(void *);

/*
 * An element in a sorted list.
 */
typedef struct sorted_list_element {
    struct sorted_list_element *next;
    void *value;
} list_element_t;

/*
 * A sorted list.
 */
typedef struct sorted_list {
    compare_function_t *compare_function;
    destroy_function_t *destroy_function;
    list_element_t *head;
} list_t;

/*
 * Creates a sorted list. The caller is responsible for freeing
 * the allocated memory using destroy_list.
 */
list_t *create_list(compare_function_t *, destroy_function_t *);

/*
 * Destroys a sorted list.
 */
void destroy_list(list_t *);

/*
 * Creates an element for a sorted_list. The caller is responsible for freeing
 * the allocated memory.
 */
list_element_t *create_list_element(void *, list_element_t *);

/*
 * Destroys a list element.
 */
void destroy_list_element(list_t *, list_element_t *);

/*
 * Insert function for a list_t.
 */
bool insert_object(list_t *, void *);

/*
 * Remove function for a list_t.
 */
bool remove_object(list_t *, void *);

/*
 * Checks if a list contains the given object.
 */
bool contains_object(list_t *, void *);

/*
 * Gets the current size of the list.
 */
int get_size(list_t *);

/*
 * An iterator for a sorted list.
 */
typedef struct sorted_list_iterator {
    list_element_t *current_index;
} list_iterator_t;

/*
 * Creates a sorted list iterator, given the list.
 */
list_iterator_t *create_iterator(list_t *);

/*
 * Destroys a sorted list iterator.
 */
void destroy_iterator(list_iterator_t *);

/*
 * Gets the current item in the sorted list iterator,
 * and advances to the next index.
 */
void *get_item(list_iterator_t *);

/*
 * Gets the next item in the sorted list iterator.
 */
void *next_item(list_iterator_t *);

#endif