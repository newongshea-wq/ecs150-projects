#include <fcntl.h>
#include <unistd.h>
#include <iostream>
using namespace std;

int main(int argc, char *argv[]) {
    char buffer[4096];

    for (int i = 1; i < argc; i++) {
        int filedir = open(argv[i], O_RDONLY);
        if (filedir < 0) {
            cout << "wcat: cannot open file" << endl;
            return 1;
        }

        int bytesRead;
        while ((bytesRead = read(filedir, buffer, sizeof(buffer))) > 0) {
            write(STDOUT_FILENO, buffer, bytesRead);
        }
        close(filedir);
    }
    return 0;
}