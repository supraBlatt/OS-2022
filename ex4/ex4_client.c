#include "utils.h"
#include <sys/random.h>

int is_number(char *str, long *val) {
    char *endptr;
    *val = strtol(str, &endptr, 10);
    if (*endptr != 0) { return -1; }
    return 0;
}

// expected args: ex4_client.out [server pid] [num1] [op] [num2]
int test_args(int argc, char *argv[], struct config *conf) {
    if (argc != 5) { return -1; }

    // ooooooo disgusting code.. testing args
    long val;
    // server pid
    if (is_number(argv[1], &val) < 0) { return -1; }
    conf->pid = val;
    // num1
    if (is_number(argv[2], &val) < 0) { return -1; }
    conf->num1 = val;
    // op. make sure it's in range too
    if (is_number(argv[3], &val) < 0 || val < ADD + 1 || val > DIV + 1) { return -1; }
    conf->op = val;
    // num2
    if (is_number(argv[4], &val) < 0) { return -1; }
    conf->num2 = val;
    return 0;
}

// returns fd of 'to_srv' if success. 0 otherwise
int get_srv_file(const struct config *conf) {

    int count = 0;
    while (count < 10) {
        int fd = open(MAGIC_FILE, O_CREAT | O_WRONLY | O_EXCL, S_IRUSR | S_IWUSR);
        if (fd >= 0) {

            // we got the file. write [pid num1 op num2] into it
            if (dprintf(fd, "%d %ld %d %ld", getpid(), conf->num1, conf->op, conf->num2) == 0) { return -1; }
            if (close(fd) < 0) { return -1; } // uh oh..
            return 0;
        }

        // erorr in opening file. check if it exists
        if (errno == EEXIST) {
            // increment count and wait a random amount of time
            count++;
            unsigned int tmp;
            if (getrandom(&tmp, sizeof(unsigned int), GRND_NONBLOCK) < 0) { return -1; }
            sleep(tmp % 6); // random amount between 0 and 5

        } else { return -1; }
    }
    // couldn't open it 10 times
    return -1;
}

// read the server reponse from shared file and print it
int get_response() {
    // parse the pid to get the filename "to_client_[pid]"
    int len = snprintf(NULL, 0, "to_client_%d", getpid());
    char* path = (char*)calloc(len+1, sizeof(char));
    sprintf(path, "to_client_%d", getpid());

    // read from file
    int fd = open(path, O_RDONLY);
    if (fd < 0) { return -1; }

    // read one char at a time and print it. naive approach
    char c; ssize_t n;
    while ((n = read(fd, &c, 1)) > 0) { printf("%c", c); }
    if (n < 0) { close(fd); return -1; }

    // delete "to_client_[pid]" file
    if (close(fd) < 0) { return -1; }
    if (remove(path) < 0) { return -1; } 
    return 0;
}

// if the server responded, print the reply and end
void server_response(int signum) {
    // read from the shared file and print result
    if (get_response() < 0) { ERROR; exit(-1); }
    exit(0);
}

// die if alarm goes off
void no_response(int signum) {
    printf("Client closed because no reponse was received from the server for 30 seconds");
    exit(-1);
}

int main(int argc, char *argv[]) {

    // test and parse input
    struct config conf;
    if (test_args(argc, argv, &conf) < 0) { ERROR; return -1; }

    // try to create 'to_serve' file
    // wait if currently used. otherwise write config to it
    if (get_srv_file(&conf) < 0) { ERROR; return -1; }

    // signal server and wait for 30 seconds
    signal(SIGALRM, no_response);
    signal(SIGUSR1, server_response);

    // timer
    kill(conf.pid, SIGUSR1);
    if (alarm(30) < 0) { ERROR; return -1; }
    pause();
}
