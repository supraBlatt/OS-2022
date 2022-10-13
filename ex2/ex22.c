#include "ex22.h"

// this entire code has to be rewritten and re-done. 
// it is quite an atrocity. forgive me for having to read it. 

// root 
// |_ child1 -- .c -> compiling -> running -> comparing output -> grading 
// |_ child2 -- .c -> ..
// |_ child3 -- .c -> ..

void append_path(const char* root, char* file, char path[MAXPATH]) {
    // vuln to overflows. using strncat raises warnings though.
    strcpy(path, root);
    strcat(path, "/");
    strcat(path, file);
}

// spooky big function 
int grade(const char* name, const char* reason, char* grade, const char* result_path) {
    // write to results file
    // [name],[grade],[reason]
    int results_fd = open(result_path, O_CREAT | O_WRONLY | O_APPEND);
    if (results_fd < 0) { perror(ERROR_OPEN); return -1;}
   
    // spooooooky copypaste. no write errors needed to be supported
    if (write(results_fd, name, strlen(name)) < 0 ) { return -1;}
    if (write(results_fd, ",", 1) < 0 ) { return -1;}
    if (write(results_fd, grade, strlen(grade)) < 0 ) { return -1;}
    if (write(results_fd, ",", 1) < 0 ) { return -1;}
    if (write(results_fd, reason, strlen(reason)) < 0 ) { return -1;}
    if (write(results_fd, "\n", 1) < 0) { return -1; } 
    if (close(results_fd) < 0 ) { perror(ERROR_CLOSE); return -1; }
    return 0;
}

int try_compile(char* output, char* c_file) {
    // try to compile the file using gcc and forks 
    pid_t pid;
    int status; 
    if ((pid=fork()) == -1) {
        return -1;
    }
    // child process. 
    if (pid == 0) {
        char* args[] = {"gcc", c_file, "-o", output, NULL};
        if (execvp("gcc", args) == -1) {
            perror("Error in: exec\n");
            _exit(-1);
        }
    } else {
        // parent waits 
        waitpid(pid, &status, 0);
    }
    return WEXITSTATUS(status) != 0 ? -1 : 0;
}

// assumes it's atleast one layer deep.. 
int try_execute(char* exe, char* input, char* output) {
    // try to run the file using fork and exec
    pid_t pid;
    int status;
    if ((pid=fork()) == -1) {
        return -1;
    }
    // child process. 
    if (pid == 0) {
        // get our input settled
        int input_fd = open(input, O_RDONLY);
        if (input_fd < 0) { perror(ERROR_OPEN); return -1;}
        if (dup2(input_fd, STDIN_FILENO) < 0) { return -1; };

        // and output stream too 
        int output_fd = open(output, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);
        if (output_fd < 0) { perror(ERROR_OPEN); return -1;}
        if (dup2(output_fd, STDOUT_FILENO) < 0) { return -1; };
      
        // prepare for exec
        char* args[] = {exe, NULL};
        if (execvp(exe, args) == -1) {
            perror("Error in: exec\n");
            if (close(input_fd) < 0 || close(output_fd) < 0) { perror(ERROR_CLOSE); }
            _exit(-1);
        }
    } else {
        // parent waits 
        waitpid(pid, &status, 0);
    }
    return (WEXITSTATUS(status) != 0) ? -1 : 0;
}

int try_compare(char* output, char* expected) { 

    // try to run the comp.out file and compare 
    pid_t pid;
    int status; 
    if ((pid=fork()) == -1) {
        return -1;
    }

    // child process. 
    if (pid == 0) {
        char* args[] = {COMP, output, expected, NULL};
        if (execvp(COMP, args) == -1) {
            perror("Error in: exec\n");
            _exit(-1);
        }
    } else {
        // parent waits 
        waitpid(pid, &status, 0);
    }
    return WEXITSTATUS(status);
}

// Iterates over root. finds the first c file
// Returns -1 on failure. 0 otherwise.
int find_c_file(const char* root, char* c_file) {
    DIR* d = opendir(root); 
    if (d == NULL) { perror(ERROR_OPEN_DIR); return -1; } 

    // kind of a copy paste. but oh well. idk how to use function pointers
    struct stat dummy;
    struct dirent* entry;
    char path[MAXPATH];
    while ((entry = readdir(d))) {
        
        // full path
        // assuming every directory is a student
        append_path(root, entry->d_name, path);
        if (stat(path, &dummy) < 0) { closedir(d); perror(ERROR_STAT); return -1; }

        if (S_ISREG(dummy.st_mode) && !strcmp(entry->d_name + strlen(entry->d_name)-2, ".c")) {
            strcpy(c_file, path);
            closedir(d);
            return 0;
        }
    }
    closedir(d);
    return -1; 
}

// oui oui looking not disgusting at all
// return -1 if error, 0 otherwise
int handle_student(const char* root, Config* conf, const char* name) { 

    // find c file 
    char c_file[MAXPATH];
    if (find_c_file(root, c_file) < 0) { 
        grade(name, "NO_C_FILE", NO_C_FILE, conf->results_fd);  
        return -1; 
    }

    // copy paste goes brr, try to compile file
    char exe[MAXPATH];
    append_path(root, COMPILED_PATH, exe);
    if (try_compile(exe, c_file) < 0) { 
        grade(name, "COMPILATION_ERROR", COMPILATION_ERROR, conf->results_fd); 
        return -1;
    };

    // try to execute file
    char output[MAXPATH];
    append_path(root, OUTPUT_PATH, output);
    if (try_execute(exe, conf->input_fd, output) < 0) {
       // clean up executable 
       if (remove(exe) != 0) { perror(ERROR_REMOVE); }  
       return -1;  
    }

    // compare file to output 
    int flag = 0;
    int compared = try_compare(output, conf->output_fd);
    if (compared == 1) {  
        grade(name, "EXCELLENT", EXCELLENT, conf->results_fd);
    } else if (compared == 2) {  
        grade(name, "SIMILAR", SIMILAR, conf->results_fd);
    } else if (compared == 3) {
        grade(name, "WRONG", WRONG, conf->results_fd);
    } else {
        flag = -1;
    }
    // clean up executable and output file. ad-hoc 
    if (remove(exe) != 0 || remove(output) != 0) { perror(ERROR_REMOVE); return -1;}
    return flag; 
}

// Serially iterate over each student's folder and grade.
// Could be made concurrent as gradings are independent. 
//   --> only need to sync writing to results.csv  
//
//   CLEAR ALL RESOURCES 
int go_over_dir(const char* root, Config* conf) {
    DIR* d = opendir(root);
    if (d == NULL) { perror(ERROR_OPEN_DIR); return -1; }

    // iterate over root. look for directories. 
    struct dirent* entry;
    struct stat dummy;
    char path[MAXPATH];
    while ((entry = readdir(d))) {
         
        // full path
        append_path(root, entry->d_name, path);

        // assuming every directory is a student
        if (stat(path, &dummy) < 0) { perror(ERROR_STAT); closedir(d); return -1; }
        if (S_ISDIR(dummy.st_mode) && strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..") ) { 
            // we gottem, handle student 
            handle_student(path, conf, entry->d_name);
        }
    }
    return 0;
}

// ad-hoc, could implement better using memchr and buffers
int read_line(const int fd, char path[MAXPATH]) {
    ssize_t n;
    char c; 
    int i = 0;

    // read bytes until hit \n
    while ((n = read(fd, &c, 1)) > 0 && i < MAXPATH) {
        if(c == '\n') { break; }
        path[i++] = c;         
    }

    // handle failure or end
    if (n < 0) { perror(ERROR_READ); return -1; }
    path[i] = '\0';
    return 0;
}

// mode: 1 if file, 0 if directory 
int valid_path(const char* path, const int mode) {
    struct stat dummy;
    if (stat(path, &dummy) < 0) { perror(ERROR_STAT); return 0; }
    return mode ? (S_ISREG(dummy.st_mode)) : (S_ISDIR(dummy.st_mode));
}

// initialize config file (input, output and results path)
// initialize the root path 
// redirect STDERR to errors.txt, and creates results.csv file
// return -1 if an error occured. 0 otherwise
int init(Config* conf, const char* path, char* root) {

    // open errors file and redirect STDERR to oit 
    int err_fd = open("errors.txt", O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);
    if (err_fd < 0) { perror(ERROR_OPEN); return -1;}
    if (dup2(err_fd, STDERR_FILENO) < 0) { return -1; };

    // try to open the config file 
    int config_fd = open(path, O_RDONLY);
    if (config_fd < 0) { 
        perror(ERROR_OPEN); 
        close(err_fd);
        return -1; 
    }

    // FIX PERROR, prints err to errors file as well..
    if (read_line(config_fd, root) < 0 ||
        !valid_path(root, 0)) {
        printf("Not a valid directory\n"); 
        close(config_fd); 
        close(err_fd); 
        return -1;
    }
    if (read_line(config_fd, conf->input_fd) < 0 ||
        !valid_path(conf->input_fd, 1)) {
        printf("Input file not exist\n"); 
        close(config_fd); 
        close(err_fd); 
        return -1;
    }
    if (read_line(config_fd, conf->output_fd) < 0 ||
        !valid_path(conf->output_fd, 1)) {
        printf("Output file not exist\n"); 
        close(config_fd); 
        close(err_fd); 
        return -1;
    }

    strcpy(conf->results_fd, RESULTS);
    int res_fd = open(RESULTS, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);
    if (res_fd < 0) { 
        perror(ERROR_OPEN); 
        close(err_fd);
        close(config_fd);
        return -1; 
    }
    return 0;
}

// this is awful code i am so sorry
// make sure to close files when exiting
int main(int argc, char* argv[]) {
    if (argc < 2) { return -1; }
   
    // initialize config data and root directory
    char root[MAXPATH];
    Config conf; 
    if (init(&conf, argv[1], root) < 0) { return -1; } ;

    // go over student data
    go_over_dir(root, &conf);
    return 0;
}
