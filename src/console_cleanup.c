#include "console_api.common.h"

int console_cleanup()
{
    if(s_cstate.init)
    {
        s_cstate.init = 0;
        console_style_reset();
        int status = CONSOLE_CLEANUP_SUCCESS;
#if defined(_WIN32)
        // Reset the original terminal configuration
        if(!SetConsoleMode(s_cstate.handle_stdout, s_cstate.term_org_mode))
            /* In case of error we set the warn return flag */
            status |= CONSOLE_CLEANUP_WARN;

        SetConsoleScreenBufferSize(
            s_cstate.handle_stdout,
            s_cstate.org_buf_info.dwSize
        );

        // Flush all unprocessed input
        // so that it wouldn't be processed by
        // cmd after the program exits
        FlushConsoleInputBuffer(s_cstate.handle_stdin);
#elif defined(__linux)
        // Revert the original terminal config
        if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &s_cstate.org_attr) < 0)
            /* In case of error we set the warn return flag */
            status |= CONSOLE_CLEANUP_WARN;
#endif
        return status;
    }

    /* In case console is already clean */
    return 0;
}
