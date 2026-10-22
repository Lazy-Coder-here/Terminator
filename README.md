# Terminator: C++ learning checkpoints

We are restarting the project in C++ and working through all 11 tasks,
one small checkpoint at a time. Every checkpoint uses commented code,
a simple explanation, and an interactive check before advancing.

## Current checkpoint

Task 1, checkpoints 1-4 are complete: the user confirmed the window, input,
two-tab, and tab-controls checks worked in `task1_window.cpp`, `task1_input.cpp`,
`task1_tabs.cpp`, and `task1_tab_controls.cpp`.

The current checkpoint is Task 1, checkpoint 5: `task1_scrollback.cpp` adds a
`vector<string>` output buffer and a scroll position to each `Tab`. Enter saves
another line, preserving earlier submissions. The window displays up to ten
saved lines at a time; PageUp and PageDown scroll by a page. A new submission
returns the view to the newest lines. Switching tabs preserves each tab's text,
unfinished input, and scroll position. Interactive confirmation is pending.
The source compiled successfully with C++17, `-Wall`, `-Wextra`, and
`-Wpedantic` on 22 October 2026, with no warnings.

The previous tab controls remain: Ctrl+T opens the first free slot,
Ctrl+Shift+W closes the selected tab, Ctrl+Tab selects the next open tab,
and F1-F6 select open slots directly. There are six numbered slots in this
checkpoint. Closing the final tab exits; reopening a closed slot starts fresh.

Each tab accepts printable ASCII characters, up to 80 characters per entry.
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

# Compile with C++17 and useful warnings, then link the X11 library.
g++ -std=c++17 -Wall -Wextra -Wpedantic task1_scrollback.cpp -o task1_scrollback -lX11

# Run the executable from the current folder.
./task1_scrollback
```

To revisit earlier checkpoints, substitute `task1_window.cpp` / `task1_window`
or `task1_input.cpp` / `task1_input`, or `task1_tabs.cpp` / `task1_tabs`,
or `task1_tab_controls.cpp` / `task1_tab_controls`,
in the compile and run commands above.

## Checks for the current checkpoint

1. Press PageUp and PageDown before entering anything; the empty buffer should
   remain empty and the program should remain responsive.
2. In Tab 1, submit `line1`, `line2`, and so on through `line15`, pressing Enter
   after each. The output should retain all 15 lines and show lines 6-15.
3. Press PageUp; lines 1-10 should appear. Press it again; the view should stop
   at the oldest page. PageDown should restore lines 6-15 and stop at the newest.
4. Press PageUp again and type `unfinished` without pressing Enter. Open Tab 2
   with Ctrl+T, submit `banana`, then press F1. Tab 1 should still show lines
   1-10 and its unfinished input. Tab 2 should have its own one-line buffer.
5. Press Enter in Tab 1. It should save line 16 and automatically show lines
   7-16, including the new submission. Tab 2's text should remain intact.
6. Scroll Tab 1 upward, minimise and restore the window, and verify the same
   page and input are redrawn.
7. In Tab 2, PageUp/PageDown should leave its single saved line visible. Close
   it with Ctrl+Shift+W, then reopen it with Ctrl+T; its buffer should be empty.
8. Check the previous tab shortcuts, Backspace, the input limit, and closing
   the final tab or the whole window.

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
