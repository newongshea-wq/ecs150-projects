#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <string>
using namespace std;

void grep(int fd, const string &t) {
    char buf[65536]; 
    string s, out; // s (str) is not processed yet
    ssize_t n;
    while ((n = read(fd, buf, sizeof buf)) > 0) { // adds n bytes ONLY
        s.append(buf, n);
        size_t a = 0, b; // a = current, b = newline 
        while ((b = s.find('\n', a)) != string::npos) { // search from a to find newline, stops if not found
            string line = s.substr(a, b - a + 1); // b-a= line length wo newline, +1 keeps nl
            if (line.find(t) != string::npos) out += line;
            a = b + 1; // move to start of nextline
        }
        s.erase(0, a); // clear processed
        write(1, out.data(), out.size()); out.clear(); // per chunk instead of per line
    }
    if (s.find(t) != string::npos) write(1, s.data(), s.size());
}
int main(int argc, char *argv[]) {
    if (argc < 2) { puts("wgrep: searchterm [file ...]"); return 1; }
    if (argc == 2) grep(0, argv[1]);
    for (int i = 2; i < argc; i++) {
        int fd = open(argv[i], O_RDONLY);
        if (fd < 0) { puts("wgrep: cannot open file"); return 1; }
        grep(fd, argv[1]); close(fd);
    }
}