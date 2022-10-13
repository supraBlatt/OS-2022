#include "utils.h"
#include <wait.h>
#define WAIT 60
#define MAX_RESULT 255 // long result

// assumes validity of data
int format_data(struct config* conf) {
    // open the "to_srv" file and read data from it into config
    FILE* fp;
    fp = fopen(MAGIC_FILE, "r"); // for fscanf
    if (fp == NULL) { return -1; } 
    
    // read "[pid] [num1] [op] [num2]" from shared file
    fscanf(fp, "%d %ld %d %ld", &conf->pid, &conf->num1, &conf->op, &conf->num2);
    return 0;
}

int run_computation(const struct config* conf, char* result) { 
    operation op = conf->op - 1; // 1-4 -> 0-3
    if (op < ADD || op > DIV) { return -1; } 
   
    // naive approach. compare to actions. calculate.
    if (op == ADD) { sprintf(result, "%ld", conf->num1 + conf->num2); } 
    else if (op == SUB) { sprintf(result, "%ld", conf->num1 - conf->num2); }
    else if(op == MULT) { sprintf(result, "%ld", (conf->num1) * (conf->num2)); } 
    else if (conf->num2 == 0) { sprintf(result, "CANNOT_DIVIDE_BY_ZERO"); } // div by 0
    else { sprintf(result, "%ld", conf->num1 / conf->num2); } 

    return 0;
}

int write_to_client(pid_t pid, char* result) { 
    int len = snprintf(NULL, 0, "to_client_%d", pid);
    char path[len+1];
    sprintf(path, "to_client_%d", pid);

    // write to response file
    int fd = open(path, O_CREAT | O_WRONLY | O_EXCL, S_IRUSR | S_IWUSR);
    if (fd < 0) { return -1; }
    if (dprintf(fd, "%s\n", result) == 0) { return -1; }

    // delete "to_client_[pid]" file
    if (close(fd) < 0) { return -1; }
    return 0;
}

int handle_client() {

    // read request from shared file
    // delete to_srv file 
    struct config conf;
    if (format_data(&conf) < 0) { return -1; }
    if (remove(MAGIC_FILE) < 0) { return -1; }

    /* // do the computation */ 
    char result[MAX_RESULT];
    if (run_computation(&conf, result) < 0) { return -1; } 

    printf("Writing to client: %s\n", result);
    // write result to "to_client_[client-pid]"
    if (write_to_client(conf.pid, result) < 0) { return -1; } 

    // send signal to child process 
    kill(conf.pid, SIGUSR1);  
    return 0;
}

void kill_children() { 
    while(wait(NULL) != -1);
}

// rip. lonely server moment
void no_request(int signum) { 
    printf("The server was closed because no service request was received for the last 60 seconds\n");
    // wait for all zombies
    kill_children();
    exit(-1);
}

// got a request from a client. stop the timer.
void request(int signum) { 
    signal(SIGUSR1, request);
    
    // the alarm didn't kill us. 
    // make a child process to handle the client.
    pid_t pid; 
    if ((pid = fork()) == 0) {
        if (handle_client() < 0) { ERROR; exit(-1); }
        exit(0);
    }
}

int main(int argc, char* argv[]) {

    // delete to_srv if exists 
    if (access(MAGIC_FILE, F_OK) == 0) { 
        if (remove(MAGIC_FILE) < 0) { ERROR; return -1; } 
    }
    signal(SIGALRM, no_request);
    signal(SIGUSR1, request);

    while (1) { 
        // initialize timer 
        if(alarm(WAIT) < 0) { ERROR; kill_children(); return -1; }
        // waits for client signal SIGUSER1
        pause(); 
    }
}
