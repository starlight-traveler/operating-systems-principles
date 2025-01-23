/**
 * @file dirwatch.c
 * @brief loops directory
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

static volatile int keep_running = 1;

static void interrupt_handler(int sig)
{
    (void)sig;
    keep_running = 0;
}

static void clear_screen(void)
{
    printf("\033[H\033[J");
    fflush(stdout);
}

static void list_once(const char *path)
{
    DIR *d = opendir(path);
    if (!d)
    {
        fprintf(stderr, "Unable to open '%s': %s\n", path, strerror(errno));
        return;
    }
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL)
    {
        puts(ent->d_name);
    }
    closedir(d);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <directory>\n", argv[0]);
        return 1;
    }

    signal(SIGINT, interrupt_handler);

    while (keep_running)
    {
        clear_screen();
        list_once(argv[1]);
        fflush(stdout);

        for (int i = 0; i < 3; i++)
        {
            if (!keep_running)
                break;
            sleep(1);
        }
    }

    return 0;
}
