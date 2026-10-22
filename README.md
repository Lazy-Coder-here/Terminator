# Terminator: C++ learning checkpoints

We are restarting the project in C++ and working through all 11 tasks,
one small checkpoint at a time. Every checkpoint uses commented code,
a simple explanation, and an interactive check before advancing.

## Current checkpoint

Task 1, checkpoints 1-5 are complete: the user confirmed the window, input,
two-tab, tab-controls, and scrolling checks worked in `task1_window.cpp`,
`task1_input.cpp`, `task1_tabs.cpp`, `task1_tab_controls.cpp`, and
`task1_scrollback.cpp`.

The current checkpoint is Task 1, checkpoint 6: `task1_workers.cpp` is a short
CONSOLE example before integrating workers into X11. It models two tabs, each
with one worker thread and one private output buffer. Each worker saves a line
every two seconds. The main thread accepts `1` or `2` to view a buffer, `stop1`
or `stop2` to stop one worker independently, and `q` to stop both and quit.
These controls are interpreted directly in C++; no external commands run.
Interactive confirmation is pending.
The example compiled with C++17, `-Wall`, `-Wextra`, `-Wpedantic`, and `-pthread`
on 22 October 2026 without warnings. Automated checks passed for both workers
progressing while input waits, separate buffers, stopping one without stopping
the other, repeated stopping, invalid input, normal exit, and end-of-input exit.

The new `TabWorker` class owns its thread. A mutex protects the saved lines and
stop flag; a condition variable waits without spinning and wakes promptly on a
stop request. The GUI integration checkpoint will use this ownership pattern
while keeping all X11 calls on the GUI thread. Process mode comes later.

The verified graphical checkpoint remains `task1_scrollback.cpp`. It stores
multiple submitted lines in each `Tab`, displays ten at a time, and supports
PageUp/PageDown with a separate scroll position for each tab.

In `task1_scrollback.cpp`, the tab controls remain: Ctrl+T opens the first free slot,
Ctrl+Shift+W closes the selected tab, Ctrl+Tab selects the next open tab,
and F1-F6 select open slots directly. There are six numbered slots in this
checkpoint. Closing the final tab exits; reopening a closed slot starts fresh.

Each graphical tab accepts printable ASCII characters, up to 80 per entry.
Enter saves and displays text in the selected tab. Later checkpoints will add
command execution and the required thread/process modes. Ctrl+W is
reserved for deleting the previous word in the later editing task.

This checkpoint uses a fixed ten-row output area, printable ASCII, and no line
wrapping. Saved lines remain in memory until their tab closes or the program
exits; this buffer is separate from the persistent command history required
later in the assignment.

## Build and run

Run these commands in Ubuntu:

```bash
# Enter the folder containing the C++ checkpoints.
cd /mnt/f/Terminator/cpp

# Compile the console worker example with C++17 and threading support.
g++ -std=c++17 -Wall -Wextra -Wpedantic task1_workers.cpp -o task1_workers -pthread

# Run the executable from the current folder.
./task1_workers
```

To revisit the latest graphical checkpoint, use:

```bash
# Compile the saved X11 scrolling checkpoint.
g++ -std=c++17 -Wall -Wextra -Wpedantic task1_scrollback.cpp -o task1_scrollback -lX11

# Open the graphical window.
./task1_scrollback
```

The earlier window, input, two-tab, and tab-controls files remain available too.

## Checks for the current checkpoint

1. Run the console example. It should display its controls and a `Choice:` prompt.
2. Wait three seconds, then enter `1`; Tab 1 should have saved at least one line.
3. Enter `2`; Tab 2 should also have produced lines although you did not view it.
4. Wait another three seconds without entering anything, then view both tabs;
   both buffers should have grown while the main thread was waiting for input.
5. Enter `stop1`, view Tab 1, and note its saved-line count. Wait three seconds
   and view both tabs again. Tab 1's count should stay fixed; Tab 2 should grow.
6. Enter `stop1` again; repeated stopping should be harmless. Enter an invalid
   choice such as `hello`; it should show the allowed controls.
7. Enter `q`; the program should stop its workers promptly and print
   `All workers stopped.` End-of-input (Ctrl+D at an empty prompt) should also
   stop both workers safely when you run the example again.

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