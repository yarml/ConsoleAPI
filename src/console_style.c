#include "console_api.common.h"
#include <console_api.h>

/* This is a number of magic values that are used by
   Linux and Windows terminals to set text style */
#define ENABLE_TERM_GFX_ATTR(n) printf("\e[%dm", n)

#define TERM_GFX_RESET      (0)

#define TERM_GFX_BOLD       (1)
#define TERM_GFX_DIM        (2) // Not supported on Windows & bold
#define TERM_GFX_UNDERLINE  (4)
#define TERM_GFX_BLINK      (5) // Not supported on Windows
#define TERM_GFX_REV_VID    (7)

#define TERM_GFX_NDIM       (22) // 22 is to reset both bold and dim
#define TERM_GFX_NBOLD      (22) // 22's actual name is normal intensity
#define TERM_GFX_NUNDERLINE (24)
#define TERM_GFX_NBLINK     (25) // Not supported on Windows
#define TERM_GFX_NREV_VID   (27)


void console_color_foreground(int r, int g, int b)
{
    s_cstate.style_fr = r;
    s_cstate.style_fg = g;
    s_cstate.style_fb = b;
    // Escape code meaning
    // \e
    // 38: set foreground color
    // 2: use 24-bit color
    // m: terminate terminal style command
    // Read docs/console_api.md#terminal-styling for more details
    // Windows mimics this behavior as well when
    // Virtual terminal sequences are enabled
    printf("\e[38;2;%d;%d;%dm", r, g, b);
}
void console_color_background(int r, int g, int b)
{
    s_cstate.style_br = r;
    s_cstate.style_bg = g;
    s_cstate.style_bb = b;
    // Escape code meaning
    // \e
    // 48: set background color
    // 2: use 24-bit color
    // m: terminate terminal style command
    // Read docs/console_api.md#terminal-styling for more details
    // Windows mimics this behavior as well when
    // Virtual terminal sequences are enabled
    printf("\e[48;2;%d;%d;%dm", r, g, b);
}
void console_color_switch(int flag)
{
    s_cstate.style_fb_switch = flag;
    if(flag)
        ENABLE_TERM_GFX_ATTR(TERM_GFX_REV_VID);
    else
        ENABLE_TERM_GFX_ATTR(TERM_GFX_NREV_VID);
}

void console_bold(int flag)
{
    s_cstate.style_bold = flag;
    // bold implies not dim
    if(flag)
        console_dim(0);
    if(flag)
        ENABLE_TERM_GFX_ATTR(TERM_GFX_BOLD);
    else
        ENABLE_TERM_GFX_ATTR(TERM_GFX_NBOLD);
}
void console_dim(int flag)
{
    s_cstate.style_dim = flag;
    // dim implies not bold
    if(flag)
        console_bold(0);
    // windows does not support dim,
    // instead we set colors to half
    // their values
#if defined(_WIN32)
    if(flag && !s_cstate.style_dim)
    {
        console_color_foreground(
            s_cstate.style_fr / 2,
            s_cstate.style_fg / 2,
            s_cstate.style_fb / 2
        );
        console_color_background(
            s_cstate.style_br / 2,
            s_cstate.style_bg / 2,
            s_cstate.style_bb / 2
        );
    }
    else if(!flag && s_cstate.style_dim)
    {
        console_color_foreground(
            s_cstate.style_fr * 2,
            s_cstate.style_fg * 2,
            s_cstate.style_fb * 2
        );
        console_color_background(
            s_cstate.style_br * 2,
            s_cstate.style_bg * 2,
            s_cstate.style_bb * 2
        );
    }
#elif defined(__linux)
    if(flag)
        ENABLE_TERM_GFX_ATTR(TERM_GFX_DIM);
    else
        ENABLE_TERM_GFX_ATTR(TERM_GFX_NDIM);
#endif
}
void console_blink(int flag)
{
    s_cstate.style_blink = flag;
    // This is not supported by windows
#if defined(__linux)
    if(flag)
        ENABLE_TERM_GFX_ATTR(TERM_GFX_BLINK);
    else
        ENABLE_TERM_GFX_ATTR(TERM_GFX_NBLINK);
#endif
}
void console_underline(int flag)
{
    s_cstate.style_underline = flag;
    if(flag)
        ENABLE_TERM_GFX_ATTR(TERM_GFX_UNDERLINE);
    else
        ENABLE_TERM_GFX_ATTR(TERM_GFX_NUNDERLINE);
}
void console_style_reset()
{
    // FIXME: We just assume these are the defaults
    // While some terminal emulators on mights
    // have different defaults

    s_cstate.style_fr = 255;
    s_cstate.style_fg = 255;
    s_cstate.style_fb = 255;

    s_cstate.style_br = 0;
    s_cstate.style_bg = 0;
    s_cstate.style_bb = 0;

    s_cstate.style_fb_switch = 0;

    s_cstate.style_bold = 0;
    s_cstate.style_underline = 0;
    s_cstate.style_blink = 0;

    ENABLE_TERM_GFX_ATTR(TERM_GFX_RESET);
}


void console_title(char const *title)
{
    // Escape code meaning
    // \e]2;txt;\e\\ sets the terminal window title to txt
    // Assuming the terminal supports the operation
    // This is not part of the Linux terminal specification
    // But part of xterm specification
    // Which most linux terminals abide by
    // If it does not work, it is not a problem really
    // Windows mimics the behavior
    printf("\e]2;%s\e\\", title);
}

