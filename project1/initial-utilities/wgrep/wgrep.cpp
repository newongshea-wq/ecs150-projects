#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <string>
using namespace std;

void grepFile(int fd, const string &term) {
    char buffer[4096];
    string line;
    int bytesRead;

    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
        for (int i = 0; i < bytesRead; i++) {
            line += buffer[i];
            if (buffer[i] == '\n') {
                if (line.find(term) != string::npos) {
                    write(STDOUT_FILENO, line.c_str(), line.size());
                }
                line = "";
            }
        }
    }

    if (line.size() > 0 && line.find(term) != string::npos) {
        write(STDOUT_FILENO, line.c_str(), line.size());
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cout << "wgrep: searchterm [file ...]" << endl;
        return 1;
    }

    string term = argv[1];

    if (argc == 2) {
        grepFile(STDIN_FILENO, term);
        return 0;
    }

    for (int i = 2; i < argc; i++) {
        int fd = open(argv[i], O_RDONLY);
        if (fd < 0) {
            cout << "wgrep: cannot open file" << endl;
            return 1;
        }
        grepFile(fd, term);
        close(fd);
    }
    return 0;
}