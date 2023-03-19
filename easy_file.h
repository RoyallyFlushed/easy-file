#ifndef _EASY_FILE_H
#define _EASY_FILE_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define return_defer(value) do {    \
    result = (value);               \
    goto return_defer;              \
} while (0)                         \

#define array_size(array) sizeof(array) / sizeof(array[0])

typedef int Errno;

typedef struct {
    char *content;
    size_t cursor;
    size_t count;
    size_t capacity;
    size_t line;
} easy_file;

Errno easy_file_open_file(const char *file_name, easy_file *ef);
Errno easy_file_split_by_lines(easy_file *ef, char ***lines, size_t *lines_size);
void easy_file_rewind_file(easy_file *ef);
void easy_file_compute_line(easy_file *ef);
char easy_file_read_char(easy_file *ef);

// Start of library definitions
#ifdef EASY_FILE_IMPLEMENTATION

// Get the size of the file helper function.
static Errno get_file_size(FILE *file, size_t *size)
{
    // Get current cursor position
    long saved_cursor = ftell(file);
    if (saved_cursor < 0) return errno;

    // Try setting cursor to end of file and exit if non-zero
    if (fseek(file, 0, SEEK_END) < 0) return errno;

    // Get the position of the last character
    long result = ftell(file);
    if (result < 0) return errno;

    // Try setting cursor back to original position and exit if non-zero
    if (fseek(file, saved_cursor, SEEK_SET) < 0) return errno;

    // Set the size
    *size = (size_t) result;

    return 0;
}

// Create a new easy_file object with the given file contents
Errno easy_file_open_file(const char *file_name, easy_file *ef)
{
    Errno result = 0;

    FILE *file = fopen(file_name, "r");
    size_t file_size;
    
    // Get file size
    Errno error = get_file_size(file, &file_size);
    if (error != 0) return_defer(error);

    // If the allocated capacity is smaller than the file size, extend alloc'ed memory
    if (ef->capacity < file_size) {
        ef->capacity = file_size;
        ef->content = realloc(ef->content, ef->capacity * sizeof(*ef->content));
    }

    // Read the file contents into the easy_file content buffer
    fread(ef->content, file_size, 1, file);
    if (ferror(file)) return_defer(errno);

    // Set count
    ef->count = file_size;

return_defer:
    // Clean up
    if (file) fclose(file);
    return result;
}

// Split the contents of the easy file buffer by line into the given lines array
Errno easy_file_split_by_lines(easy_file *ef, char ***lines, size_t *lines_size)
{
    Errno result = 0;
    size_t lines_count = 0;

    // Get line size
    for (size_t i = 0; ef->content[i] != '\0'; i++) {
        if (ef->content[i] == '\n') lines_count++;
    }

    *lines_size = lines_count;

    // Create lines array
    *lines = calloc(lines_count, sizeof(char*));
    if (!(*lines)) {
        fprintf(stderr, "Err: Unable to allocate memory for lines!\n");
        return_defer(errno);
    }

    size_t line_begin = 0;

    // Populate lines array
    for (size_t i = 0; i <= lines_count; i++) {
        size_t line_end = line_begin;

        // Keep looking ahead until we reach a new line or the end
        while(ef->content[line_end] != '\n' && ef->content[line_end] != '\0') line_end++;

        // Get size
        size_t line_size = line_end - line_begin;

        // Allocate memory for the new string
        (*lines)[i] = malloc((line_size + 1) * sizeof(char));
        if (!(*lines)[i]) {
            fprintf(stderr, "Err: Unable to allocate memory for lines!\n");
            if (*lines) {
                for (size_t j = 0; j < i; j++) if ((*lines)[j]) free((*lines)[j]);
                free(*lines);
            }
            return_defer(errno);
        }

        // Put the substring into the allocated string
        strncpy((*lines)[i], (const char*) &ef->content[line_begin], line_size);
        (*lines)[i][line_size] = '\0';

        line_begin = line_end + 1;
    }

return_defer:
    return result;
}

void easy_file_rewind_file(easy_file *ef)
{
    ef->cursor = 0;
    ef->line = 0;
}

void easy_file_compute_line(easy_file *ef)
{
    size_t saved_cursor = ef->cursor;

    easy_file_rewind_file(ef);

    while (ef->cursor <= saved_cursor) {
        if (ef->content[ef->cursor] == '\n') ef->line++;
        ef->cursor++;
    }

    ef->cursor = saved_cursor;
}

char easy_file_read_char(easy_file *ef)
{
    easy_file_compute_line(ef);
    return ef->content[ef->cursor++];
}

#endif

#endif // _EASY_FILE_H
