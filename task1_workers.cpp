/*
 * Terminator in C++: Task 1, checkpoint 6.
 *
 * This is a small CONSOLE learning example before threads enter the X11 GUI.
 * It models two tabs, each with its own worker thread and output buffer.
 * A worker adds a numbered line every two seconds until it is stopped.
 * The main thread accepts controls and displays snapshots of either buffer.
 * These controls are handled by this C++ program; no commands are executed.
 */

#include <bits/stdc++.h>

using namespace std;

class TabWorker {
private:
    const int tab_number;
    vector<string> output_lines;
    bool stop_requested = false;
    mutable mutex state_mutex;
    condition_variable wake_worker;
    thread worker;

    void run() {
        unique_lock<mutex> lock(state_mutex);
        int line_number = 1;

        while (true) {
            const bool should_stop = wake_worker.wait_for(
                lock, chrono::seconds(2), [this] { return stop_requested; }
            );

            if (should_stop) {
                return;
            }

            output_lines.push_back(
                "Tab " + to_string(tab_number)
                + ": background line " + to_string(line_number)
            );
            line_number++;
        }
    }

public:
    explicit TabWorker(int number) : tab_number(number) {
        worker = thread(&TabWorker::run, this);
    }

    TabWorker(const TabWorker&) = delete;
    TabWorker& operator=(const TabWorker&) = delete;

    ~TabWorker() {
        stop();
    }

    void stop() {
        {
            lock_guard<mutex> lock(state_mutex);
            stop_requested = true;
        }

        wake_worker.notify_one();

        if (worker.joinable()) {
            worker.join();
        }
    }

    vector<string> get_output_snapshot() const {
        lock_guard<mutex> lock(state_mutex);
        return output_lines;
    }
};

void show_tab(const TabWorker& tab, int number) {
    const vector<string> lines = tab.get_output_snapshot();

    cout << "\nTab " << number << " has " << lines.size() << " saved lines.\n";

    if (lines.empty()) {
        cout << "No lines yet. Wait at least two seconds, then view it again.\n";
    }
    else {
        for (const string& line : lines) {
            cout << line << '\n';
        }
    }
}

int main() {
    try {
        TabWorker tab1(1);
        TabWorker tab2(2);

        cout << "Task 1: two-worker console example\n"
             << "Each worker saves a line every two seconds.\n"
             << "1: view Tab 1 | 2: view Tab 2\n"
             << "stop1: stop worker 1 | stop2: stop worker 2 | q: quit\n";

        string choice;

        while (true) {
            cout << "\nChoice: " << flush;

            if (!getline(cin, choice)) {
                break;
            }

            if (choice == "1") {
                show_tab(tab1, 1);
            }
            else if (choice == "2") {
                show_tab(tab2, 2);
            }
            else if (choice == "stop1") {
                tab1.stop();
                cout << "Worker 1 stopped. Its saved lines can still be viewed.\n";
            }
            else if (choice == "stop2") {
                tab2.stop();
                cout << "Worker 2 stopped. Its saved lines can still be viewed.\n";
            }
            else if (choice == "q") {
                break;
            }
            else {
                cout << "Use 1, 2, stop1, stop2, or q.\n";
            }
        }

        cout << "Stopping any remaining workers...\n";
    }
    catch (const exception& error) {
        cerr << "Worker example failed: " << error.what() << '\n';
        return 1;
    }

    cout << "All workers stopped.\n";
    return 0;
}