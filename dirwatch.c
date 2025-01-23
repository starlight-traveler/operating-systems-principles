/**
 * @file dirwatch.c
 * @brief Adding contents and symlink
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
#include <fcntl.h>
#include <ctype.h>

static volatile int keep_running = 1;

// Interrupt handler, could clean up if needed
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

// File types, from stat(2)
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

static int read_first_line(const char *file_path, char *buf, size_t max_len)
{
    int fd = open(file_path, O_RDONLY);
    if (fd < 0)
        return -1;

    size_t idx = 0;
    char c;
    ssize_t r;
    while (idx < max_len - 1 && (r = read(fd, &c, 1)) == 1)
    {
        if (c == '\n')
            break;
        buf[idx++] = isprint((unsigned char)c) ? c : '#';
    }
    buf[idx] = '\0';
    close(fd);
    return 0;
}

static int read_link_target(const char *path, char *buf, size_t n)
{
    ssize_t len = readlink(path, buf, n - 1);
    if (len < 0)
        return -1;
    buf[len] = '\0';
    return 0;
}

static void list_once(const char *path)
{
    DIR *d = opendir(path);
    if (!d)
    {
        fprintf(stderr, "Unable to open '%s': %s\n", path, strerror(errno));
        return;
    }

    // Could continue to cause problems if not widened correctly
    printf("%-18s %8s %-6s %4s %-14s %s\n", "NAME", "SIZE", "TYPE", "MODE", "OWNER", "CONTENTS");
    printf("----------------------------------------------------------------------------------------\n");

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

        char contents[512] = {0};
        if (S_ISDIR(st.st_mode))
        {
            snprintf(contents, sizeof(contents), "(directory)");
        }
        else if (S_ISLNK(st.st_mode))
        {
            if (read_link_target(full, contents, sizeof(contents)) == 0)
            {
                char tmp[512];
                snprintf(tmp, sizeof(tmp), "-> %s", contents);
                snprintf(contents, sizeof(contents), "%s", tmp);
            }
            else
            {
                snprintf(contents, sizeof(contents), "-> [readlink error: %s]", strerror(errno));
            }
        }
        else if (S_ISREG(st.st_mode))
        {
            if (read_first_line(full, contents, sizeof(contents)) != 0)
            {
                snprintf(contents, sizeof(contents), "[open/read error: %s]", strerror(errno));
            }
        }
        else
        {
            snprintf(contents, sizeof(contents), "(special)");
        }

        printf("%-18s %7lldB %-6s %04o %-14s %s\n",
               ent->d_name,
               (long long)st.st_size,
               file_type(st.st_mode),
               (unsigned)(st.st_mode & 07777),
               owner,
               contents);
    }

    closedir(d);
}

// Main, can segfault on certain types of files
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
