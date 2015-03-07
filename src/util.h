#ifndef _UTIL_H_
#define _UTIL_H_

/*
 * Tokenizes a string, given the string, the delimiter, and whether
 * the tokens should be sorted. This method returns a list_t containing
 * the resulting tokens.
 */
list_t *tokenize(char *, char, bool);

/*
* Reads the contents of a file, given the path, and returns it as a string. If this
* function returns NULL, there was an error. Otherwise, the caller is responsible
* for freeing the allocated memory.
*/
char *read_file(char *file_path);

#endif