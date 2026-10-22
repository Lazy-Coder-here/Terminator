# Terminator: C++ learning checkpoints

We are restarting the project in C++ and working through all 11 tasks,
one small checkpoint at a time. Every checkpoint uses commented code,
a simple explanation, and an interactive check before advancing.

## Current checkpoint

Task 1, checkpoints 1-3 are complete: the user confirmed the window, input,
and two-tab checks worked in `task1_window.cpp`, `task1_input.cpp`, and
`task1_tabs.cpp`.

The current checkpoint is Task 1, checkpoint 4: `task1_tab_controls.cpp` keeps
text editing inside `Tab` and introduces `TabManager` to open, close, and select
tabs. Start with one tab; this checkpoint supports six numbered slots. Ctrl+T
opens the first free slot, Ctrl+Shift+W closes the selected tab, Ctrl+Tab selects
the next open tab, and F1-F6 select open slots directly. Closing the final tab
exits. Reopening a closed slot starts with fresh text. Interactive confirmation
is pending. The new file compiled successfully with C++17, `-Wall`, `-Wextra`,
and `-Wpedantic` on 22 October 2026, with no warnings.

Each tab accepts printable ASCII characters, up to 80 characters per entry.
Enter saves and displays text in the selected tab. Later checkpoints will add
command execution, scrolling, and the required thread/process modes. Ctrl+W is
reserved for deleting the previous word in the later editing task.

## Build and run

Run these commands in Ubuntu:

```bash
# Enter the folder containing the C++ checkpoints.
cd /mnt/f/Terminator/cpp

# Compile with C++17 and useful warnings, then link the X11 library.
g++ -std=c++17 -Wall -Wextra -Wpedantic task1_tab_controls.cpp -o task1_tab_controls -lX11

# Run the executable from the current folder.
./task1_tab_controls
```

To revisit earlier checkpoints, substitute `task1_window.cpp` / `task1_window`
or `task1_input.cpp` / `task1_input`, or `task1_tabs.cpp` / `task1_tabs`,
in the compile and run commands above.

## Checks for the current checkpoint

1. In Tab 1, type `apple` and leave it unsubmitted.
2. Press Ctrl+T; Tab 2 should open with empty input and no submitted text.
3. Type `banana` and press Enter; Tab 2 should show it as submitted text.
4. Press F1; unfinished `apple` should still be in Tab 1's input.
5. Press Enter in Tab 1, then use Ctrl+Tab; each tab should keep its own
   submitted text (`apple` and `banana`).
6. Open Tab 3 with Ctrl+T. Press F2 and then Ctrl+Shift+W; Tab 2 should close
   and Tab 3 should become selected. Ctrl+Tab should skip the closed slot.
7. Press Ctrl+T again; the reopened Tab 2 should contain fresh text. Tab 1's
   submitted `apple` should remain intact. F1-F6 should select only open slots.
8. Open tabs until six are open. Another Ctrl+T should show a limit notice.
   Close one tab and open another; it should reuse the free slot normally.
9. Check Backspace, the 80-character input limit, and minimise/restore as before.
10. Close tabs with Ctrl+Shift+W until none remain; the program should return
    to the Ubuntu prompt. Restart it and verify the window's Close button too.

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
