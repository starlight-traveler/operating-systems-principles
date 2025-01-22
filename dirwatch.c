/**
 * @file dirwatch.c
 * @brief list directory contents
 * @author Ryan Paillet
 * @date January 20th, 2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <directory>\n", argv[0]);
        return 1;
    }

    DIR *d = opendir(argv[1]);
    if (!d)
    {
        fprintf(stderr, "Unable to open '%s': %s\n", argv[1], strerror(errno));
        return 1;
    }

    struct dirent *ent;
    while ((ent = readdir(d)) != NULL)
    {
        printf("%s\n", ent->d_name);
    }
    closedir(d);
    return 0;
}
