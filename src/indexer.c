#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include "indexer.h"
#include "tokenizer.h"

/*
* Creates an indexer entry record, given the file path. The caller is
* responsible for freeing the allocated memory.
*/
indexer_entry_record_t *create_indexer_entry_record(char *file_path) {
    indexer_entry_record_t *record = malloc(sizeof(indexer_entry_record_t));
    record->file_path = malloc(strlen(file_path) + sizeof(char));
    strcpy(record->file_path, file_path);
    record->count = 0;
    return record;
}

/*
* Destroys an indexer entry record.
*/
void destroy_indexer_entry_record(indexer_entry_record_t *record) {
    free(record->file_path);
    free(record);
}

/*
* Gets an index entry record, given the file path. Returns NULL if it does not exist.
*/
indexer_entry_record_t *get_indexer_entry_record(indexer_entry_t *indexer_entry, char *file_path) {
    list_iterator_t *iterator = create_iterator(indexer_entry->records);
    indexer_entry_record_t *record = get_item(iterator);
    while (record != NULL) {
        if (strcmp(file_path, record->file_path) == 0) {
            destroy_iterator(iterator);
            return record;
        }
        record = next_item(iterator);
    }
    destroy_iterator(iterator);
    return NULL;
}

/*
* Comparison function for indexer entry records that are inside a list.
*/
static int record_compare_function(void *first, void *second) {
    indexer_entry_record_t *first_record = first;
    indexer_entry_record_t *second_record = second;
    return first_record->count > second_record->count ? 1 : -1;
}

/*
* Destroy function for indexer entry records that are inside a list.
*/
static void record_destroy_function(void *object) {
    destroy_indexer_entry_record(object);
}

/*
* Creates an indexer entry. The caller is responsible for freeing
* the allocated memory.
*/
indexer_entry_t *create_indexer_entry(char *token) {
    indexer_entry_t *entry = malloc(sizeof(indexer_entry_t));
    entry->records = create_list(&record_compare_function,
            &record_destroy_function);
    entry->token = malloc(strlen(token) + sizeof(char));
    strcpy(entry->token, token);
    return entry;
}

/*
* Destroys an indexer entry.
*/
void destroy_indexer_entry(indexer_entry_t *entry) {
    destroy_list(entry->records);
    free(entry->token);
    free(entry);
}

/*
* Gets an index entry, given the token. Returns NULL if it does not exist.
*/
indexer_entry_t *get_indexer_entry(indexer_t *indexer, char *token) {
    list_iterator_t *iterator = create_iterator(indexer->entries);
    indexer_entry_t *entry = get_item(iterator);
    while (entry != NULL) {
        if (strcmp(token, entry->token) == 0) {
            destroy_iterator(iterator);
            return entry;
        }
        entry = next_item(iterator);
    }
    destroy_iterator(iterator);
    return NULL;
}

/*
* Comparison function for indexer entries that are inside a list.
*/
static int entry_compare_function(void *first, void *second) {
    indexer_entry_t *first_entry = first;
    indexer_entry_t *second_entry = second;
    return strcmp(first_entry->token, second_entry->token) > 0 ? -1 : 1;
}

/*
* Destroy function for indexer entries that are inside a list.
*/
static void entry_destroy_function(void *object) {
    destroy_indexer_entry((indexer_entry_t *) object);
}

/*
* Creates an indexer. The caller is responsible for freeing the
* allocated memory.
*/
indexer_t *create_indexer() {
    indexer_t *indexer = malloc(sizeof(indexer_t));
    indexer->entries = create_list(&entry_compare_function,
            &entry_destroy_function);
    return indexer;
}

/*
* Destroys an indexer.
*/
void destroy_indexer(indexer_t *indexer) {
    destroy_list(indexer->entries);
    free(indexer);
}


/*
 * Converts an uppercase letter character to its lowercase equivalent.
 */
static int to_lower_case(int c) {
    if (c >= 'A' && c <= 'Z') {
        c -= 'A';
        c += 'a';
    }
    return c;
}

/*
* Reads the contents of a file, given the path, and returns it as a string. If this
* function returns NULL, there was an error. Otherwise, the caller is responsible
* for freeing the allocated memory.
*/
static char *read_file(char *file_path) {
    FILE *file = fopen(file_path, "r");
    if (file != NULL) {
        /* we get a stat for the file so we have the size in bytes before reading */
        struct stat file_stat;
        if (fstat(fileno(file), &file_stat) == 0) {
            /* time to read the contents of the file */
            off_t file_size = file_stat.st_size;
            if (file_size > 0) {
                char *data = malloc((size_t) file_size + sizeof(char));
                char *current_index = data;
                int current;
                while ((current = fgetc(file)) != EOF) {
                    sprintf(current_index, "%c", to_lower_case(current));
                    current_index += sizeof(char);
                }
                sprintf(data + file_size, "%c", '\0');
                /* once we've read the file's contents, we can close it */
                fclose(file);
                return data;
            } else {
                /* file is empty. we should close it and return an empty string */
                fclose(file);
                char *str = malloc(sizeof(char));
                *str = '\0';
                return str;
            }
        } else {
            fprintf(stderr, "Error: Could not get stat for file.\n");
            fclose(file);
        }
    } else {
        fprintf(stderr, "Error: Problem opening file.\n");
    }
    return NULL;
}

static void handle_token(indexer_t *indexer, char *file_path, char *token) {
    /* we check for an instance of this entry already in the indexer */
    indexer_entry_t *entry = get_indexer_entry(indexer, token);
    if (entry == NULL) {
        /* if not, we create a new entry */
        entry = create_indexer_entry(token);
        insert_object(indexer->entries, entry);
    }
    /* we check for an instance of this record already in the indexer */
    indexer_entry_record_t *record = get_indexer_entry_record(entry, file_path);
    if (record == NULL) {
        /* if not, we create a new record */
        record = create_indexer_entry_record(file_path);
        insert_object(entry->records, record);
    }
    record->count++; //TODO: We need a way of resorting these after..
}

/*
* Parses a file given the indexer and the file path. This function will
* add and update indexer entries for the given indexer.
*/
static bool parse_file(indexer_t *indexer, char *file_path) {
    char *file_data = read_file(file_path);
    if (file_data == NULL) {
        return false;
    }
    list_t *token_list = tokenize(file_data);
    list_iterator_t *iterator = create_iterator(token_list);
    if (iterator != NULL) {
        char *token = get_item(iterator);
        while (token != NULL) {
            handle_token(indexer, file_path, token);
            token = next_item(iterator);
        }
        destroy_iterator(iterator);
    }
    destroy_list(token_list);
    free(file_data);
    return true;
}

/*
* Iterates through files in a directory, given the path, and recursively adds and updates
* entries for the given indexer.
*/
static bool traverse_directory(indexer_t *indexer, char *path) {
    DIR *directory = opendir(path);
    /* we recursively traverse the given directory and parse any files */
    if (directory != NULL) {
        struct dirent *current_file;
        while ((current_file = readdir(directory)) != NULL) {
            char *file_name = current_file->d_name;
            if (strcmp(file_name, ".") == 0 || strcmp(file_name, "..") == 0) {
                /* we skip local and parent directory file links */
                continue;
            }
            size_t file_path_size = strlen(path) + strlen(file_name) + (2 * sizeof(char));
            char *file_path = malloc(file_path_size);
            strcpy(file_path, path);
            strcat(file_path, "/");
            strcat(file_path, file_name);
            if (current_file->d_type == DT_DIR) {
                traverse_directory(indexer, file_path);
            } else if (current_file->d_type == DT_REG) {
                parse_file(indexer, file_path);
            }
            free(file_path);
        }
        closedir(directory);
        return true;
    }
    else {
        /* error opening directory */
        return false;
    }
}

/*
* Runs the indexer, given the path to the directory to recursively
* traverse through, or a file to parse.
*/
bool run_indexer(indexer_t *indexer, char *path) {
    if (!traverse_directory(indexer, path) && !parse_file(indexer, path)) {
        /* could not traverse given directory or could not parse given file */
        return false;
    }
    return true;
}