#ifndef CONSOLE_API_INTERNAL_COMMON_H
#define CONSOLE_API_INTERNAL_COMMON_H
/*
 * Provides linux&windows cross-platform
 * functions to manipulate the console window.
 */

/*
 * All functions that are different between
 * platforms have the format(or similar to):
 * ret_type f_name(params...)
 * {
 *      ret_type ret;
 *      // Common prologue
 * #if defined(_WIN32)
 *      // Windows code
 * #elif defined(__linux)
 *      // Linux code
 * #endif
 *      // Common epilogue
 *      return ret;
 * }
 *
 * Internal function which are specific
 * to a platform have the format:
 * #if defined(<platform-define>)
 * ret_type console_s_<platform>_f_name(params...) {...}
 * #endif
 *
 * where platform is either `win` or `linux`
 * and platform-define is either `_WIN32` or `__linux`
 */


/* Common includes */
#include <console_api.h>

#include <ctype.h>
#include <stdarg.h>
#include <limits.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

/* Platform specific includes & defines*/
#if defined(_WIN32)
    #include <windows.h>
#elif defined(__linux)
    #include <poll.h>
    #include <fcntl.h>
    #include <signal.h>
    #include <termios.h>

    #include <sys/stat.h>
    #include <sys/types.h>
    #include <sys/ioctl.h>
    #include <linux/input.h>

    #define DIR_DEV_INPUT_BY_PATH "/dev/input/by-path/"

#else
    #error "Unsupported platform. This game only supports Linux & Windows"
#endif

#if defined(_WIN32)
    /* Windows adds _ in front of all
       POSIX functions that it implements.
       In the source code, we do not use _
       Hence why we define them without _
       in Windows */
    #ifndef fileno
	    #define fileno _fileno
	#endif
	#ifndef isatty
	    #define isatty _isatty
	#endif
	
	/* Older versions of gcc do not define this macro, we define it if it's not there  */
	#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
		#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
	#endif
#endif

/* Console state struct */
struct CONSOLE_STATE;
typedef struct CONSOLE_STATE console_state;
struct CONSOLE_STATE
{
    /* Common fields */
    int init; /* Set to 1 when the structure is initialized */

    /* Console style state, never actually used */
    int style_fr, style_fg, style_fb; /* Foreground colors */
    int style_br, style_bg, style_bb; /* Background colors */
    int style_fb_switch; /* Switch foreground and background */
    int style_bold, style_dim, style_underline, style_blink;

    /* Platform specific fields */
#if defined(_WIN32)
    /* STDIN and STDOUT handles */
    HANDLE handle_stdout;
    HANDLE handle_stdin;

    CONSOLE_SCREEN_BUFFER_INFO org_buf_info;
    DWORD term_org_mode;
    DWORD term_g_mode;

    DWORD term_org_input_mode;
    DWORD term_txt_input_mode;

    int org_mode_set;
#elif defined(__linux)
    struct termios org_attr; /* Original terminal configuration */
    struct termios g_attr; /* Game terminal attributes */
    int org_attr_set; /* set to 1 when stdin_org_attr is valid */

    /* Buffer used by console_key_state for a bitmap of all key states */
    char kmap[KEY_MAX / 8 + 1];
#endif
};

extern console_state s_cstate;

#endif /* CONSOLE_API_INTERNAL_COMMON_H */
