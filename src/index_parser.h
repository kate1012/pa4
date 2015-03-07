#ifndef _INDEX_PARSER_H_
#define _INDEX_PARSER_H_

#include "sorted_list.h"
#include "indexer.h"

/*
 * Parses and loads an indexer into memory, given the file path
 * of the indexer file.
 */
indexer_t *parse_indexer_file(char *);

#endif