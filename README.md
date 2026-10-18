# Terminator: C++ learning checkpoints

We are restarting the project in C++ and working through all 11 tasks,
one small checkpoint at a time. Every checkpoint uses commented code,
a simple explanation, and an interactive check before advancing.

## Current checkpoint

Task 1, checkpoint 1: `task1_window.cpp` creates an X11 window, displays
a message, handles redraw events, and closes cleanly.

The next checkpoint will add a prompt and keyboard input.

## Build and run

Run these commands in Ubuntu:

```bash
# Enter the folder containing the C++ checkpoints.
cd /mnt/f/Terminator/cpp

# Compile with C++17 and useful warnings, then link the X11 library.
g++ -std=c++17 -Wall -Wextra -Wpedantic task1_window.cpp -o task1_window -lX11

# Run the executable from the current folder.
./task1_window
```

The required compiler, Make, and X11 development package are already installed
in Ubuntu-26.04. Each later checkpoint will explain any new dependencies.

## Requirements for all future checkpoints

- Work through all 11 tasks in the assignment, in order.
- Use C++ with the required Linux/POSIX and Xlib APIs.
- Use `using namespace std;` in the C++ examples, as requested.
- Use `<bits/stdc++.h>` for standard-library headers with Ubuntu's `g++`,
  while including X11 and required Linux/POSIX headers separately.
- Use simple explanations and comment the code thoroughly.
- Do not use pseudo-terminals or APIs such as `forkpty`, `openpty`, or
  `posix_openpt` to execute commands.
- Do not delegate command execution to Bash or any other shell, including
  through `system`, `popen`, or shell `-c` invocations.
- Launch external programs directly with `fork` and `execvp`.
- Implement parsing, expansion, redirection, pipelines, and script
  interpretation within Terminator itself.
- Capture directly launched programs' output using ordinary pipes.
- Implement both the required thread mode and process mode as Task 1 develops.
- Treat compilation and interactive GUI verification as separate checks.
