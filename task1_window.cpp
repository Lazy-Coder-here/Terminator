/*
 * Terminator in C++: Task 1, checkpoint 1.
 *
 * Goal: open an X11 window, draw a message, and close cleanly.
 * This checkpoint introduces the GUI before typing, tabs, or commands.
 */

#include <bits/stdc++.h> cerr.
#include <X11/Xlib.h>

using namespace std;

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

    XStoreName(display, window, "Terminator C++ - Task 1: first window");

    XSelectInput(display, window, ExposureMask);

    Atom wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", false);
    const Atom wm_protocols = XInternAtom(display, "WM_PROTOCOLS", false);
    XSetWMProtocols(display, window, &wm_delete_window, 1);

    const GC gc = XCreateGC(display, window, 0, nullptr);
    XSetForeground(display, gc, black);

    XMapWindow(display, window);

    const string message = "Rahul was here.";

    bool running = true;
    while (running) {
        XEvent event{};

        XNextEvent(display, &event);

        if (event.type == Expose && event.xexpose.count == 0) {
            XClearWindow(display, window);

            XDrawString(display, window, gc, 30, 50,
                        message.c_str(), static_cast<int>(message.size()));
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
