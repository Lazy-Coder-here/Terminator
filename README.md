# Terminator: C++ learning checkpoints

We are restarting the project in C++ and working through all 11 tasks,
one small checkpoint at a time. Every checkpoint uses commented code,
a simple explanation, and an interactive check before advancing.

## Current checkpoint

Task 1, checkpoints 1-6 are complete: the user confirmed the window, input,
two-tab, tab-controls, and scrolling checks worked in `task1_window.cpp`,
`task1_input.cpp`, `task1_tabs.cpp`, `task1_tab_controls.cpp`, and
`task1_scrollback.cpp`, and the console-worker checks in `task1_workers.cpp`.

The current checkpoint is Task 1, checkpoint 7: `task1_gui_workers.cpp` connects
one worker thread to each open GUI tab. A worker queues a numbered message every
two seconds, including while its tab is hidden. The GUI transfers queued messages
into each tab's output buffer and handles all X11 calls itself.
Interactive confirmation is pending.
The source compiled with C++17, `-Wall`, `-Wextra`, `-Wpedantic`, `-lX11`, and
`-pthread` on 22 October 2026 without warnings.
Automated checks passed for pipe wake-ups, hidden-tab progress, separate buffers,
stable older pages during new output, independent worker cleanup, fresh workers
on reopening, all six workers, the tab limit, and closing the pipe on scope exit.
These checks exercise the actual classes without opening an X11 window; the
interactive GUI checks below remain for the user.

`TabWorker` owns its thread and mutex-protected queue. `WakeupPipe` provides an
ordinary nonblocking pipe used only to notify the GUI that output is available.
The GUI uses `poll()` to wait for either X11 events or worker notifications.
The pipe contains wake-up bytes; actual text is transferred from the queues.
`TabManager` owns each worker through a `unique_ptr`, so closing a tab stops
and joins its worker before resetting the tab. Workers stop before the shared
pipe closes on application exit. Worker exceptions are forwarded to the GUI.

Output still displays ten lines at a time. Scrolling up holds the current older
page in place as new messages arrive; offset zero follows the newest text.
Enter saves typed text and returns the view to the newest lines. These messages
demonstrate worker integration; command execution and process mode come later.

The tab controls remain: Ctrl+T opens the first free slot,
Ctrl+Shift+W closes the selected tab, Ctrl+Tab selects the next open tab,
and F1-F6 select open slots directly. There are six numbered slots in this
checkpoint. Closing the final tab exits; reopening a closed slot starts fresh.

Each graphical tab accepts printable ASCII characters, up to 80 per entry.
Enter saves and displays text in the selected tab. Later checkpoints will add
command execution and process mode. Ctrl+W is
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

# Compile the GUI worker checkpoint with X11 and threading support.
g++ -std=c++17 -Wall -Wextra -Wpedantic task1_gui_workers.cpp -o task1_gui_workers -lX11 -pthread

# Run the executable from the current folder.
./task1_gui_workers
```

To revisit the earlier scrolling checkpoint, use:

```bash
# Compile the saved X11 scrolling checkpoint.
g++ -std=c++17 -Wall -Wextra -Wpedantic task1_scrollback.cpp -o task1_scrollback -lX11

# Open the graphical window.
./task1_scrollback
```

The earlier window, input, two-tab, and tab-controls files remain available too.
The verified console worker example is still available as `task1_workers.cpp`.

## Checks for the current checkpoint

1. Run the GUI checkpoint and wait three seconds. Tab 1 should automatically
   display a numbered background message without any keypress.
2. Type `apple` without pressing Enter, then open Tab 2 with Ctrl+T. Submit
   `banana` there. Its input and messages should belong to Tab 2.
3. Stay in Tab 2 for five seconds, then press F1. Tab 1 should have gained
   background messages while hidden and should retain unfinished `apple`.
4. Submit enough short entries to exceed ten saved lines, press PageUp, and
   remember the first visible line. Wait four seconds: the same older page
   should remain visible while the saved-line count grows. PageDown should
   reach the newest text. Minimise/restore should redraw the correct page.
5. Close Tab 1 with Ctrl+Shift+W. Tab 2 should continue producing messages.
   Reopen Tab 1 with Ctrl+T: old text should be gone, and after two seconds its
   new worker should start with `Tab 1: background line 1`.
6. Open all six tabs, then try Ctrl+T once more. The limit notice should appear;
   switching tabs and typing should remain responsive with all workers active.
7. Close the window while workers are running; it should return promptly to
   Ubuntu. Run it again and close its single tab with Ctrl+Shift+W; that should
   also stop the worker and exit promptly.

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