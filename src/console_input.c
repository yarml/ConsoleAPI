#include "console_api.common.h"

// console_fgets & console_scanf share the same
// prologue and epilogue where the original
// terminal configuration is reset
// Non zero return code means error
static int s_console_input_prologue()
{
#if defined(_WIN32)
    // Discard all previous unprocessed input
    FlushConsoleInputBuffer(s_cstate.handle_stdin);

    GetConsoleMode(s_cstate.handle_stdin, &s_cstate.term_org_input_mode);

    // Disable mouse & window events while we get text input
    s_cstate.term_txt_input_mode =
        s_cstate.term_org_input_mode ^ ENABLE_MOUSE_INPUT ^ ENABLE_WINDOW_INPUT;
    SetConsoleMode(s_cstate.handle_stdin, s_cstate.term_txt_input_mode);
#elif defined(__linux)
    // Load the default original terminal configuration
    // this also discards everything in stdin that was not read
    // which is a behavior we want
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &s_cstate.org_attr) < 0)
        return -1;

    struct termios tmp;
    tcgetattr(STDIN_FILENO, &tmp);

    if(
       tmp.c_cflag != s_cstate.org_attr.c_cflag
    || tmp.c_iflag != s_cstate.org_attr.c_iflag
    || tmp.c_oflag != s_cstate.org_attr.c_oflag
    || tmp.c_lflag != s_cstate.org_attr.c_lflag
    || tmp.c_ispeed != s_cstate.org_attr.c_ispeed
    || tmp.c_ospeed != s_cstate.org_attr.c_ospeed
    || tmp.c_line != s_cstate.org_attr.c_line
    || memcmp(tmp.c_cc, s_cstate.org_attr.c_cc, sizeof(tmp.c_cc))
    )
    {
        return -1;
    }
#endif
    return 0;
}

static int s_console_input_epilogue()
{
#if defined(_WIN32)
    // Discard all data in stdin
    FlushConsoleInputBuffer(s_cstate.handle_stdin);

    SetConsoleMode(s_cstate.handle_stdin, s_cstate.term_org_input_mode);
#elif defined(__linux)
    // Load the game terminal configuration
    // also discard any data that was in stdin
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &s_cstate.g_attr) < 0)
        return -1;
#endif
}

int console_scanf(size_t timeout, char *format, ...)
{
    s_console_input_prologue();
    int timeout_respected = 1;

    /* The way we force a timeout depends on the OS */
    if(timeout)
    {
#if defined(_WIN32)
        /* In windows, we can use WaitForSingleObject,
           which, among other things, can be used to block
           until there is data to be read in STDIN.
        */
        DWORD result = WaitForSingleObject(s_cstate.handle_stdin, timeout);

        if(result == WAIT_FAILED)
            return -1;

        if(result == WAIT_TIMEOUT)
            timeout_respected = 0;
#elif defined(__linux)

        /* The poll system call can let us know
           which operations are ready to be done
           on a file while specifying a timeout
           We use this to see if reading can be
           done from STDIN with a timeout as
           specified by the user */
        struct pollfd pfd;

        pfd.fd = STDIN_FILENO;
        pfd.events = POLLIN;

        if(poll(&pfd, 1, timeout) < 0)
            return -1;

        if(
           pfd.revents & POLLERR
        || pfd.revents & POLLNVAL
        || pfd.revents & POLLHUP
        )
            return -1;

        // if after timeout stdin was still not ready
        // to give some data, then it means the user
        // didn't press enter yet
        if(!(pfd.revents & POLLIN))
            timeout_respected = 0;
#endif
    }

    if(timeout_respected)
    {
        // if stdin had data ready before timeout, we read that data
        va_list vargs;
        va_start(vargs, format);
        vscanf(format, vargs);
        va_end(vargs);
    }

    s_console_input_epilogue();

    /* The function returns non zero if timeout was not respected */
    return !timeout_respected;
}

char *console_fgets(char *s, int len)
{
    if(s_console_input_prologue())
        return 0;
    scanf(" "); // Skip all leading whitespace
    char *rs = fgets(s, len, stdin);

    // Remove all trailing whitespace
    size_t in_len = strlen(rs);
    for(size_t i = in_len - 1; i < in_len; --i)
        if(!isspace(rs[i]))
        {
            /* When we find first non space character
               we put the null termination right after it */
            rs[i + 1] = 0;
            break;
        }

    if(s_console_input_epilogue())
        return 0;
    return rs;
}
