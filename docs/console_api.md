# Console API
Console API is a multi-platform library to manipulate and access the state of
the console. This documentation provides a list of all the capabilities that this
API provides. All these capabilities should be guaranteed to work in both
Desktop Linux and Windows.

# Table of content
- [Console API](#console-api)
- [Table of content](#table-of-content)
- [Usage](#usage)
- [Capabilities](#capabilities)
  - [Clear Screen](#clear-screen)
  - [Get the state of a keyboard key](#get-the-state-of-a-keyboard-key)
  - [Wait for keyboard key press and release](#wait-for-keyboard-key-press-and-release)
  - [Text user input](#text-user-input)
  - [Menu](#menu)
  - [Styled output](#styled-output)
  - [Console Title](#console-title)
- [Implementation details](#implementation-details)
  - [Common](#common)
    - [Text styling](#text-styling)
  - [Linux](#linux)
    - [Terminal Setup](#terminal-setup)
    - [Keyboard Key state](#keyboard-key-state)
    - [Timed input](#timed-input)
  - [Windows](#windows)
    - [Terminal setup](#terminal-setup-1)
    - [Keyboard Key state](#keyboard-key-state-1)
    - [Timed input](#timed-input-1)
- [Known Issues](#known-issues)
  - [CONAPI\_LINUX\_ISSUE\_01](#conapi_linux_issue_01)
  - [CONAPI\_WIN\_ISSUE\_02](#conapi_win_issue_02)
  - [CONAPI\_WIN\_ISSUE\_03](#conapi_win_issue_03)


# Usage
The API first expects to be initialized using `console_init()`, in this step,
the API will initialize all its internal states depending on the platform.

When the game is done, the API requires to call a cleanup function so that the
API can have a chance to properly release any state that needs releasing,
and restore the terminal to its original configuration. This is done
using `console_cleanup()`.

# Capabilities
## Clear Screen
The API provides `console_clear()` to clear and reset the position of the
cursor in the terminal window.

## Get the state of a keyboard key
The API provides `console_key_state(key)` to get the Pressed/Released state
of a keyboard key. Implementing this on Linux was a nightmare.
Read [Linux/Keyboard Key state](#keyboard-key-state)

## Wait for keyboard key press and release
The API provides `console_wait_clicks(keys[], kcount)` which blocks until one of
the keys in `keys` is pressed then released. It returns which key was pressed.

The API also provides `console_wait_click(key)` which does the same but with
only 1 key.

## Text user input
When using the console API, it is discouraged(Would not even work) to use
standard C library's input function with `stdin`. This is because the API
int the initialization phase tampers with the terminal configuration in ways
that make all user input invisible, and also makes the ENTER key have no effect
on the input buffer which can lead to weird behaviors. The API however
provides 2 functions which are similar to standard C library functions that will
revert to the default configuration before asking the user for input.

`console_scanf(timeout, fmt, ...)` works just like standard C's `scanf`, with
the exception that if `timeout` is not `0`, then the user is expected to have
their input ready in `timeout` milliseconds. Otherwise, all of the input is
discarded. This function, unlike `scanf`, returns `0` is the timeout was
respected(including if the timeout was `0`, ie. disabled), otherwise, it
returns a non-zero value.

`console_fgets(s, len)` works exactly like standard C's `fgets`, with a few
exceptions:
- It always uses `stdin`
- It removes all leading and trailing whitespace in the string

## Menu
The API defines a way to display menus that are navigable using keyboard
arrows.

This is achieved by passing a pointer to an array of `struct menu_ent`.

The structure is defined as follows:
```yaml
menu_ent:
    string: ent_name
    int: ent_val
    string: ent_detail
    int: disabled
```

This array pointer is passed to `console_menu(prompt, menu_ent[], ent_count)`
which does the following:
- Display the prompt.
- Display the menu.
- Use keyboard arrows to navigate the menu.
- Display details for the selected entry.
- Grey out, and disable entries that have `disabled` set to non-zero
- The value specified by `ent_val` is returned by `console_menu`

An example of the usage of the menu:
```c
menu_ent entries[3]=
{
  { "Menu entry 1", 0, "Detail about entry 1", 0 }, // Not disabled
  { "Menu entry 1", 1, "Detail about entry 1", 0 }, // Not disabled
  { "Menu entry 1", 2, "Detail about entry 1", 1 }  // Disabled
};

size_t val = console_menu("Choose an option", entries, 3);

switch(val)
{
    case 0: /* Menu entry 1 */
        /* ... */
        break;
    /* ... */
}
```

## Styled output
Console API exposes `console_color_foreground(r, g, b)`,
`console_color_background(r, g, b)` , `console_color_switch(flag)`,
`console_bold(flag)`, `console_dim(flag)`, `console_blink(flag)`,
`console_underline(flag)` and `console_style_reset()`.

---
Not all these flags are equally supported across platforms. On Windows for
example, `blink` is not supported. And if a Linux terminal is very(very) old,
it may also not support some or all those flags. These functions should only be
considered as hints, and the underlying terminal can completely ignore them.

## Console Title
The API eposes `console_title(title)` to set the terminal's title.

---
This is also not guaranteed to work on all terminals. For example, terminals
embedded in IDEs might not have a title to begin with, or a terminal can be
outdated that it does not support this feature. It is also worthy to note
that the method used to set the terminal title is not part of the Linux
specification of a terminal, but an xterm extension to the said specification.
However most modern terminals follow the xterm specification as well, including
Windows terminals.

# Implementation details
## Common
### Text styling
To style the output, we use escape sequences(Supported by Linux, can be enabled
in windows). These escaped sequences are both documented in Linux manual
and Windows documentation. It should be noted that Linux manual discourages
the direct use of escape sequences, while Windows documentation says they are
the recommended method to style output. On modern Linux terminals, the
Linux manual warning can safely be ignored.

- https://man7.org/linux/man-pages/man4/console_codes.4.html
- https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences

## Linux
### Terminal Setup
By default in Linux, terminals are setup in the canonical mode; that is, input
is not directly sent to the application until carriage return is hit, and also
all keyboard keys that are pressed are immediately displayed in the screen. This
default configuration is not ideal, thankfully, linux offers a number of
system calls to override this default behavior. These system calls have
wrapper C functions defined in `termios.h` and `sys/ioctl.h`.

- https://stackoverflow.com/a/59923166
- https://man7.org/linux/man-pages/man3/termios.3.html
- https://man7.org/linux/man-pages/man2/ioctl.2.html
- https://man7.org/linux/man-pages/man2/ioctl_tty.2.html

### Keyboard Key state
The terminal interface defined with `termios.h`, and even the low level
`sys/ioctl.h` provide no way to get the currently pressed keyboard keys.
As such, another (way more complicated) method was used, and that is by
using Linux's Input API. This method also consists of using `ioctl`, but
not on terminal files, but rather on keyboard event files.

Compared to Windows, the method of getting a keyboard key state in Linux is much
more involved. We start by guessing which event files belong to connected
keyboards, then check the state of each keyboard to see if the requested key
was pressed. As soon as we find a press the functions returns a `PRESSED`
status, otherwise, if no keyboard reposrts the key being pressed, the function
then returns `RELEASED`.

- https://docs.kernel.org/input/input.html

### Timed input
Linux exposes a system call; `poll` which blocks the program until a file can
do a certain action. In this instance, we tell `poll` to wait until `stdin` can
be read from. `poll` also supports setting a timeout so that if the file is not
ready for the action after a specified amount of milliseconds, it will let the
program continue execution and will return a code to signal the timeout.

Before letting the user enter input, we first need to restore the original
configuration so that button presses would be visible to the user, and then
flush all the data that was in `stdin`, because, even when button presses
are not visible, `stdin` still registers them.

- https://man7.org/linux/man-pages/man2/poll.2.html
- https://softwareengineering.stackexchange.com/a/190246 ; This post uses
  `select` but the manual page for `select` recommends using `poll` instead.
- https://man7.org/linux/man-pages/man2/select.2.html

## Windows
### Terminal setup
In windows, the setup process consists of getting handles to STDIN and STDOUT,
then changing the terminal configuration to allow escape codes sequences in the
output(This makes windows emulate a Linux terminal). Then we disable scrolling
in the terminal window, so that when it is cleared, the user cannot scroll up
and see cleared content.

- https://learn.microsoft.com/en-us/windows/console/getstdhandle
- https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences
- https://learn.microsoft.com/en-us/windows/console/setconsolemode
- https://learn.microsoft.com/en-us/windows/console/window-and-screen-buffer-size
- https://learn.microsoft.com/en-us/windows/console/setconsolescreenbuffersize

### Keyboard Key state
Windows APIs expose `GetKeyState` which returns a `SHORT` whose highest
bit is set to `1` if and only if the key is pressed.

- https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getkeystate

### Timed input
Windows API exposes `WaitForSingleObject`, which is very similar to Linux's
`poll`. It blocks until there is data to be read from `stdin`(That is,
the user pressed ENTER), and allows to set a timeout so that if not input
is available before that time, the function will just return and signal a
timeout.

- https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitforsingleobject


# Known Issues
## CONAPI_LINUX_ISSUE_01
Depending on the configuration of the Linux system of the user, getting the
state of keyboard keys can require elevated permissions. There is no workaround
this issue other than to tell the user to change his keyboard event file
read permission to allow unprivileged users to access the state of keyboard
keys, or to require the user to run the game using elevated privileges, which
is the worse solution one can propose.

---
On most desktop Linux distributions, the permissions of the keyboard event files
should allow the program to run without problem.

## CONAPI_WIN_ISSUE_02
The buffer of the console screen stays small even after program ends. This means
the user cannot scroll up even after leaving the program.

## CONAPI_WIN_ISSUE_03
Apparently, the timed input does not work for some reason in Windows. Too lazy
to figure it out now.
