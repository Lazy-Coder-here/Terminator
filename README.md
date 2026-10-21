# Terminator: C++ learning checkpoints

We are restarting the project in C++ and working through all 11 tasks,
one small checkpoint at a time. Every checkpoint uses commented code,
a simple explanation, and an interactive check before advancing.

## Current checkpoint

Task 1, checkpoints 1 and 2 are complete: the user confirmed the first window
and all input checks worked in `task1_window.cpp` and `task1_input.cpp`.

The current checkpoint is Task 1, checkpoint 3: `task1_tabs.cpp` groups the input,
submitted text, and notice into a `Tab` class, creates two independent objects,
and switches between them with F1 and F2. The strings are private. Public methods
perform editing, while read-only getters provide text for drawing. Interactive
confirmation is pending.

Each tab accepts printable ASCII characters, up to 80 characters per entry.
Enter saves and displays text in the selected tab. Later checkpoints will add
opening/closing tabs, command execution, and scrolling.

## Build and run

Run these commands in Ubuntu:

```bash
# Enter the folder containing the C++ checkpoints.
cd /mnt/f/Terminator/cpp

# Compile with C++17 and useful warnings, then link the X11 library.
g++ -std=c++17 -Wall -Wextra -Wpedantic task1_tabs.cpp -o task1_tabs -lX11

# Run the executable from the current folder.
./task1_tabs
```

To revisit earlier checkpoints, substitute `task1_window.cpp` / `task1_window`
or `task1_input.cpp` / `task1_input` in the compile and run commands above.

## Checks for the current checkpoint

1. In Tab 1, type `apple` and leave it unsubmitted.
2. Press F2; Tab 2 should start with empty input and no submitted text.
3. Type `banana` and press Enter; Tab 2 should show it as submitted text.
4. Press F1; unfinished `apple` should still be in Tab 1's input.
5. Press Enter in Tab 1, then switch between tabs; each should keep its own
   submitted text (`apple` and `banana`).
6. Edit unfinished text with Backspace in one tab; the other tab's text should
   remain intact. An input-limit notice should also belong only to its tab.
7. Minimise and restore the window; the selected tab's text should be redrawn.
8. Close the window; the program should return to the Ubuntu prompt.

The required compiler, Make, and X11 development package are already installed
in Ubuntu-26.04. Each later checkpoint will explain any new dependencies.

## Requirements for all future checkpoints

- Work through all 11 tasks in the assignment, in order.
- Use C++ with the required Linux/POSIX and Xlib APIs.
- Use `using namespace std;` in the C++ examples, as requested.
- Use `<bits/stdc++.h>` for standard-library headers with Ubuntu's `g++`,
  while including X11 and required Linux/POSIX headers separately.
- Use simple explanations and comment the code thoroughly.
- OOP is welcome throughout all 11 tasks. Use classes and encapsulation wherever
  they make the code clearer, with simple explanations and thorough comments.
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

The earlier C exercises remain in the parent folder for comparison.
