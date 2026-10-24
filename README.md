# Terminator: C++ learning checkpoints

We are restarting the project in C++ and working through all 11 tasks,
one small checkpoint at a time. Every checkpoint uses commented code,
a simple explanation, and an interactive check before advancing.

## Current checkpoint

Task 1, checkpoints 1-7 are complete: the user confirmed the window, input,
two-tab, tab-controls, and scrolling checks worked in `task1_window.cpp`,
`task1_input.cpp`, `task1_tabs.cpp`, `task1_tab_controls.cpp`, and
`task1_scrollback.cpp`, the console-worker checks in `task1_workers.cpp`, and
the GUI-worker checks in `task1_gui_workers.cpp`.

The current checkpoint is Task 1, checkpoint 8: `task1_processes.cpp` is a console
example before integrating process workers into X11. One parent manages two
simulated tabs, with one child process and one ordinary output pipe per tab.
Each child sends a numbered message every two seconds. The parent stores the
received lines in separate buffers and prints a buffer when asked.
Interactive confirmation is pending.
The source compiled with C++17, `-Wall`, `-Wextra`, and `-Wpedantic` on
23 October 2026 without warnings. Automated checks passed for three distinct
PIDs, separate output, independent stopping, child reaping, repeated stopping,
partial input, CRLF input, batched controls, normal exit, and end-of-input exit.

The `ProcessTabs` class owns both child PIDs, pipe descriptors, and parent-side
buffers. It creates all pipes before forking. Each child closes unrelated ends,
keeps only its own write end, and sends newline-delimited text. The parent closes
its write ends and uses `poll()` to wait for stdin or output from either child.
It handles partial reads and splits complete lines into the correct buffer.
`stop1` and `stop2` send SIGTERM to one positive child PID, then use `waitpid()`
to reap it. Normal exit, EOF, and construction errors clean up owned resources.
This example forks from a single-threaded program and runs no external commands.

The latest verified GUI checkpoint remains `task1_gui_workers.cpp`. It connects
one worker thread to each GUI tab while keeping all X11 calls on the GUI thread.
Process workers will be connected to that GUI in a later checkpoint, including
the required `useProcess=True` selection. External command execution comes later.

In the GUI checkpoint, Ctrl+T opens the first free slot,
Ctrl+Shift+W closes the selected tab, Ctrl+Tab selects the next open tab,
and F1-F6 select open slots directly. There are six numbered slots in this
checkpoint. Closing the final tab exits; reopening a closed slot starts fresh.

Each graphical tab accepts printable ASCII characters, up to 80 per entry.
Enter saves and displays text in the selected tab. Later checkpoints will add
command execution and process mode. Ctrl+W is
reserved for deleting the previous word in the later editing task.

The GUI checkpoint uses a fixed ten-row output area, printable ASCII, and no line
wrapping. Saved lines remain in memory until their tab closes or the program
exits; this buffer is separate from the persistent command history required
later in the assignment.

## Build and run

Run these commands in Ubuntu:

```bash
# Enter the folder containing the C++ checkpoints.
cd /mnt/f/Terminator/cpp

# Compile the console process example with C++17 and useful warnings.
g++ -std=c++17 -Wall -Wextra -Wpedantic task1_processes.cpp -o task1_processes

# Run the executable from the current folder.
./task1_processes
```

To revisit the latest graphical checkpoint, use:

```bash
# Compile the saved X11 worker checkpoint.
g++ -std=c++17 -Wall -Wextra -Wpedantic task1_gui_workers.cpp -o task1_gui_workers -lX11 -pthread

# Open the graphical window.
./task1_gui_workers
```

The earlier window, input, two-tab, and tab-controls files remain available too.
The verified console worker example is still available as `task1_workers.cpp`.

## Checks for the current checkpoint

1. Run the console example. It should print one parent PID and two different
   child PIDs, followed by its controls and a `Choice:` prompt.
2. Wait three seconds, then enter `1` and `2`. Each buffer should contain
   numbered messages identifying its own tab and child PID.
3. Wait another three seconds without viewing Tab 2, then enter `2` again.
   Its buffer should have grown while it was not being viewed.
4. Enter `stop1`, view Tab 1 with `1`, and note its count. Wait three seconds,
   then view both tabs: Tab 1's count should stay fixed while Tab 2's grows.
5. Enter `stop1` again; it should be harmless. Enter `hello`; the program
   should show the allowed controls.
6. Enter `stop2`, then view both buffers. Their saved lines should remain
   available, and neither count should grow after waiting three seconds.
7. Enter `q`; it should return promptly to Ubuntu after printing
   `All children stopped and reaped.` Run it again and enter `q` while both
   children are active. EOF (Ctrl+D at an empty prompt) should also clean up.

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