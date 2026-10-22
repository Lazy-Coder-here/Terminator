/*
 * Terminator in C++: Task 1, checkpoint 4.
 *
 * Goal: open, close, and switch tabs while keeping their text separate.
 * Tab stores and edits one tab's text. TabManager manages the open tabs.
 * Enter still saves text for display; command execution comes later.
 */

#include <bits/stdc++.h> 
#include <X11/Xlib.h>    
#include <X11/Xutil.h>   
#include <X11/keysym.h>  

using namespace std;

const size_t INPUT_CHARACTER_LIMIT = 80;

const int TAB_SLOT_COUNT = 6;

class Tab {
private:
    string input;                                 
    string submitted = "(nothing submitted yet)"; 
    string notice;                                

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
        submitted = input;
        input.clear();
        notice = "Text submitted.";
    }

    void set_notice(const string& message) {
        notice = message;
    }

    const string& get_input() const {
        return input;
    }

    const string& get_submitted() const {
        return submitted;
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

    XDrawString(display, window, gc, 20, 80,
                prompt_line.c_str(), static_cast<int>(prompt_line.size()));

    const string tab_help = "Ctrl+T: open | Ctrl+Shift+W: close | Ctrl+Tab: next";
    const string input_help = "F1-F6: select open tab | Backspace: delete | Enter: submit";
    XDrawString(display, window, gc, 20, 110,
                tab_help.c_str(), static_cast<int>(tab_help.size()));
    XDrawString(display, window, gc, 20, 135,
                input_help.c_str(), static_cast<int>(input_help.size()));

    const string label = "Last submitted text in this tab:";
    const string& submitted = tab.get_submitted();
    const string& notice = tab.get_notice();
    XDrawString(display, window, gc, 20, 180,
                label.c_str(), static_cast<int>(label.size()));
    XDrawString(display, window, gc, 20, 205,
                submitted.c_str(), static_cast<int>(submitted.size()));
    XDrawString(display, window, gc, 20, 245,
                notice.c_str(), static_cast<int>(notice.size()));

    XFlush(display);
}

void handle_typing(Tab& tab, KeySym key, const char characters[], int count) {
    if (key == XK_BackSpace) {
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
    XStoreName(display, window, "Terminator C++ - Task 1: tab controls");
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
