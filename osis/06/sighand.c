#include "stdio.h"
#include "signal.h"
#include "fcntl.h"
#include "syslog.h"

#include "../suplib/astring.h"
#include "../suplib/aio.h"

const char* PATH_TO_CONF = "/home/anton/anton_work/sem6/osis/06/sig.conf";
const mode_t open_mode = O_RDONLY;
const mode_t per_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

typedef struct sigc_t {
    const char* name;
    int number;
    bool is_handled;
} sigc;

sigc signals[32] = {{.name = "HUP", .number = 1, false},
                    {.name = "INT", .number = 2, false},
                    {.name = "QUIT", .number = 3, false},
                    {.name = "ILL", .number = 4, false},
                    {.name = "TRAP", .number = 5, false},
                    {.name = "ABRT", .number = 6, false},
                    {.name = "BUS", .number = 7, false},
                    {.name = "FPE", .number = 8, false},
                    {.name = "USR1", .number = 10, false},
                    {.name = "SEGV", .number = 11, false},
                    {.name = "USR2", .number = 12, false},
                    {.name = "PIPE", .number = 13, false},
                    {.name = "ALRM", .number = 14, false},
                    {.name = "TERM", .number = 15, false},
                    {.name = "STKFLT", .number = 16, false},
                    {.name = "CHLD", .number = 17, false},
                    {.name = "CONT", .number = 18, false},
                    {.name = "TSTP", .number = 20, false},
                    {.name = "TTIN", .number = 21, false},
                    {.name = "TTOU", .number = 22, false},
                    {.name = "URG", .number = 23, false},
                    {.name = "XCPU", .number = 24, false},
                    {.name = "XFSZ", .number = 25, false},
                    {.name = "VTALRM", .number = 26, false},
                    {.name = "PROF", .number = 27, false},
                    {.name = "WINCH", .number = 28, false},
                    {.name = "POLL", .number = 29, false},
                    {.name = "PWR", .number = 30, false},
                    {.name = "SYS", .number = 31, false}};
const uint32_t SIGNALS_COUNT = 31;

int daemon()
{
    // Before this calling this function, please close all file descriptors
    pid_t pid = fork();
    if (pid == -1) {
        return -1;
    } else if (pid != 0) {
        return pid;
    }

    if (setsid() == -1 || chdir("/") == -1) {
        return -1;
    }

    open("/dev/null", O_RDONLY);
    open("/dev/null", O_RDWR);
    open("/dev/null", O_RDWR);
    return 0;
}

bool load_config(const char* path, bool is_interrupt);

void handler(int signum)
{
    syslog(LOG_INFO, "Signal %s registered!\n", sys_siglist[signum]);
    if (signum == SIGHUP) {
        if (load_config(PATH_TO_CONF, true)) {
            syslog(LOG_INFO, "New config loaded!\n");
        } else {
            syslog(LOG_INFO, "An error occured while loading config file!\n");
        }
    }
}

void register_signal_handler(int sig_id, bool is_interrupt)
{
    const char* prefix = is_interrupt ? "Interrupt: " : "";
    if (signal(sig_id, handler) != SIG_ERR) {
        syslog(LOG_INFO, "%sHandler of %s successfully registered!", prefix, sys_siglist[sig_id]);
        return;
    }
    syslog(LOG_WARNING, "Can't register handler for %s!", sys_siglist[sig_id]);
}

void reset_all_handlers()
{
    for (uint32_t i = 0; i < SIGNALS_COUNT; i++) {
        signal(signals[i].number, SIG_DFL);
    }
}

void register_signals_handlers(astring_arr* arr, bool is_interrupt)
{

    reset_all_handlers();
    for (uint32_t i = 0; i < arr->size; i++) {
        if (arr->data[i].len != 0) {
            for (uint32_t j = 0; j < SIGNALS_COUNT; j++) {
                if (strcmp(arr->data[i].data, signals[j].name) == 0) {
                    register_signal_handler(signals[j].number, is_interrupt);
                    break;
                }
            }
        }
    }
}

bool load_config(const char* path, bool is_interrupt)
{
    int fd_conf = 0;
    astring_arr arr = {NULL, 0};
    if ((fd_conf = open(path, open_mode)) == -1) {
        return false;
    }

    READ_STATE state = readwords(fd_conf, &arr);
    if (state == IO_ERROR || state == NO_MEMORY) {
        return false;
    }
    close(fd_conf);

    register_signals_handlers(&arr, is_interrupt);
    free_astring_arr(&arr);
    return true;
}

int main(int argc, char *argv[])
{
    int daemon_result = daemon();
    if (daemon_result == -1) {
        return 1;
    } else if (daemon_result != 0) {
        return 0;
    }

    if (signal(SIGHUP, handler) == SIG_ERR) {
        printf("Can't register signal handler!\n");
        return 1;
    } else if (!load_config(PATH_TO_CONF, false)) {
        printf("An error occured while loading config file!\n");
    }

    while (true) { }
    return 0;
}
