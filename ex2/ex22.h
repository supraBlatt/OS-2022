#include <stdio.h> 
#include <stdlib.h> 
#include <fcntl.h>
#include <string.h>
#include <ctype.h> // used for isspace, isalpha, tolower. QoL

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <wait.h> 

#define MAXPATH 151

// macro hell bois
#define ERROR_OPEN "Error in: open"
#define ERROR_READ "Error in: read"
#define ERROR_STAT "Error in:stat"
#define ERROR_OPEN_DIR "Error in: opendir"
#define ERROR_CLOSE "Error in: close"
#define ERROR_REMOVE "Error in: remove"

#define NO_C_FILE "0"
#define COMPILATION_ERROR "10"
#define WRONG "50"
#define SIMILAR "75"
#define EXCELLENT "100"

#define RESULTS "results.csv"
#define COMPILED_PATH "temp.out"
#define OUTPUT_PATH "temp_out.txt"
#define COMP "./comp.out"
// all errors go to error.txt 

// data struct fork important paths. ad-hoc
typedef struct config {
    char input_fd[MAXPATH]; 
    char output_fd[MAXPATH];
    char results_fd[MAXPATH];
} Config; 
