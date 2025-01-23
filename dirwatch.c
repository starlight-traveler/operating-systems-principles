/**
 * @file dirwatch.c
 * @brief Adding metadata to the loop
 * @author Ryan Paillet
 * @date January 20th, 2025
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>

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

static const char *file_type(mode_t mode)
{
    if (S_ISREG(mode))
        return "file";
    if (S_ISDIR(mode))
        return "dir";
    if (S_ISLNK(mode))
        return "link";
    if (S_ISBLK(mode))
        return "blk";
    if (S_ISCHR(mode))
        return "chr";
    if (S_ISFIFO(mode))
        return "fifo";
    if (S_ISSOCK(mode))
        return "sock";
    return "other";
}

static void get_owner_name(uid_t uid, char *owner, size_t n)
{
    struct passwd *pwd = getpwuid(uid);
    if (pwd)
    {
        snprintf(owner, n, "%s", pwd->pw_name);
    }
    else
    {
        snprintf(owner, n, "%u", (unsigned)uid);
    }
}

static void list_once(const char *path)
{
    DIR *d = opendir(path);
    if (!d)
    {
        fprintf(stderr, "Unable to open '%s': %s\n", path, strerror(errno));
        return;
    }

    printf("%-20s %10s %-6s %4s %-16s\n", "NAME", "SIZE", "TYPE", "MODE", "OWNER");
    printf("---------------------------------------------------------------------\n");

    struct dirent *ent;
    while ((ent = readdir(d)) != NULL)
    {
        char full[1024];
        snprintf(full, sizeof(full), "%s/%s", path, ent->d_name);

        struct stat st;
        if (lstat(full, &st) != 0)
        {
            fprintf(stderr, "lstat('%s'): %s\n", full, strerror(errno));
            continue;
        }

        char owner[64];
        get_owner_name(st.st_uid, owner, sizeof(owner));
        printf("%-20s %10lld %-6s %04o %-16s\n",
               ent->d_name,
               (long long)st.st_size,
               file_type(st.st_mode),
               (unsigned)(st.st_mode & 07777),
               owner);
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
