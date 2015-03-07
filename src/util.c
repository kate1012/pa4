#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include "sorted_list.h"
#include "util.h"

static int tokenize_sort_function(void *first_object, void *second_object) {
    char *first = first_object;
    char *second = second_object;
    return strcmp(first, second) == 0 ? 0 : -1;
}

static int tokenize_list_function(void *first_object, void *second_object) {
    return -1;
}

static void tokenize_destroy_function(void *object) {
    free(object);
}

/*
 * Tokenizes a string, given the string, the delimiter, and whether
 * the tokens should be sorted. This method returns a list_t containing
 * the resulting tokens.
 */
list_t *tokenize(char *string, char delimiter, bool sort) {
    list_t *list = sort ? create_list(&tokenize_sort_function, &tokenize_destroy_function) :
            create_list(&tokenize_list_function, &tokenize_destroy_function);
    char *last_position = string;
    char *position = NULL;
    while ((position = strchr(last_position, delimiter)) != NULL) {
        size_t line_size = position - last_position;
        char *line = malloc(line_size + sizeof(char));
        memcpy(line, last_position, line_size);
        sprintf(line + line_size, "%c", '\0');
        insert_object(list, line);
        last_position = position + sizeof(char);
    }
    size_t final_line_size = strlen(last_position);
    if (final_line_size > 0) {
        char *line = malloc(final_line_size + sizeof(char));
        memcpy(line, last_position, final_line_size);
        sprintf(line + final_line_size, "%c", '\0');
        insert_object(list, line);
    }
    return list;
}

/*
* Reads the contents of a file, given the path, and returns it as a string. If this
* function returns NULL, there was an error. Otherwise, the caller is responsible
* for freeing the allocated memory.
*/
char *read_file(char *file_path) {
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
                    sprintf(current_index, "%c", current);
                    current_index += sizeof(char);
                }
                sprintf(data + file_size, "%c", '\0');
                /* once we've read the file's contents, we can close it */
                fclose(file);
                return data;
            } else {
                /* file is empty. we should close it and return an empty string */
                fclose(file);
                char *data = malloc(sizeof(char));
                *data = '\0';
                return data;
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