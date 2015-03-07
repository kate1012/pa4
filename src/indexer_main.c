#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "indexer.h"

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Error: Invalid number of arguments.\n"
                "Usage: indexer <inverted-index file name> "
                "<directory or file name>\n");
        return EXIT_FAILURE;
    } else if (strcmp(argv[1], argv[2]) == 0) {
        fprintf(stderr, "Error: Target index file and file to be "
                "indexed are the same.\n");
        return EXIT_FAILURE;
    }
    char *new_file_path = argv[1];
    char *input_path = argv[2];

    FILE *new_file;
    /* first we check if the new indexer file already exists */
    if (access(new_file_path, F_OK) != -1) {
        /* if it does, we give the user a few options */
        printf("File already exists. Type '1' to overwrite, "
                "'2' to append, or '3' to cancel.\n");
        fflush(stdout);
        int option;
        scanf("%d", &option);
        /* if the user wants to quit, we do so */
        if (option == 3) return EXIT_SUCCESS;
        /* we choose our write mode depending on what the user wants */
        new_file = fopen(new_file_path, option == 1 ? "w" : "a");
    } else {
        /* if not, we create a new file */
        new_file = fopen(new_file_path, "a+");
    }

    /* time to create and run our indexer */
    indexer_t *indexer = create_indexer();
    bool success = run_indexer(indexer, input_path);
    if (success) {
        /* first we create an iterator for the indexer entries */
        list_iterator_t *entries = create_iterator(indexer->entries);
        /* next, we iterate through every entry */
        indexer_entry_t *entry = get_item(entries);
        while (entry != NULL) {
            fprintf(new_file, "<list> %s\n", entry->token);
            /* when we get an entry, we iterate through each record */
            list_iterator_t *records = create_iterator(entry->records);
            indexer_entry_record_t *record = get_item(records);
            int record_counter = 0;
            while (record != NULL) {
                /* time to print the record data - we only want 5 records per line */
                if (record_counter == 5) {
                    record_counter = 0;
                    fprintf(new_file, "\n");
                }
                fprintf(new_file, "%s %i", record->file_path, record->count);
                record = next_item(records);
                /* if we have another record, we print a space to prefix it */
                if (record != NULL) fprintf(new_file, " ");
                record_counter++;
            }
            destroy_iterator(records);
            entry = next_item(entries);
            fprintf(new_file, "\n</list>\n");
        }
        destroy_iterator(entries);
    } else {
        fprintf(stderr, "Error parsing input file(s). Does the given file or directory exist?\n");
    }
    fclose(new_file);
    destroy_indexer(indexer);
    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}