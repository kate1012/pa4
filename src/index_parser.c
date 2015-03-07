#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "index_parser.h"
#include "util.h"

/*
 * Parses and loads an indexer into memory, given the file path
 * of the indexer file.
 */
indexer_t *parse_indexer_file(char *file_path) {
    indexer_t *indexer = create_indexer();

    char *input = read_file(file_path);
    if(input == NULL) {
        return NULL;
    }
    list_t *lines = tokenize(input, '\n', false);
    free(input);

    list_iterator_t *iterator = create_iterator(lines);
    char *line = get_item(iterator);
    indexer_entry_t *current_entry = NULL;
    while (line != NULL) {
        char *index = NULL;
        if ((index = strstr(line, "<list> ")) != NULL) {
            char *token = index + (7 * sizeof(char));
            if ((current_entry = get_indexer_entry(indexer, token)) == NULL) {
                current_entry = create_indexer_entry(token);
                insert_object(indexer->entries, current_entry);
            }
        } else if (strstr(line, "</list>")) {
            current_entry = NULL;
        } else {
            list_t *tokens = tokenize(line, ' ', false);
            list_iterator_t *token_iterator = create_iterator(tokens);
            char *file = get_item(token_iterator);
            char *count = next_item(token_iterator);
            while (file != NULL && count != NULL) {
                indexer_entry_record_t *record;
                if((record = get_indexer_entry_record(current_entry, file)) == NULL) {
                    record = create_indexer_entry_record(file);
                    insert_object(current_entry->records, record);
                }
                record->count = atoi(count);
                file = next_item(token_iterator);
                count = next_item(token_iterator);
            }
            destroy_iterator(token_iterator);
            destroy_list(tokens);
        }
        line = next_item(iterator);
    }

    destroy_iterator(iterator);
    destroy_list(lines);
    return indexer;
}