#include "time.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"

#include "fcntl.h"
#include "sys/wait.h"

#include "../suplib/aio.h"
#include "../suplib/aproc.h"
#include "../suplib/astring.h"

uint8_t is_alive = 1;

void sigint_handler(int sig)
{
    printf("SIGINT handled!\n");
    is_alive = 0;
}

int write_current_time_to_descriptor(int d)
{
    char text[100];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(text, sizeof(text) - 1, "%d.%m.%Y %H:%M:%S\n", t);
    if (write(d, text, strlen(text)) != strlen(text)) {
        syslog(LOG_ERR, "Error occured in writing to descripto!\n");
        return 1;
    }
    return 0;
}

int child_process_function(proc_dp* proc)
{
    int result = 0;
    while (is_alive) {
        if (!result && write_current_time_to_descriptor(proc->pipes.writed)) {
            close(proc->pipes.writed);
            result = 1;
        }
        sleep(3);
    }
    return result;
}

int read_from_pipe(proc_dp** processes, int count)
{
    printf("Parent process started!\n");
    astring str = {NULL, 0, 0};

    sleep(2);
    READ_STATE state = readline(processes[0]->pipes.readd, &str);
    printf("Parent read: %s\n", str.data);

    str.len = 0;
    state = readline(processes[1]->pipes.readd, &str);
    printf("Parent read: %s\n", str.data);

    str.len = 0;
    state = readline(processes[2]->pipes.readd, &str);
    printf("Parent read: %s\n", str.data);

    printf("Parent process exit!\n");
    return 0;
}

int main(int argc, char *argv[])
{
    int processes_count = 4;
    int8_t is_child = 0;
    proc_dp** processes = create_processes_dp(&processes_count, &is_child);
    if (is_child == -1) {
        printf("Fork wailed!\n");
        return 3;
    } else if (is_child == 1) {
        child_process_function(*processes);
    } else {
        read_from_pipe(processes, processes_count);
    }

    return 0;
}
