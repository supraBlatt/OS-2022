#include <stdio.h> 
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

// inspiration taken from
// https://github.com/jmreyes/simple-c-shell/blob/master/simple-c-shell.com
// https://brennan.io/2015/01/16/write-a-shell-in-c/

# define MAXLINE 101 // max number of characters from user input 
# define MAXHIST 101 // max amount of commands in history

char cmd_history[MAXHIST][MAXLINE]; // disgusting
int cur_history;


int cd(int argc, char* argv[]) {

    // assume files don't need sanitazion
    // no support for cd ~ or cd -
    if (argc < 2 || chdir(argv[1]) == -1) {
        perror("cd failed");
        return -1;
    }
    return 0;
}

void add_to_history(pid_t p_pid, char * cmd_name) {
    // add to history array if we have space. vulnerable tho.
    // also assumes there's no need to clear history
    if (cur_history < MAXHIST) snprintf(cmd_history[cur_history++], MAXLINE, "%d %s", p_pid, cmd_name);
}

int history() {
    // print history array 
    for (int i=0; i < cur_history; i++) { printf("%s\n", cmd_history[i]); }
    printf("%d history\n", getpid());
    return 0;
}

void prompt() {
    printf("$ ");
    fflush(stdout);
}

// could be way less disgusting with a {cmd_name:func} dict 
int handle_command(int argc, char * argv[]) {

    // test for builtin commands
    pid_t pid = getpid();
    if (! strcmp(argv[0], "exit")) exit(0); 
    else if (! strcmp(argv[0], "cd")) cd(argc, argv);
    else if (! strcmp(argv[0], "history")) history();

    // a different command. use execvp
    else {
        if ((pid=fork()) == -1) {
            return -1;
        }

        // child process. 
        if (pid == 0) {
            if (execvp(argv[0], argv) == -1) {
                char err[MAXLINE];
                snprintf(err, MAXLINE, "%s failed", argv[0]);
                perror(err);
                _exit(-1);
            }
        } else {

            // parent waits 
            waitpid(pid, NULL, 0);
        }
    }

    // add command to history
    add_to_history(pid, argv[0]);
    return 0;
}

void init(int argc, char* argv[]) {
    if (argc < 2) return;
    
    // length of - :argv[1]:argv[2]:...:argv[argc-1]
    int length = argc - 1; 
    for (int i=1; i<argc; i++) {
        length += strlen(argv[i]);
    }

    // start copying things into a new env variable 
    char* old_path = getenv("PATH");
    int new_length = strlen(old_path) + length + 1;
    char* new_path = calloc(sizeof(char),  new_length); 
    strcat(new_path, old_path);  

    // copy all arguments 
    for (int i=1; i<argc; i++) {
        strcat(new_path, ":");
        strcat(new_path, argv[i]);
    }

    new_path[new_length-1] = '\0';
    setenv("PATH", new_path, 1);
    free(new_path);
}

int main(int argc, char* argv[], char ** envp) {
    // assume 
    // max token amount = maxline length 
    char line[MAXLINE];
    char * tokens[MAXLINE];
    int num_tokens;

    //initialize shell variables from argv 
    init(argc, argv);
    printf("path=%s\n", getenv("PATH"));
    // main shell loop 
    while(1) {
        prompt(); 

        // empty the line buffer and get input 
        memset(line, '\0', MAXLINE);  
        fgets(line, MAXLINE, stdin);

        // if no input, continue. otherwise keep splitting 
        if ((tokens[0] = strtok(line, " \n\t")) == NULL) continue; 
        num_tokens = 1;

        while ((tokens[num_tokens] = strtok(NULL, " \n\t")) != NULL) num_tokens++;
        tokens[num_tokens] = NULL; 

        int a = handle_command(num_tokens, tokens);
        if (a == -1) { break; } 
    }
    
    exit(0); 
}
