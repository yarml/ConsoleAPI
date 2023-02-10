#include "console_api.common.h"

void console_clear()
{
#if defined(_WIN32)
    // Windows tries to mimic Linux's console codes
    // in what it calls virtual terminal sequences
    // To clear the window, it is very similar to linux
    // except that windows does not support \e[3J
    // to clear the scroll, so we only clear the current screen
    // and hope the user does not scroll up or down
    // We mitigate this by disabling scroll entirely
    // during console_init
    printf("\e[1;1H\e[2J");
#elif defined(__linux)
    // Escape code meaning
    // \e[x;yH  moves the cursor to x,y(origin is 1,1)
    // \e[3J    clear the terminal scroll
    // \e[2J    clear the terminal screen
    printf("\e[1;1H\e[3J\e[2J");
#endif
}
