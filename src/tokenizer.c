#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <tic.h>
#include "tokenizer.h"

/*
 * Flag for whether a given character is valid for this tokenizer.
 */
static bool is_valid(char c) {
    return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
}

/*
 * Compare function for token entries in a list.
 */
static int compare_function(void *first, void *second) {
    return 1; /* we don't care about the order here */
}

/*
 * Destroy function for token entries in a list.
 */
static void destroy_function(void *string) {
    free(string);
}

/*
 * Allocates memory for and copies a token given a pointer to the start
 * of the token and a pointer to the end. The caller is responsible for
 * freeing the allocated memory.
 */
static char *create_token(char *start_index, char *current_index) {
    size_t token_size = current_index - start_index;
    if (token_size == 0) {
        return NULL;
    }
    char *token = malloc(token_size + sizeof(char));
    memcpy(token, start_index, token_size);
    token[token_size] = '\0';
    return token;
}

/*
 * Tokenizes a string and returns the token in a sorted linked list.
 */
list_t *tokenize(char *string) {
    list_t *list = create_list(&compare_function, &destroy_function);
    if (list == NULL) {
        fprintf(stderr, "Error creating list! Not enough memory?\n");
        return NULL;
    }
    char *start_index = string;
    char *current_index = string;
    char *end_index = string + strlen(string);
    while (current_index != end_index) {
        if (is_valid(*current_index)) {
            /* character is valid, so we keep searching */
            current_index += sizeof(char);
            if (current_index == end_index) {
                /* we've hit the end of the string, so we need to insert the last token */
                char *token = create_token(start_index, current_index);
                if (token != NULL) {
                    insert_object(list, token);
                }
            }
        } else {
            /* we've found an invalid character, so we need to insert the token right before it */
            char *token = create_token(start_index, current_index);
            if (token != NULL) {
                insert_object(list, token);
            }
            current_index += sizeof(char);
            start_index = current_index;
        }
    }
    return list;
}