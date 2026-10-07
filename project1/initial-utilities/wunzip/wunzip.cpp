#include <fcntl.h>
#include <unistd.h>
#include <iostream>
using namespace std;

// mostly just a reverse of wzip, unpacks the run len encoded stuff

const int IN_SIZE = 5 * 8192;
const int OUT_SIZE = 65536;
char inBuffer[IN_SIZE];
char outBuffer[OUT_SIZE];
int outLen = 0;

void flushOutput() {
    if (outLen > 0) {
        write(STDOUT_FILENO, outBuffer, outLen);
        outLen = 0;
    }
}

void addChar(char c) {
    if (outLen == OUT_SIZE) {
        flushOutput();
    }
    outBuffer[outLen] = c;
    outLen++;
}

int readFull(int fd, char *buf, int size) {
    int total = 0;
    while (total < size) {
        int n = read(fd, buf + total, size - total);
        if (n <= 0) {
            break;
        }
        total += n;
    }
    return total;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cout << "wunzip: file1 [file2 ...]" << endl;
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        int fd = open(argv[i], O_RDONLY);
        if (fd < 0) {
            cout << "wunzip: cannot open file" << endl;
            return 1;
        }

        int bytesRead;
        while ((bytesRead = readFull(fd, inBuffer, IN_SIZE)) >= 5) {
            for (int pos = 0; pos + 5 <= bytesRead; pos += 5) {
                int count = *(int *)(inBuffer + pos);
                char c = inBuffer[pos + 4];

                for (int k = 0; k < count; k++) {
                    addChar(c);
                }
            }
        }
        close(fd);
    }

    flushOutput();
    return 0;
}