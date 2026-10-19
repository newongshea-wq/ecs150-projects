#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

using namespace std;

// using a global search path that starts with /bin only
static vector<string> search_path = {"/bin"};

// the one error message that was given
static void print_error() {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}

// split string into tokens separated by whitespace.
// > is always own token
static vector<string> token(const string &s) {
    vector<string> tokens;
    string cur;

    for (char c : s) {
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            if (!cur.empty()) { tokens.push_back(cur); cur.clear(); }
        } else if (c == '>') {
            if (!cur.empty()) { tokens.push_back(cur); cur.clear(); }
            tokens.push_back(">");
        } else {
            cur += c;
        }
    }
    if (!cur.empty()) { tokens.push_back(cur); }
    return tokens;
}

// look thru search path for an exe, then return "" if not found
static string find_exe(const string &cmd) {
    for (const string &dir : search_path) {
        string full = dir + "/" + cmd;
        if (access(full.c_str(), X_OK) == 0) { return full; }
    }
    return "";
}

// run one command, if launches child, return pid. OW return -1
static pid_t run_command(const string &text) {
    vector<string> tokens = token(text);
    if (tokens.empty()) { return -1; }

    // redirection
    vector<string> args;
    string outfile;
    bool redirect = false;
    for (size_t i = 0; i < tokens.size(); i++) {
        if (tokens[i] == ">") {
            if (redirect) { print_error(); return -1; } // multiple '>'
            redirect = true;
            // only works if exactly one filename follows, and nothing after
            if (i + 2 != tokens.size() || tokens[i + 1] == ">") {
                print_error();
                return -1;
            } // no file after >
            outfile = tokens[i + 1];
            break;
        } else {
            args.push_back(tokens[i]);
        }
    }

    if (args.empty()) { print_error(); return -1; } // > file with no command

    // built in commands:
    if (args[0] == "exit") {
        if (args.size() != 1) { print_error(); return -1; }
        exit(0);
    }
    if (args[0] == "cd") {
        if (args.size() != 2 || chdir(args[1].c_str()) != 0) {
            print_error();
        }
        return -1; // always return, built ins never fork (this was the bug)
    }
    if (args[0] == "path") {
        search_path.assign(args.begin() + 1, args.end());
        return -1;
    }

    // external command
    string exe = find_exe(args[0]);
    if (exe.empty()) { print_error(); return -1; }

    pid_t pid = fork();
    if (pid < 0) { print_error(); return -1; } // fork
    if (pid == 0) { // child
        if (redirect) {
            int fd = open(outfile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) { print_error(); _exit(1); }
            if (dup2(fd, STDOUT_FILENO) < 0 || dup2(fd, STDERR_FILENO) < 0) {
                print_error();
                _exit(1);
            }
            close(fd);
        }
        vector<char *> argv;
        for (string &a : args) argv.push_back(const_cast<char *>(a.c_str()));
        argv.push_back(nullptr);
        execv(exe.c_str(), argv.data());
        // only reaches here if execv fails
        print_error();
        _exit(1);
    }
    return pid;
}

// run one full line, split on &, launch all then wait for all to finish
static void run_line(const string &line) {
    vector<pid_t> pids;
    size_t start = 0;
    while (true) {
        size_t amp = line.find('&', start);
        string piece = line.substr(start, amp == string::npos ? string::npos : amp - start);
        pid_t pid = run_command(piece);
        if (pid > 0) { pids.push_back(pid); }
        if (amp == string::npos) { break; }
        start = amp + 1;
    }
    for (pid_t pid : pids) waitpid(pid, nullptr, 0);
}

int main(int argc, char *argv[]) {
    FILE *input = stdin;
    bool interactive = true;

    if (argc > 2) { print_error(); exit(1); }
    if (argc == 2) {
        input = fopen(argv[1], "r");
        if (input == nullptr) { print_error(); exit(1); }
        interactive = false;
    }

    char *buf = nullptr;
    size_t buf_cap = 0;
    while (true) {
        if (interactive) {
            printf("wish> ");
            fflush(stdout);
        }
        ssize_t n = getline(&buf, &buf_cap, input);
        if (n < 0) { break; } // means EOF
        run_line(string(buf));
    }
    free(buf);
    exit(0);
}