#include <fcntl.h>
#include <unistd.h>
#include <cstdio>

int main(int argc, char *argv[]) {
    char buf[65536]; // bigger just in case, I'm looking at you wzip and wunzip
    for (int i = 1; i < argc; i++) { // when arg is 1, no file given, exits
        int fd = open(argv[i], O_RDONLY); // fd is -1 if the file cannot be opened
        if (fd < 0) { puts("wcat: cannot open file"); return 1; } // exit cond for previous comment
        // write by chunks immediately (to save space): 
        for (ssize_t n; (n = read(fd, buf, sizeof buf)) > 0;) write(1, buf, n);
        // if read is 0 then EOF, but any value > 0 means data. write only n bytes to not have trash
        close(fd);
    }
    return 0;
}