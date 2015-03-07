#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "index_parser.h"
#include "util.h"

/*
 * Handles a logical 'or' search.
 */
static void handle_or_search(indexer_t *indexer, list_iterator_t *token_iterator, list_t *results) {
    /* we iterate through every token */
    char *token = next_item(token_iterator);
    while (token != NULL) {
        /* we attempt to find an indexer entry for our token */
        indexer_entry_t *entry = get_indexer_entry(indexer, token);
        if (entry != NULL) {
            /* if we have one, we iterate through every record in the entry */
            list_iterator_t *entry_record_iterator = create_iterator(entry->records);
            indexer_entry_record_t *record = get_item(entry_record_iterator);
            while (record != NULL) {
                /* if the record isn't already in our result list, we insert it */
                if (!contains_object(results, record->file_path)) {
                    insert_object(results, strdup(record->file_path));
                }
                record = next_item(entry_record_iterator);
            }
            destroy_iterator(entry_record_iterator);
        }
        token = next_item(token_iterator);
    }
}

/*
 * Checks to see whether or not the given record is mapped
 * to every token in the given list for the given indexer.
 */
static bool check_record(indexer_t *indexer, list_t *tokens, indexer_entry_record_t *record) {
    bool success = true;
    list_iterator_t *token_iterator = create_iterator(tokens);
    /* we skip the first token, it's the command */
    char *token = next_item(token_iterator);
    while (token != NULL) {
        indexer_entry_t *token_entry = get_indexer_entry(indexer, token);
        if (token_entry == NULL) {
            /* we don't have an entry for this token */
            success = false;
            break;
        }
        if(get_indexer_entry_record(token_entry, record->file_path) == NULL) {
            /* the entry for this token doesn't contain our record */
            success = false;
            break;
        }
        token = next_item(token_iterator);
    }
    destroy_iterator(token_iterator);
    return success;
}

/*
 * Handles an 'and' search.
 */
static void handle_and_search(indexer_t *indexer, list_t *tokens, list_t *results) {
    /* we iterate through every entry in the indexer */
    list_iterator_t *entry_iterator = create_iterator(indexer->entries);
    indexer_entry_t *entry = get_item(entry_iterator);
    while (entry != NULL) {
        /* if this entry's token is one of ours, we want to work with it */
        if (contains_object(tokens, entry->token)) {
            /* we iterate through every record in the entry */
            list_iterator_t *record_iterator = create_iterator(entry->records);
            indexer_entry_record_t *record = get_item(record_iterator);
            while (record != NULL) {
                /* we check to see if the record is mapped to every token and isn't already a result */
                if(check_record(indexer, tokens, record) && !contains_object(results, record->file_path)) {
                    insert_object(results, strdup(record->file_path));
                }
                record = next_item(record_iterator);
            }
            destroy_iterator(record_iterator);
        }
        entry = next_item(entry_iterator);
    }
    destroy_iterator(entry_iterator);
}

/*
 * Prints the given search results to the standard out.
 */
static void print_results(list_t *results) {
    list_iterator_t *results_iterator = create_iterator(results);
    char *result = get_item(results_iterator);
    while (result != NULL) {
        printf("[%s]", result);
        result = next_item(results_iterator);
        if (result != NULL) printf(", ");
    }
    printf("\n");
    destroy_iterator(results_iterator);
}

static int compare_function(void *first_object, void *second_object) {
    char *first = first_object;
    char *second = second_object;
    int result = strcmp(first, second);
    return result == 0 ? 0 : (result > 0 ? 1 : -1);
}

static void destroy_function(void *object) {
    free(object);
}

/*
 * Parses and handles user input, given the indexer.
 */
static bool handle_input(indexer_t *indexer, char *input) {
    /* first, we tokenize the input */
    list_t *tokens = tokenize(input, ' ', true);
    list_iterator_t *token_iterator = create_iterator(tokens);
    char *command = get_item(token_iterator);

    list_t *results = create_list(&compare_function, &destroy_function);
    bool success = true;
    /* next, we check to see which command the user entered */
    if (strcmp(command, "sa") == 0) {
        /* time to do an 'and' search */
        handle_and_search(indexer, tokens, results);
    } else if (strcmp(command, "so") == 0) {
        /* time to do an 'or' search */
        handle_or_search(indexer, token_iterator, results);
    } else {
        /* invalid command, we need to tell the user */
        success = false;
    }
    /* if we have any results, we should print them */
    print_results(results);
    destroy_list(results);

    destroy_iterator(token_iterator);
    destroy_list(tokens);
    return success;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Error: Invalid number of arguments.\n"
                "Usage: search <inverted-index file name>\n");
        return EXIT_FAILURE;
    }
    /* first, we parse the indexer file and load it into memory */
    indexer_t *indexer = parse_indexer_file(argv[1]);
    if(indexer == NULL) {
        /* we couldn't parse/load the indexer */
        return EXIT_FAILURE;
    }
    /* we poll the user for commands, waiting for the 'quit' command to exit */
    char input[300];
    *input = '\0';
    fgets(input, 300, stdin);
    input[strlen(input) - 1] = '\0';
    while (strcmp(input, "q") != 0) {
        if (!handle_input(indexer, input)) {
            /* user entered an invalid command */
            fprintf(stderr, "Error: Invalid command.\n");
        }
        fflush(stdout);
        *input = '\0';
        fgets(input, 300, stdin);
        input[strlen(input) - 1] = '\0';
    }
    destroy_indexer(indexer);
    return EXIT_SUCCESS;
}