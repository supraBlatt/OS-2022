#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define ERROR printf("ERROR_FROM_EX4\n")
#define MAGIC_FILE "to_srv"

// trying out this idea..
typedef enum { ADD, SUB, MULT, DIV } operation;
struct config {
    pid_t pid;
    long num1;
    operation op;
    long num2;
};