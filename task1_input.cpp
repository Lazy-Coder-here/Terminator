/*
 * Terminator in C++: Task 1, checkpoint 2.
 *
 * Goal: draw a prompt, accept typing, delete with Backspace, and submit
 * with Enter. Submitted text is displayed inside the window for checking.
 * Command execution, tabs, and scrolling come in later checkpoints.
 */

#include <bits/stdc++.h> 
#include <X11/Xlib.h>    
#include <X11/Xutil.h>   
#include <X11/keysym.h>  

using namespace std;

const size_t INPUT_CHARACTER_LIMIT = 80;

void redraw(Display* display, Window window, GC gc,
            const string& input, const string& submitted, const string& notice) {
    const string prompt_line = "user@terminator> " + input + "_";

    XClearWindow(display, window);

    XDrawString(display, window, gc, 20, 45,
                prompt_line.c_str(), static_cast<int>(prompt_line.size()));

    const string help = "Type text. Backspace deletes. Enter submits.";
    XDrawString(display, window, gc, 20, 75,
                help.c_str(), static_cast<int>(help.size()));

    const string label = "Last submitted text:";
    XDrawString(display, window, gc, 20, 120,
                label.c_str(), static_cast<int>(label.size()));
    XDrawString(display, window, gc, 20, 145,
                submitted.c_str(), static_cast<int>(submitted.size()));

    XDrawString(display, window, gc, 20, 185,
                notice.c_str(), static_cast<int>(notice.size()));

    XFlush(display);
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
    XStoreName(display, window, "Terminator C++ - Task 1: prompt and typing");

    XSelectInput(display, window, ExposureMask | KeyPressMask);

    Atom wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", false);
    const Atom wm_protocols = XInternAtom(display, "WM_PROTOCOLS", false);
    XSetWMProtocols(display, window, &wm_delete_window, 1);

    const GC gc = XCreateGC(display, window, 0, nullptr);
    XSetForeground(display, gc, black);

    string input;                                  
    string submitted = "(nothing submitted yet)";  
    string notice;                                 
    bool running = true;

    XMapWindow(display, window);

    while (running) {
        XEvent event{};

        XNextEvent(display, &event);

        if (event.type == Expose && event.xexpose.count == 0) {
            redraw(display, window, gc, input, submitted, notice);
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

            if (key == XK_BackSpace) {
                if (!input.empty()) {
                    input.pop_back(); 
                    notice.clear();   
                }
            }
            else if (key == XK_Return || key == XK_KP_Enter) {
                submitted = input;  
                input.clear();            
                notice = "Text submitted.";
            }
            else {
                for (int i = 0; i < count; i++) {
                    const unsigned char character =
                        static_cast<unsigned char>(characters[i]);

                    if (character < 32 || character > 126) {
                        continue;
                    }

                    if (input.size() < INPUT_CHARACTER_LIMIT) {
                        input.push_back(static_cast<char>(character));
                        notice.clear();
                    }
                    else {
                        notice = "Input limit reached: 80 characters.";
                        break;
                    }
                }
            }

            redraw(display, window, gc, input, submitted, notice);
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
