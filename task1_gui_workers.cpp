/*
 * Terminator in C++: Task 1, checkpoint 7.
 *
 * Goal: connect one background worker to every open X11 tab.
 * Workers queue a numbered message every two seconds, even in hidden tabs.
 * Only the GUI thread calls X11 or edits a Tab's display data.
 * An ordinary pipe wakes the GUI when a worker has queued new output.
 * Enter still saves text; external command execution comes later.
 */

#include <bits/stdc++.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

using namespace std;

const size_t INPUT_CHARACTER_LIMIT = 80;

const size_t VISIBLE_OUTPUT_LINES = 10;

const int TAB_SLOT_COUNT = 6;

class WakeupPipe {
private:
    int descriptors[2];
public:
    WakeupPipe() {
        if (pipe(descriptors) == -1) {
            throw system_error(errno, generic_category(), "Creating wake-up pipe");
        }

        for (int descriptor : descriptors) {
            if (fcntl(descriptor, F_SETFL, O_NONBLOCK) == -1 ||
                fcntl(descriptor, F_SETFD, FD_CLOEXEC) == -1) {
                const int saved_error = errno;
                close(descriptors[0]);
                close(descriptors[1]);
                throw system_error(saved_error, generic_category(),
                                   "Configuring wake-up pipe");
            }
        }
    }

    WakeupPipe(const WakeupPipe&) = delete;
    WakeupPipe& operator=(const WakeupPipe&) = delete;

    ~WakeupPipe() {
        close(descriptors[0]);
        close(descriptors[1]);
    }

    int get_read_descriptor() const {
        return descriptors[0];
    }

    void notify() const noexcept {
        const char notice = 'x';
        ssize_t written;
        do {
            written = write(descriptors[1], &notice, 1);
        } while (written == -1 && errno == EINTR);

        if (written == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("Writing worker notification");
        }
    }

    void drain() const {
        char notices[64];
        while (true) {
            const ssize_t count = read(descriptors[0], notices, sizeof(notices));
            if (count > 0) {
                continue;
            }
            if (count == -1 && errno == EINTR) {
                continue;
            }
            if (count == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                return;
            }
            if (count == 0) {
                throw runtime_error("Wake-up pipe closed unexpectedly.");
            }
            throw system_error(errno, generic_category(), "Reading wake-up pipe");
        }
    }
};

class TabWorker {
private:
    const int tab_number;
    WakeupPipe& notifications;
    mutex state_mutex;
    condition_variable wake_worker;
    bool stop_requested = false;
    vector<string> pending_lines;
    exception_ptr failure;
    thread worker;

    void run() noexcept {
        try {
            unique_lock<mutex> lock(state_mutex);
            int line_number = 1;

            while (!wake_worker.wait_for(
                lock, chrono::seconds(2), [this] { return stop_requested; }
            )) {

                pending_lines.push_back(
                    "Tab " + to_string(tab_number)
                    + ": background line " + to_string(line_number)
                );
                line_number++;
                notifications.notify();
            }
        }
        catch (...) {

            {
                lock_guard<mutex> lock(state_mutex);
                failure = current_exception();
            }
            notifications.notify();
        }
    }

public:
    TabWorker(int number, WakeupPipe& pipe) :
        tab_number(number), notifications(pipe) {
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

    vector<string> take_output() {
        lock_guard<mutex> lock(state_mutex);
        if (failure) {
            rethrow_exception(failure);
        }
        vector<string> batch;
        batch.swap(pending_lines);
        return batch;
    }
};

class Tab {
private:
    string input;
    vector<string> output_lines;
    string notice;
    size_t scroll_offset = 0;
public:
    void append_output(const string& line) {
        output_lines.push_back(line);
        if (scroll_offset > 0) {
            scroll_offset++;
        }
    }

    bool append_character(char character) {
        if (input.size() < INPUT_CHARACTER_LIMIT) {
            input.push_back(character);
            notice.clear();
            return true;
        }

        notice = "Input limit reached: 80 characters.";
        return false;
    }

    void erase_last_character() {
        if (!input.empty()) {
            input.pop_back();
            notice.clear();
        }
    }

    void submit_input() {
        append_output("user@terminator> " + input);
        input.clear();
        scroll_offset = 0;
        notice = "Text saved in this tab's output buffer.";
    }

    void scroll_up() {
        if (output_lines.size() <= VISIBLE_OUTPUT_LINES) {
            return;
        }

        const size_t maximum_offset = output_lines.size() - VISIBLE_OUTPUT_LINES;
        scroll_offset = min(scroll_offset + VISIBLE_OUTPUT_LINES, maximum_offset);
    }

    void scroll_down() {
        if (scroll_offset > VISIBLE_OUTPUT_LINES) {
            scroll_offset -= VISIBLE_OUTPUT_LINES;
        }
        else {
            scroll_offset = 0;
        }
    }

    void set_notice(const string& message) {
        notice = message;
    }

    const string& get_input() const {
        return input;
    }

    const vector<string>& get_output_lines() const {
        return output_lines;
    }

    size_t get_scroll_offset() const {
        return scroll_offset;
    }

    const string& get_notice() const {
        return notice;
    }
};

class TabManager {
private:
    WakeupPipe& notifications;
    Tab tabs[TAB_SLOT_COUNT];
    unique_ptr<TabWorker> workers[TAB_SLOT_COUNT];
    bool open_slots[TAB_SLOT_COUNT]{};
    int active_index = 0;
    int open_count = 1;
public:
    explicit TabManager(WakeupPipe& pipe) : notifications(pipe) {
        workers[0] = make_unique<TabWorker>(1, notifications);
        open_slots[0] = true;
    }

    void open_tab() {
        if (open_count == TAB_SLOT_COUNT) {
            tabs[active_index].set_notice("Tab limit reached: 6 open tabs.");
            return;
        }

        for (int index = 0; index < TAB_SLOT_COUNT; index++) {
            if (!open_slots[index]) {
                workers[index] = make_unique<TabWorker>(index + 1, notifications);

                tabs[index] = Tab{};
                open_slots[index] = true;
                open_count++;
                active_index = index;
                return;
            }
        }
    }

    void select_next_tab() {
        for (int offset = 1; offset <= TAB_SLOT_COUNT; offset++) {
            const int next_index = (active_index + offset) % TAB_SLOT_COUNT;

            if (open_slots[next_index]) {
                active_index = next_index;
                return;
            }
        }
    }

    bool close_active_tab() {
        workers[active_index].reset();
        open_slots[active_index] = false;

        tabs[active_index] = Tab{};
        open_count--;

        if (open_count == 0) {
            return false;
        }

        select_next_tab();
        return true;
    }

    void select_tab(int index) {
        if (is_open(index)) {
            active_index = index;
        }
    }

    bool is_open(int index) const {
        return index >= 0 && index < TAB_SLOT_COUNT && open_slots[index];
    }

    int get_active_index() const {
        return active_index;
    }

    Tab& get_active_tab() {
        return tabs[active_index];
    }

    const Tab& get_active_tab() const {
        return tabs[active_index];
    }

    bool collect_output() {
        bool active_changed = false;
        for (int index = 0; index < TAB_SLOT_COUNT; index++) {
            if (!open_slots[index]) {
                continue;
            }
            const vector<string> batch = workers[index]->take_output();
            for (const string& line : batch) {
                tabs[index].append_output(line);
            }
            if (index == active_index && !batch.empty()) {
                active_changed = true;
            }
        }
        return active_changed;
    }
};

void redraw(Display* display, Window window, GC gc, const TabManager& manager) {
    const Tab& tab = manager.get_active_tab();
    const string prompt_line = "user@terminator> " + tab.get_input() + "_";

    XClearWindow(display, window);

    int label_x = 20;
    for (int index = 0; index < TAB_SLOT_COUNT; index++) {
        if (!manager.is_open(index)) {
            continue;
        }

        string label = "[" + to_string(index + 1);
        if (index == manager.get_active_index()) {
            label += "*";
        }
        label += "]";

        XDrawString(display, window, gc, label_x, 30,
                    label.c_str(), static_cast<int>(label.size()));
        label_x += 80;
    }

    const string tab_help = "Ctrl+T: open | Ctrl+Shift+W: close | Ctrl+Tab: next";
    const string input_help = "F1-F6: select open tab | Backspace: delete | Enter: submit";
    const string scroll_help = "PageUp/PageDown: scroll | Each tab's worker adds a line every 2s";
    XDrawString(display, window, gc, 20, 60,
                tab_help.c_str(), static_cast<int>(tab_help.size()));
    XDrawString(display, window, gc, 20, 85,
                input_help.c_str(), static_cast<int>(input_help.size()));
    XDrawString(display, window, gc, 20, 110,
                scroll_help.c_str(), static_cast<int>(scroll_help.size()));

    const vector<string>& lines = tab.get_output_lines();

    const size_t end_index = lines.size() - tab.get_scroll_offset();
    size_t begin_index = 0;
    if (end_index > VISIBLE_OUTPUT_LINES) {
        begin_index = end_index - VISIBLE_OUTPUT_LINES;
    }

    string output_status = "Output buffer is empty. Submit some text with Enter.";
    if (!lines.empty()) {
        output_status = "Saved lines: " + to_string(lines.size())
                        + " | Showing: " + to_string(begin_index + 1)
                        + "-" + to_string(end_index);
    }
    XDrawString(display, window, gc, 20, 145,
                output_status.c_str(), static_cast<int>(output_status.size()));

    int row = 0;
    for (size_t index = begin_index; index < end_index; index++) {
        const string& line = lines[index];
        XDrawString(display, window, gc, 20, 170 + row * 20,
                    line.c_str(), static_cast<int>(line.size()));
        row++;
    }

    XDrawLine(display, window, gc, 20, 375, 780, 375);
    XDrawString(display, window, gc, 20, 400,
                prompt_line.c_str(), static_cast<int>(prompt_line.size()));
    const string& notice = tab.get_notice();
    XDrawString(display, window, gc, 20, 425,
                notice.c_str(), static_cast<int>(notice.size()));

    XFlush(display);
}

void handle_typing(Tab& tab, KeySym key, const char characters[], int count) {
    if (key == XK_Page_Up) {
        tab.scroll_up();
    }
    else if (key == XK_Page_Down) {
        tab.scroll_down();
    }
    else if (key == XK_BackSpace) {
        tab.erase_last_character();
    }
    else if (key == XK_Return || key == XK_KP_Enter) {
        tab.submit_input();
    }
    else {
        for (int i = 0; i < count; i++) {
            const unsigned char character =
                static_cast<unsigned char>(characters[i]);

            if (character < 32 || character > 126) {
                continue;
            }

            if (!tab.append_character(static_cast<char>(character))) {
                break;
            }
        }
    }
}

int main() {
    Display* display = XOpenDisplay(nullptr);
    if (display == nullptr) {
        cerr << "Cannot open the X display. Run this program from Ubuntu.\n";
        return 1;
    }

    const int screen = DefaultScreen(display);
    const unsigned long black = BlackPixel(display, screen);
    const unsigned long white = WhitePixel(display, screen);

    const Window window = XCreateSimpleWindow(
        display, RootWindow(display, screen),
        100, 100, 800, 450, 1, black, white
    );
    XStoreName(display, window, "Terminator C++ - Task 1: GUI worker threads");
    XSelectInput(display, window, ExposureMask | KeyPressMask);

    Atom wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", false);
    const Atom wm_protocols = XInternAtom(display, "WM_PROTOCOLS", false);
    XSetWMProtocols(display, window, &wm_delete_window, 1);

    const GC gc = XCreateGC(display, window, 0, nullptr);
    XSetForeground(display, gc, black);

    int exit_status = 0;
    try {
        WakeupPipe notifications;
        TabManager manager(notifications);
        bool running = true;
        XMapWindow(display, window);

        while (running) {
            bool needs_redraw = false;

            while (running && XPending(display) > 0) {
                XEvent event{};
                XNextEvent(display, &event);

                if (event.type == Expose && event.xexpose.count == 0) {
                    needs_redraw = true;
                }
                else if (event.type == MappingNotify) {
                    XRefreshKeyboardMapping(&event.xmapping);
                }
                else if (event.type == KeyPress) {
                    char characters[32];
                    KeySym key;
                    const int count = XLookupString(
                        &event.xkey, characters, static_cast<int>(sizeof(characters)),
                        &key, nullptr
                    );

                    const bool control = (event.xkey.state & ControlMask) != 0;
                    const bool shift = (event.xkey.state & ShiftMask) != 0;
                    const bool alt = (event.xkey.state & Mod1Mask) != 0;

                    if (control && !shift && !alt && (key == XK_t || key == XK_T)) {
                        manager.open_tab();
                    }
                    else if (control && shift && !alt &&
                             (key == XK_w || key == XK_W)) {

                        running = manager.close_active_tab();
                    }
                    else if (control && !shift && !alt && key == XK_Tab) {
                        manager.select_next_tab();
                    }
                    else if (control || alt) {
                        continue;
                    }
                    else if (key >= XK_F1 && key <= XK_F6) {
                        manager.select_tab(static_cast<int>(key - XK_F1));
                    }
                    else {
                        handle_typing(manager.get_active_tab(), key, characters, count);
                    }
                    needs_redraw = true;
                }
                else if (event.type == ClientMessage &&
                         event.xclient.message_type == wm_protocols &&
                         event.xclient.format == 32 &&
                         static_cast<Atom>(event.xclient.data.l[0]) == wm_delete_window) {
                    running = false;
                }
            }

            if (!running) {
                break;
            }

            const bool output_changed = manager.collect_output();
            if (needs_redraw || output_changed) {
                redraw(display, window, gc, manager);
            }

            if (XPending(display) > 0) {
                continue;
            }

            pollfd sources[2] = {
                {ConnectionNumber(display), POLLIN, 0},
                {notifications.get_read_descriptor(), POLLIN, 0}
            };
            int ready;
            do {
                ready = poll(sources, 2, -1);
            } while (ready == -1 && errno == EINTR);

            if (ready == -1) {
                throw system_error(errno, generic_category(), "Waiting for GUI events");
            }
            for (const pollfd& source : sources) {
                if (source.revents & (POLLERR | POLLHUP | POLLNVAL)) {
                    throw runtime_error("An event connection became unavailable.");
                }
            }
            if (sources[1].revents & POLLIN) {
                notifications.drain();
            }
        }

    }
    catch (const exception& error) {

        cerr << "GUI worker checkpoint failed: " << error.what() << '\n';
        exit_status = 1;
    }

    XFreeGC(display, gc);
    XDestroyWindow(display, window);
    XCloseDisplay(display);
    return exit_status;
}
