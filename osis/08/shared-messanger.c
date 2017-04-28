#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "stdint.h"
#include "string.h"
#include "unistd.h"

#include "poll.h"
#include "fcntl.h"
#include "getopt.h"
#include "signal.h"
#include "sys/ipc.h"
#include "sys/msg.h"
#include "sys/sem.h"
#include "sys/shm.h"
#include "sys/uio.h"
#include "termios.h"
#include "sys/stat.h"
#include "sys/wait.h"
#include "sys/types.h"

#include "../suplib/aio.h"
#include "../suplib/aipcv.h"
#include "../suplib/aproc.h"
#include "../suplib/astring.h"

struct global_args_t {
    bool is_exclude_start;
    bool is_client;
    char *sem_name;
    int project_id;
} global_args;
static struct option OPTS[] = {{"id", required_argument, NULL, 'i'},
                               {"name", required_argument, NULL, 'n'},
                               {"client", no_argument, NULL, 'c'},
                               {"exclude", no_argument, NULL, 'e'},
                               {0, 0, 0, 0}};

typedef struct squeue_t {
    semv* sem;
    int msqid;
    int shmid;
    void* data;
    ssize_t capacity;
} squeue;

typedef enum MSG_TYPES_T {
    PID_MSG_T = 0,
    INT_MSG_T = 1,
    STRING_MSG_T = 2,
} MSG_TYPES;

typedef enum INT_MSG_TYPES_T {
    READ_DATA_FROM_QUEUE = 0,
} INT_MSG_TYPES_T;

typedef struct int_msg_t {
    long type;
    int data;
} int_msg;
size_t int_msg_data_size = sizeof(int_msg) - sizeof(long);

typedef struct pid_msg_t {
    long type;
    pid_t data;
} pid_msg;
size_t pid_msg_data_size = sizeof(int_msg) - sizeof(long);

typedef struct string_msg_t {
    long type;
    bool is_last_el;
    size_t size; // This size not of data, but of memory after struct
    char data[1]; // Attention! C struct hack!
} string_msg;

const uint32_t SEM_COUNT = 2;
volatile sig_atomic_t is_alive = 1;
void sigint_handler(int sig) { is_alive = 0; }

int parse_arguments(int argc, char *argv[])
{
    int c;
    int opt_idx;
    int proj_id;
    global_args.is_client = false;
    global_args.is_exclude_start = false;
    const int default_project_id = 1488;

    while ((c = getopt_long(argc, argv, "cen:i:", OPTS, &opt_idx)) != -1) {
        switch (c) {
        case 'n':
            global_args.sem_name = optarg;
            break;
        case 'e':
            global_args.is_exclude_start = true;
            break;
        case 'c':
            global_args.is_client = true;
            break;
        case 'i':
            proj_id = atoi(optarg);
            global_args.project_id = !proj_id ? default_project_id : proj_id;
            break;
        default:
            return -1;
        }
    }
    return 0;
}

static inline int is_sem_created(squeue *msg, key_t key, bool is_exclude)
{
    if (is_exclude) {
        return (msg->sem = excreate_semv_k(key, SEM_COUNT)) == NULL ? 1 : 0;
    }
    return (msg->sem = create_semv_k(key, SEM_COUNT)) == NULL ? 1 : 0;
}

squeue* connect_or_create_squeue(char *key_line, int project_id,
                                 size_t size, int oflag)
{
    squeue *msg;
    key_t key = ftok(key_line, project_id);
    if (key == -1) {
        goto key_error;
    } else if ((msg = (squeue*)malloc(sizeof(squeue))) == NULL) {
        goto malloc_error;
    } else if ((msg->msqid = msgget(key, oflag | 0666) == -1)) {
        goto msgget_error;
    }

    if (oflag == 0) {
        if ((msg->sem = get_semv_k(key)) == NULL) {
            goto sem_error;
        }
    } else {
        if (is_sem_created(msg, key, oflag & IPC_EXCL)) {
            goto sem_error;
        }
    }

    if ((msg->shmid = shmget(key, size, oflag)) == -1) {
        goto shmget_error;
    } else if ((msg->data = shmat(msg->shmid, NULL, 0)) == (char *)-1) {
        goto shmmat_error;
    }
    msg->capacity = size;
    return msg;

 shmmat_error:
 shmget_error:
 sem_error:
 msgget_error:
    free(msg);
 malloc_error:
 key_error:
    return NULL;
}

squeue *create_squeue(char *key_line, int project_id,
                      bool is_exclude, size_t size)
{
    int oflag = is_exclude ? IPC_EXCL | IPC_CREAT : IPC_CREAT;
    return connect_or_create_squeue(key_line, project_id, size, oflag);
}

squeue* connect_squeue(char *key_line, int project_id, size_t size)
{
    return connect_or_create_squeue(key_line, project_id, size, 0);
}

string_msg* find_last_squeue_el(squeue *q, size_t *offset)
{
    string_msg* lastmsg = (string_msg*)q->data;
    while (!lastmsg->is_last_el) {
        *offset += sizeof(string_msg) + lastmsg->size;
        lastmsg = (string_msg*)q->data + *offset;
    }
    return lastmsg;
}

int wait_for_server_reading(squeue *q)
{
    int_msg msg = {INT_MSG_T, READ_DATA_FROM_QUEUE};
    if (msgsnd(q->msqid, &msg, int_msg_data_size, 0)
        && semwait_empty_semv(q->sem->semid, 1) == 0
        && semwait_semv(q->sem->semid, 1) == 0
        && sempost_semv(q->sem->semid, 1) == 0) {
        return 1;
    }
    return 0;
}

void append_msg(string_msg *lastmsg, void *data, size_t size)
{
    if (lastmsg->size != 0) {
        lastmsg->is_last_el = false;
        lastmsg += sizeof(string_msg) + lastmsg->size;
    }

    lastmsg->type = STRING_MSG_T;
    lastmsg->is_last_el = true;
    lastmsg->size = size;
    memcpy(lastmsg->data, data, size);
}

int put_msg_to_shared_queue(squeue *q, void *data, size_t size)
{
    size_t offset = 0;
    size_t new_el_size = sizeof(string_msg) + size;
    string_msg* lastmsg = find_last_squeue_el(q, &offset);

    if (offset + new_el_size >= q->capacity) {
        if (wait_for_server_reading(q)) {
            goto server_wait_error;
        }
        lastmsg = find_last_squeue_el(q, &offset);
    }
    append_msg(lastmsg, data, size);
    return 0;

 server_wait_error:
    return 1;
}

string_msg* get_next_smsg(string_msg* smsg)
{
    if (smsg->is_last_el) {
        return NULL;
    }
    return smsg + sizeof(string_msg) + smsg->size;
}

char *input_string(FILE* fp, size_t size) {
    char *str;
    int ch;
    size_t len = 0;
    str = (char*)realloc(NULL, sizeof(char)*size);
    if(!str) {
        return str;
    }
    while((ch = fgetc(fp)) != EOF && ch != '\n'){
        str[len++] = ch;
        if(len == size){
            str = (char*)realloc(str, sizeof(char)*(size += 16));
            if(!str) {
                return str;
            }
        }
    }
    str[len++] = '\0';
    return (char*)realloc(str, sizeof(char) * len);
}

int client_func(squeue *q)
{
    printf("Client start work!\n");
    while (is_alive) {
        char *msg = input_string(stdin, 80);
        if (msg != NULL) {
            if (strcmp(msg, "exit") == 0) {
                return 0;
            }
            semwait_semv(q->sem->semid, 0);
            put_msg_to_shared_queue(q, msg, strlen(msg) + 1);
            sempost_semv(q->sem->semid, 0);
        }
    }
    printf("Client stop working!\n");
    return 0;
}

int server_func(squeue *q)
{
    printf("Server start work!\n");
    while (is_alive) {
        semwait_semv(q->sem->semid, 0);
        string_msg* smsg = (string_msg*)q->data;
        do
            {
                if (smsg->size) {
                    printf("%s\n", smsg->data);
                }
            } while ((smsg = get_next_smsg(smsg)) != NULL);
        smsg = (string_msg*)q->data;
        smsg->is_last_el = true;
        smsg->size = 0;
        do
            {
                errno = 0;
                sempost_semv(q->sem->semid, 0);
                if (errno != 0) {
                    printf("%d %d\n", errno, EINTR);
                }
            } while (errno == EINTR);
    }
    printf("Server stop working!\n");
    return 0;
}

int set_signals_handlers()
{
    struct sigaction sa;
    sa.sa_flags = 0;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    squeue *q;
    size_t qsize = 500;

    if (set_signals_handlers()) {
        goto  sighand_error;
    } else if (parse_arguments(argc, argv)) {
        goto arg_error;
    } else if (global_args.is_client) {
        if ((q = connect_squeue(global_args.sem_name,
                                global_args.project_id, qsize)) == NULL) {
            goto shared_mem_error;
        }
        client_func(q);
    } else {
        if ((q = create_squeue(global_args.sem_name,
                               global_args.project_id,
                               global_args.is_exclude_start,
                               qsize)) == NULL) {
            goto shared_mem_error;
        }
        server_func(q);
        shmctl(q->msqid, IPC_RMID, NULL);
        shmctl(q->sem->semid, IPC_RMID, NULL);
    }

    shmdt(q->data);
    printf("Normal exit!\n");
    return 0;

 shared_mem_error:
    printf("Shared mem error!\n");;
    return 4;
 arg_error:
    printf("Arg error!\n");
    return 3;
 sighand_error:
    printf("Set sig handler error!\n");
    return 1;
}
