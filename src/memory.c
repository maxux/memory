#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

#define COLOR_RED      "\033[1;31m"
#define COLOR_YELLOW   "\033[1;33m"
#define COLOR_BLUE     "\033[1;34m"
#define COLOR_GREEN    "\033[1;32m"
#define COLOR_GREEN_N  "\033[0;32m"
#define COLOR_RESET    "\033[0m"

typedef struct ps_t {
    int pid;
    char *name;
    size_t memory;
    char *cmdline;

} ps_t;

static void diep(char *s) {
    perror(s);
    exit(EXIT_FAILURE);
}

static int isnum(char *val) {
    while(*val && *val >= '0' && *val <= '9')
        val++;

    return (*val == '\0');
}

static char *colorsize(size_t memory) {
    if(memory > 100 * 1024) // 100 MB
        return COLOR_RED;

    else if(memory > 60 * 1024) // 60 MB
        return COLOR_YELLOW;

    else if(memory > 20 * 1024) // 20 MB
        return COLOR_BLUE;

    else if(memory > 0)
        return COLOR_GREEN;

    return COLOR_GREEN_N;
}

static int pscmp(const void *a, const void *b) {
    return (((ps_t *) a)->memory - ((ps_t *) b)->memory);
}

static char *cmdtrunc(char *cmdline) {
    size_t len = strlen(cmdline);

    if(len < 33)
        return cmdline;

    strcpy(cmdline + 31, "...");

    if(len < 34)
        return cmdline;

    strcpy(cmdline + 34, cmdline + len - 6);

    return cmdline;
}

static ps_t *pidcmd(char *pid, ps_t *data) {
    FILE *fp;
    char path[128], buffer[512];

    sprintf(path, "/proc/%s/cmdline", pid);

    if(!(fp = fopen(path, "r")))
        diep(path);

    if(fgets(buffer, sizeof(buffer), fp) != NULL)
        data->cmdline = strdup(buffer);

    fclose(fp);

    return data;
}

static ps_t pidmem(char *pid) {
    FILE *fp;
    char path[128], buffer[512];
    ps_t data;

    sprintf(path, "/proc/%s/status", pid);

    if(!(fp = fopen(path, "r")))
        diep(path);

    data.cmdline = NULL;
    data.memory = 0;
    data.pid = 0;

    while(fgets(buffer, sizeof(buffer), fp) != NULL) {
        if(strncmp(buffer, "Name:", 5) == 0) {
            if(!(data.name = strdup(buffer + 6)))
                diep("strdup");

            data.name[strlen(data.name) - 1] = '\0';

        } else if(strncmp(buffer, "Pid:", 4) == 0)
            data.pid = atoi(buffer + 5);

        else if(strncmp(buffer, "VmRSS:", 6) == 0)
            data.memory = atoi(buffer + 7);
    }

    fclose(fp);

    // fetch cmdline if available
    pidcmd(pid, &data);

    return data;
}

int main() {
    DIR *dir;
    struct dirent *content;
    size_t elements = 0;
    ps_t *pslist = NULL;

    if(!(dir = opendir("/proc")))
        diep("/proc");

    // read amount of entries
    while((content = readdir(dir)) != NULL)
        elements += 1;

    rewinddir(dir);

    // allocates entries
    if(!(pslist = calloc(elements, sizeof(ps_t))))
        diep("calloc");

    // fetch entries
    size_t index = 0;

    while((content = readdir(dir)) != NULL) {
        if(!isnum(content->d_name))
            continue;

        pslist[index] = pidmem(content->d_name);
        index += 1;
    }

    closedir(dir);

    // sorting list
    qsort(pslist, index, sizeof(ps_t), pscmp);

    // listing processes
    printf(" PID    | Name                        | VmRSS\n");
    printf("--------+-----------------------------+-----------------\n");

    for(size_t i = 0; pslist[i].name; i++) {
        ps_t *data = &pslist[i];

        // skip kernel threads
        if(data->memory < 1)
            continue;

        char *color = colorsize(data->memory);
        float memsize = data->memory / 1024.0;

        printf("%s %-6d | %-40s | %.3f MB\n", color, data->pid, cmdtrunc(data->cmdline), memsize);
    }

    printf("%s", COLOR_RESET);
    printf("--------+-----------------------------+-----------------\n");
    printf(" PID    | Name                        | VmRSS\n");

    return 0;
}
