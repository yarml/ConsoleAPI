#include "console_api.common.h"

int console_init()
{
    /* If the user is using file redirection,
       this is a game, we do not want that */
    if(!isatty(fileno(stdin)) || !isatty(fileno(stdout)))
        return CONSOLE_INIT_MCAP_NTTY;

    /* 0 initialize cstate, regardless of platform */
    memset(&s_cstate, 0, sizeof(s_cstate));

    /* Do not use buffering in the game
       Because by default, unless \n was
       written to stdout, the output is not
       guaranteed to be flushed, and the user may not
       see anything */
    setbuf(stdout, 0);

    s_cstate.init = 1;

    atexit((void(*)()) console_cleanup);
#if defined(_WIN32)
    /*
     * Windows initialization:
     *   - Get the STDIN and STDOUT handles
     *   - Save initial terminal configuration
     *   - Modify terminal configuration as follows:
     *      -- Enable Virtual Console escape codes
     *   - Disable Terminal scrolling
     */

    // get STDIN and STDOUT handles
    s_cstate.handle_stdin = GetStdHandle(STD_INPUT_HANDLE);
    s_cstate.handle_stdout = GetStdHandle(STD_OUTPUT_HANDLE);

    if(
       s_cstate.handle_stdin == INVALID_HANDLE_VALUE
    || s_cstate.handle_stdout == INVALID_HANDLE_VALUE
    )
        return CONSOLE_INIT_ERR;

    if(!GetConsoleMode(s_cstate.handle_stdout, &s_cstate.term_org_mode))
        return CONSOLE_INIT_ERR;

    s_cstate.org_mode_set = 1;

    s_cstate.term_g_mode = s_cstate.term_org_mode;

    // Enable Virtual Console escape codes
    s_cstate.term_g_mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

    // Set the new terminal mode
    if (!SetConsoleMode(s_cstate.handle_stdout, s_cstate.term_g_mode))
        return CONSOLE_INIT_ERR;


    // Disabling terminal scrolling
    CONSOLE_SCREEN_BUFFER_INFO bufinf;
    GetConsoleScreenBufferInfo(s_cstate.handle_stdout, &bufinf);

    // Save original buffer info to restore it when program exits
    s_cstate.org_buf_info = bufinf;

    SHORT win_h = bufinf.srWindow.Bottom - bufinf.srWindow.Top + 1;
    SHORT buf_w = bufinf.dwSize.X;

    // Disable scrolling by making the buffer as small as the window
    COORD new_buf_size;
    new_buf_size.X = buf_w;
    new_buf_size.Y = win_h;

    // set the new screen buffer dimensions
    SetConsoleScreenBufferSize(s_cstate.handle_stdout, new_buf_size);

    HWND consoleWindow = GetConsoleWindow();
    SetWindowLong(consoleWindow, GWL_STYLE, GetWindowLong(consoleWindow, GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);

    console_clear();

    return CONSOLE_INIT_SUCCESS;
#elif defined(__linux)
    /*
     * Linux initialization:
     *   - Save the initial terminal configuration
     *   - Modify the terminal configuration such that:
     *      -- Characters do not get echoed when keys are pressed
     *      -- We do not wait for carriage return before sending input
     *      -- A lot of other smaller modifications are done
     *      -- Ctrl-C does not terminate the program
     */


    // Save the original terminal configuration
    if(tcgetattr(STDIN_FILENO, &s_cstate.org_attr) < 0)
        return CONSOLE_INIT_ERR;

    s_cstate.org_attr_set = 1;

    // Copy the original configuration
    s_cstate.g_attr = s_cstate.org_attr;

    // Configure the new terminal configuration
    cfmakeraw(&s_cstate.g_attr);

    // cfmakeraw disables other useful terminal features
    // for example it  disables output processing
    // This prevents escape sequences from being
    // processed, \n doesn't translate to \r\n, etc
    // to prevent these issues, we enable
    // output processing again
    s_cstate.g_attr.c_oflag |= OPOST;

    s_cstate.g_attr.c_lflag |= IEXTEN;

    // Use the new terminal configuration
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &s_cstate.g_attr) < 0)
        return CONSOLE_INIT_ERR;

    /* The documentation for tcsetattr says
       that to really check for errors, tcgetattr
       should be called again and compared to
       the desired value */
    struct termios tmp;
    if(tcgetattr(STDIN_FILENO, &tmp) < 0)
    {
        /* In case of error, we reset the original configuration and leave */
        tcsetattr(STDIN_FILENO, TCSANOW, &s_cstate.org_attr);
        return CONSOLE_INIT_ERR;
    }

    if(
       tmp.c_cflag != s_cstate.g_attr.c_cflag
    || tmp.c_iflag != s_cstate.g_attr.c_iflag
    || tmp.c_oflag != s_cstate.g_attr.c_oflag
    || tmp.c_lflag != s_cstate.g_attr.c_lflag
    || tmp.c_ispeed != s_cstate.g_attr.c_ispeed
    || tmp.c_ospeed != s_cstate.g_attr.c_ospeed
    || tmp.c_line != s_cstate.g_attr.c_line
    || memcmp(tmp.c_cc, s_cstate.g_attr.c_cc, sizeof(tmp.c_cc))
    )
    {
        /* In case of error, we reset the original configuration and leave */
        tcsetattr(STDIN_FILENO, TCSANOW, &s_cstate.org_attr);
        return CONSOLE_INIT_ERR;
    }

    printf("\e[;r");

    console_clear();

    return CONSOLE_INIT_SUCCESS;
#endif
}
