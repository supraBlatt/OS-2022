#include <stdlib.h>
#include <stdio.h> 
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
// used for isspace, isalpha, tolower. QoL
#include <ctype.h> 

// HIGH KEY ASSUMING IT'S ASCII. THIS DOES NOT WORK ON UTF-8

/* 
 * Read from the file until either EOF is hit, reading fails, 
 * or last character read is not a space character. 
 *
 * RETURN 0 if hit EOF, 
 *        -1 if failed 
 *        the character read otherwise
 */
int read_until_not_space(int fd) {
    char c; 
    ssize_t n;

    // advance the fd until failure or end of file 
    while ((n = read(fd, &c, 1)) > 0) { 
        if (! isspace(c)) { return c; }  
    }

    if (n < 0) { perror("Error in: read"); return -1; } 
    return 0; 
}

int compare_files(char* p1, char* p2) {

    // try to open the files 
    int fd1, fd2;
    if ((fd1 = open(p1, O_RDONLY)) == -1) { perror("Error in: open"); return -1; }  
    if ((fd2 = open(p2, O_RDONLY)) == -1) { perror("Error in: open"); return -1; }  
    if (!strcmp(p1, p2)) { return 1; }

    int diff = 0; 
    char c1, c2; 
    ssize_t n1, n2; 
    int flag1, flag2; 

    // read byte by byte
    while (1) {
        
        if ((n1 = read(fd1, &c1, 1)) < 0) { perror("Error in: read"); return -1; }  
        if ((n2 = read(fd2, &c2, 1)) < 0) { perror("Error in: read"); return -1; } 
       
        // return the flag + 1 [ 1 if the same, 2 if similar ] 
        if (n1 == 0 && n2 == 0) { return diff + 1; } 
        if (n1 == 0 || n2 == 0) {
            // keep reading on the other for spaces only
            int fd = fd1;
            if (n1 == 0) { fd = fd2; }  

            int tmp = read_until_not_space(fd);
            if (tmp < 0) { return -1; }
            return (tmp == 0) ? 2 : 3;
        }

        // at least one character is a space 
        // ad-hoc as fuck
        if (isspace(c1) || isspace(c2)) {
            int tmp1 = -2;
            int tmp2 = -2; 
            if (isspace(c1)) { tmp1 = read_until_not_space(fd1); }
            if (isspace(c2)) { tmp2 = read_until_not_space(fd2); }
            if (tmp1 == -1 || tmp2 == -1) { return -1; }
                
            if (tmp1 == 0 && tmp2 == 0) { return diff + 1; } 
            if (tmp1 == 0 || tmp2 == 0) { return 3; }

            // both are not space. prep for comparison
            c1 = tmp1; 
            c2 = tmp2;
        }
        
        // compare current character read 
        if (c1 == c2) { continue; }

        // if both of them are not space. rip. 
        if (! isspace(c1) && ! isspace(c2)) {
            if (tolower(c1) == tolower(c2)) { diff = 1; continue; }
            return 3; 
        }
    }
    
}

int main(int argc, char* argv[]) {
    if (argc != 3) return -1;
    int a = compare_files(argv[1], argv[2]);
    exit(a); 
}

