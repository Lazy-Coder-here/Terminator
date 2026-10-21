# Terminator: C++ learning checkpoints

We are restarting the project in C++ and working through all 11 tasks,
one small checkpoint at a time. Every checkpoint uses commented code,
a simple explanation, and an interactive check before advancing.

## Current checkpoint

Task 1, checkpoint 1 is complete: the user confirmed that `task1_window.cpp`
runs successfully.

The current checkpoint is Task 1, checkpoint 2: `task1_input.cpp` adds a prompt,
keyboard input, Backspace, and Enter. Enter saves and displays the latest
submitted text. Interactive confirmation is pending.

The input accepts printable ASCII characters, up to 80 characters per entry.
Later checkpoints will add tab buffers, command execution, and scrolling.

## Build and run

Run these commands in Ubuntu:

```bash
# Enter the folder containing the C++ checkpoints.
cd /mnt/f/Terminator/cpp

# Compile with C++17 and useful warnings, then link the X11 library.
g++ -std=c++17 -Wall -Wextra -Wpedantic task1_input.cpp -o task1_input -lX11

# Run the executable from the current folder.
./task1_input
```

To revisit the first checkpoint, substitute `task1_window.cpp` and
`task1_window` in the compile and run commands above.

## Checks for the current checkpoint

1. Type `abc`; it should appear beside the prompt.
2. Press Backspace once; the input should become `ab`.
3. Press Enter; `ab` should appear under "Last submitted text" and the input
   should become empty.
4. Try Backspace and Enter with empty input; the program should stay usable.
5. Minimise and restore the window; the saved text should remain visible.
6. Try typing beyond 80 characters; a limit message should appear. Backspace
   should make room to type another character.
7. Close the window; the program should return to the Ubuntu prompt.

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

The earlier C exercises remain in the parent folder for comparison.
