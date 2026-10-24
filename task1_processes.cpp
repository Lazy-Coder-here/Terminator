/*
 * Terminator in C++: Task 1, checkpoint 8.
 *
 * A CONSOLE example before adding process mode to the X11 window.
 * The parent manages two simulated tabs. Each tab has one child process.
 * Children send a numbered message every two seconds through ordinary pipes.
 * Only the parent stores the received lines and prints requested snapshots.
 * Controls are interpreted here in C++; no shell or external command is run.
 */

#include <bits/stdc++.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <sys/wait.h>

using namespace std;

const int PROCESS_TAB_COUNT = 2;

class ProcessTabs {
private:
    int pipes[PROCESS_TAB_COUNT][2] = {{-1, -1}, {-1, -1}};
    pid_t child_pids[PROCESS_TAB_COUNT] = {-1, -1};
    vector<string> output_lines[PROCESS_TAB_COUNT];
    string partial_lines[PROCESS_TAB_COUNT];

    static void close_descriptor(int& descriptor) noexcept {
        if (descriptor >= 0) {
            close(descriptor);
            descriptor = -1;
        }
    }

    void reap_child(int index) noexcept {
        if (child_pids[index] <= 0) {
            return;
        }

        pid_t result;
        do {
            result = waitpid(child_pids[index], nullptr, 0);
        } while (result == -1 && errno == EINTR);

        if (result >= 0 || errno == ECHILD) {
            child_pids[index] = -1;
        }
        else {
            perror("Collecting child exit status");
        }
    }

    [[noreturn]] void run_child(int index) noexcept {
        const int output_descriptor = pipes[index][1];

        for (int tab = 0; tab < PROCESS_TAB_COUNT; tab++) {
            for (int end = 0; end < 2; end++) {
                if (pipes[tab][end] != output_descriptor) {
                    close(pipes[tab][end]);
                }
            }
        }

        try {
            int line_number = 1;
            while (true) {
                sleep(2);
                const string message = "Tab " + to_string(index + 1)
                    + " [PID " + to_string(getpid()) + "]: background line "
                    + to_string(line_number++) + "\n";

                size_t sent = 0;
                while (sent < message.size()) {
                    const ssize_t count = write(
                        output_descriptor, message.data() + sent, message.size() - sent
                    );
                    if (count > 0) {
                        sent += static_cast<size_t>(count);
                    }
                    else if (count == -1 && errno == EINTR) {
                        continue;
                    }
                    else {
                        _exit(1);
                    }
                }
            }
        }
        catch (...) {
            _exit(1);
        }
    }

    void cleanup() noexcept {
        for (int index = 0; index < PROCESS_TAB_COUNT; index++) {
            stop(index);
        }
        for (int index = 0; index < PROCESS_TAB_COUNT; index++) {
            close_descriptor(pipes[index][0]);
            close_descriptor(pipes[index][1]);
        }
    }

public:
    ProcessTabs() {
        try {
            for (int index = 0; index < PROCESS_TAB_COUNT; index++) {
                if (pipe(pipes[index]) == -1) {
                    throw system_error(errno, generic_category(), "Creating child pipe");
                }
                for (int descriptor : pipes[index]) {
                    if (fcntl(descriptor, F_SETFD, FD_CLOEXEC) == -1) {
                        throw system_error(errno, generic_category(), "Configuring pipe");
                    }
                }
                if (fcntl(pipes[index][0], F_SETFL, O_NONBLOCK) == -1) {
                    throw system_error(errno, generic_category(), "Configuring pipe read");
                }
            }

            for (int index = 0; index < PROCESS_TAB_COUNT; index++) {
                const pid_t result = fork();
                if (result == -1) {
                    throw system_error(errno, generic_category(), "Creating child process");
                }
                if (result == 0) {
                    run_child(index);
                }
                child_pids[index] = result;
            }

            for (int index = 0; index < PROCESS_TAB_COUNT; index++) {
                close_descriptor(pipes[index][1]);
            }
        }
        catch (...) {
            cleanup();
            throw;
        }
    }

    ProcessTabs(const ProcessTabs&) = delete;
    ProcessTabs& operator=(const ProcessTabs&) = delete;

    ~ProcessTabs() {
        cleanup();
    }

    int get_read_descriptor(int index) const {
        return pipes[index][0];
    }

    pid_t get_child_pid(int index) const {
        return child_pids[index];
    }

    const vector<string>& get_output_lines(int index) const {
        return output_lines[index];
    }

    void stop(int index) noexcept {
        if (child_pids[index] <= 0) {
            return;
        }
        if (kill(child_pids[index], SIGTERM) == -1 && errno != ESRCH) {
            perror("Stopping child process");
            return;
        }
        reap_child(index);
    }

    void collect_output(int index) {
        if (pipes[index][0] < 0) {
            return;
        }

        char bytes[256];
        while (true) {
            const ssize_t count = read(pipes[index][0], bytes, sizeof(bytes));
            if (count > 0) {
                partial_lines[index].append(bytes, static_cast<size_t>(count));
                size_t newline;
                while ((newline = partial_lines[index].find('\n')) != string::npos) {
                    output_lines[index].push_back(partial_lines[index].substr(0, newline));
                    partial_lines[index].erase(0, newline + 1);
                }
            }
            else if (count == 0) {
                if (!partial_lines[index].empty()) {
                    output_lines[index].push_back(partial_lines[index]);
                    partial_lines[index].clear();
                }
                close_descriptor(pipes[index][0]);
                reap_child(index);
                return;
            }
            else if (errno == EINTR) {
                continue;
            }
            else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return;
            }
            else {
                throw system_error(errno, generic_category(), "Reading child output");
            }
        }
    }
};

void show_tab(const ProcessTabs& tabs, int index) {
    const vector<string>& lines = tabs.get_output_lines(index);
    cout << "\nTab " << index + 1 << " has " << lines.size() << " saved lines.\n";
    if (lines.empty()) {
        cout << "No lines yet. Wait two seconds, then view it again.\n";
    }
    for (const string& line : lines) {
        cout << line << '\n';
    }
}

bool handle_choice(const string& choice, ProcessTabs& tabs) {
    if (choice == "1" || choice == "2") {
        show_tab(tabs, choice[0] - '1');
    }
    else if (choice == "stop1" || choice == "stop2") {
        const int index = choice.back() - '1';
        tabs.stop(index);
        tabs.collect_output(index);
        cout << "Child " << index + 1 << " stopped and reaped.\n";
    }
    else if (choice == "q") {
        return false;
    }
    else {
        cout << "Use 1, 2, stop1, stop2, or q.\n";
    }
    return true;
}

int main() {
    try {
        ProcessTabs tabs;
        cout << "Task 1: two-process console example\n"
             << "Parent PID: " << getpid() << '\n'
             << "Tab 1 child PID: " << tabs.get_child_pid(0) << '\n'
             << "Tab 2 child PID: " << tabs.get_child_pid(1) << '\n'
             << "1: view Tab 1 | 2: view Tab 2\n"
             << "stop1: stop child 1 | stop2: stop child 2 | q: quit\n"
             << "\nChoice: " << flush;

        bool running = true;
        string pending_input;
        while (running) {
            pollfd sources[3] = {
                {STDIN_FILENO, POLLIN, 0},
                {tabs.get_read_descriptor(0), POLLIN, 0},
                {tabs.get_read_descriptor(1), POLLIN, 0}
            };
            int ready;
            do {
                ready = poll(sources, 3, -1);
            } while (ready == -1 && errno == EINTR);
            if (ready == -1) {
                throw system_error(errno, generic_category(), "Waiting for pipe events");
            }

            for (const pollfd& source : sources) {
                if (source.revents & (POLLERR | POLLNVAL)) {
                    throw runtime_error("An input descriptor became unavailable.");
                }
            }
            for (int index = 0; index < PROCESS_TAB_COUNT; index++) {
                if (sources[index + 1].revents & (POLLIN | POLLHUP)) {
                    tabs.collect_output(index);
                }
            }

            if (sources[0].revents & (POLLIN | POLLHUP)) {
                char input[128];
                const ssize_t count = read(STDIN_FILENO, input, sizeof(input));
                if (count == 0) {
                    break;
                }
                if (count < 0) {
                    if (errno == EINTR) {
                        continue;
                    }
                    throw system_error(errno, generic_category(), "Reading controls");
                }

                pending_input.append(input, static_cast<size_t>(count));
                size_t newline;
                while (running && (newline = pending_input.find('\n')) != string::npos) {
                    string choice = pending_input.substr(0, newline);
                    pending_input.erase(0, newline + 1);
                    if (!choice.empty() && choice.back() == '\r') {
                        choice.pop_back();
                    }
                    tabs.collect_output(0);
                    tabs.collect_output(1);
                    running = handle_choice(choice, tabs);
                    if (running) {
                        cout << "\nChoice: " << flush;
                    }
                }
            }
        }
        cout << "Stopping any remaining children...\n";
    }
    catch (const exception& error) {
        cerr << "Process example failed: " << error.what() << '\n';
        return 1;
    }
    cout << "All children stopped and reaped.\n";
    return 0;
}
