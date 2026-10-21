/*
 * Terminator in C++: Task 1, checkpoint 3.
 *
 * Goal: show two tabs with separate text buffers and switch using F1/F2.
 * Each tab remembers unfinished input, submitted text, and feedback.
 * Enter continues to save text for display in this checkpoint.
 */

#include <bits/stdc++.h> 
#include <X11/Xlib.h>   
#include <X11/Xutil.h>   
#include <X11/keysym.h>  

using namespace std;

const size_t INPUT_CHARACTER_LIMIT = 80;

const int TAB_COUNT = 2;

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

void redraw(Display* display, Window window, GC gc,
            const Tab& tab, int active_tab) {
    const string tab_bar = "[1: F1]    [2: F2]    Active tab: "
                           + to_string(active_tab + 1);

    const string prompt_line = "user@terminator> " + tab.get_input() + "_";

    const string& submitted = tab.get_submitted();
    const string& notice = tab.get_notice();

    XClearWindow(display, window);

    XDrawString(display, window, gc, 20, 30,
                tab_bar.c_str(), static_cast<int>(tab_bar.size()));

    XDrawString(display, window, gc, 20, 80,
                prompt_line.c_str(), static_cast<int>(prompt_line.size()));

    const string help = "F1/F2 switch tabs. Backspace deletes. Enter submits.";
    XDrawString(display, window, gc, 20, 110,
                help.c_str(), static_cast<int>(help.size()));

    const string label = "Last submitted text in this tab:";
    XDrawString(display, window, gc, 20, 155,
                label.c_str(), static_cast<int>(label.size()));
    XDrawString(display, window, gc, 20, 180,
                submitted.c_str(), static_cast<int>(submitted.size()));

    XDrawString(display, window, gc, 20, 220,
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
    XStoreName(display, window, "Terminator C++ - Task 1: two tabs");

    XSelectInput(display, window, ExposureMask | KeyPressMask);

    Atom wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", false);
    const Atom wm_protocols = XInternAtom(display, "WM_PROTOCOLS", false);
    XSetWMProtocols(display, window, &wm_delete_window, 1);

    const GC gc = XCreateGC(display, window, 0, nullptr);
    XSetForeground(display, gc, black);

    Tab tabs[TAB_COUNT];
    int active_tab = 0; 
    bool running = true;

    XMapWindow(display, window);

    while (running) {
        XEvent event{}; 
        XNextEvent(display, &event); 

        if (event.type == Expose && event.xexpose.count == 0) {
            redraw(display, window, gc, tabs[active_tab], active_tab);
        }
        else if (event.type == MappingNotify) {
            XRefreshKeyboardMapping(&event.xmapping);
        }
        else if (event.type == KeyPress) {
            if (event.xkey.state & (ControlMask | Mod1Mask)) {
                continue;
            }

            char characters[32];
            KeySym key;
            const int count = XLookupString(
                &event.xkey, characters, static_cast<int>(sizeof(characters)),
                &key, nullptr
            );

            if (key == XK_F1) {
                active_tab = 0;
            }
            else if (key == XK_F2) {
                active_tab = 1;
            }
            else {
                handle_typing(tabs[active_tab], key, characters, count);
            }

            redraw(display, window, gc, tabs[active_tab], active_tab);
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