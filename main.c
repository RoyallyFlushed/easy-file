
#include <stdio.h>

#define EASY_FILE_IMPLEMENTATION
#include "easy_file.h"

int main(void)
{
    easy_file file = {0};
    easy_file_open_file("./input.txt", &file);

    char **lines;
    size_t lines_size;

    if (0 > easy_file_split_by_lines(&file, &lines, &lines_size)) exit(1);

    for (size_t i = 0; i <= lines_size; i++) {
        printf("%s\n", lines[i]);
    }

    // Clean up
    free(file.content);

    for (size_t i = 0; i <= lines_size; i++) free(lines[i]);
    free(lines);

    return 0;
}
