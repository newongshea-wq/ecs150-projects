#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
using namespace std;

// simple run length encoding/compression

const int OUT_SIZE = 5 * 8192;  
char outBuffer[OUT_SIZE];
int outLen = 0;

void flushOutput() {
    if (outLen > 0) {
        write(STDOUT_FILENO, outBuffer, outLen);
        outLen = 0;
    }
}

void addRun(int count, char c) {
    if (outLen + 5 > OUT_SIZE) {
        flushOutput();
    }
    memcpy(outBuffer + outLen, &count, 4);
    outBuffer[outLen + 4] = c;
    outLen += 5;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cout << "wzip: file1 [file2 ...]" << endl;
        return 1;
    }

    char buffer[4096];
    char current = 0;
    int count = 0;

    for (int i = 1; i < argc; i++) {
        int fd = open(argv[i], O_RDONLY);
        if (fd < 0) {
            cout << "wzip: cannot open file" << endl;
            return 1;
        }

        int bytesRead;
        while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
            for (int j = 0; j < bytesRead; j++) {
                if (count > 0 && buffer[j] == current) {
                    count++;
                } else {
                    if (count > 0) {
                        addRun(count, current);
                    }
                    current = buffer[j];
                    count = 1;
                }
            }
        }
        close(fd);
    }

    if (count > 0) {
        addRun(count, current);
    }
    flushOutput();
    return 0;
}