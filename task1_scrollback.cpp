/*
 * Terminator in C++: Task 1, checkpoint 5.
 *
 * Goal: keep multiple submitted lines in each tab and scroll through them.
 * Tab owns its text buffer and scroll position. TabManager manages open tabs.
 * Enter saves the typed text; command execution comes in a later checkpoint.
 */

#include <bits/stdc++.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

using namespace std;

const size_t INPUT_CHARACTER_LIMIT = 80;
const size_t VISIBLE_OUTPUT_LINES = 10;
const int TAB_SLOT_COUNT = 6;

class Tab {
private:
    string input;
    vector<string> output_lines;
    string notice;
    size_t scroll_offset = 0;

public:
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
        output_lines.push_back("user@terminator> " + input);
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
    Tab tabs[TAB_SLOT_COUNT];
    bool open_slots[TAB_SLOT_COUNT]{};
    int active_index = 0;
    int open_count = 1;

public:
    TabManager() {
        open_slots[0] = true;
    }

    void open_tab() {
        if (open_count == TAB_SLOT_COUNT) {
            tabs[active_index].set_notice("Tab limit reached: 6 open tabs.");
            return;
        }

        for (int index = 0; index < TAB_SLOT_COUNT; index++) {
            if (!open_slots[index]) {
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
    const string scroll_help = "PageUp: older text | PageDown: newer text";

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

    XStoreName(display, window, "Terminator C++ - Task 1: output and scrolling");
    XSelectInput(display, window, ExposureMask | KeyPressMask);

    Atom wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", false);
    const Atom wm_protocols = XInternAtom(display, "WM_PROTOCOLS", false);
    XSetWMProtocols(display, window, &wm_delete_window, 1);

    const GC gc = XCreateGC(display, window, 0, nullptr);
    XSetForeground(display, gc, black);

    TabManager manager;
    bool running = true;

    XMapWindow(display, window);

    while (running) {
        XEvent event{};
        XNextEvent(display, &event);

        if (event.type == Expose && event.xexpose.count == 0) {
            redraw(display, window, gc, manager);
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
            else if (control && shift && !alt && (key == XK_w || key == XK_W)) {
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

            if (running) {
                redraw(display, window, gc, manager);
            }
        }
        else if (event.type == ClientMessage &&
                 event.xclient.message_type == wm_protocols &&
                 event.xclient.format == 32 &&
                 static_cast<Atom>(event.xclient.data.l[0]) == wm_delete_window) {
            running = false;
        }
    }

    XFreeGC(display, gc);
    XDestroyWindow(display, window);
    XCloseDisplay(display);

    return 0;
}