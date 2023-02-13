#ifndef CONSOLE_API_H
#define CONSOLE_API_H

#include <stddef.h>

/*
 * Details about the Console API can be found
 * in docs/console_api.h
 */

struct MENU_ENT;
typedef struct MENU_ENT menu_ent;
struct MENU_ENT
{
    char *ent_name;
    size_t ent_val;
    char *ent_detail;
    int disabled;
};


#define CONSOLE_INIT_SUCCESS     (0)
#define CONSOLE_INIT_MCAP        (1)
#define CONSOLE_INIT_ERR         (2)
#define CONSOLE_INIT_MCAP_NO_KBD (3)
#define CONSOLE_INIT_MCAP_NTTY   (4)
/*
 * Return values:
 *   SUCCESS: Everything okay
 *   MCAP: missing capabilities from terminal(Unspecified)
 *   ERR: Error during initialization
 *   MCAP_NO_KBD: No keyboard was found
 *   MCAP_NTTY: stdin or stdout is not a terminal device
 */
int console_init();

#define CONSOLE_CLEANUP_SUCCESS (0)
#define CONSOLE_CLEANUP_WARN    (1)
/*
 * Return values:
 *   SUCCESS: Cleanup was done okay
 *   WARN: The terminal couldn't be reverted
 *         to its original state, and may stay
 *         messed up even after the program
 *         terminates. A warning should be
 *         displayed to the user by the calling
 *         function.
 */
int console_cleanup();

void console_clear();

/*
 * Returns 1 if the key is pressed
 * Returns 0 if the key is released
 *
 * key should be one of CONSOLE_KEY_* defined below
 */
int console_key_state(int key);

void console_wait_click(int key);
/* Returns which key was pressed */
int console_wait_clicks(int *keys, size_t kcount);

/*
 * Returns 0 if timout(in millis) was respected, 1 otherwise
 * If timout=0, always returns 0 as if timeout was always respected
 */
int console_scanf(size_t timeout, char *format, ...);

char *console_fgets(char *s, int len);

size_t console_menu(char *prompt, menu_ent *entries, size_t entries_count);


/* CONSOLE_KEY_* */
/* These definitions depend on the plateform */
#if defined(_WIN32)
    #include <windows.h>

    /* Very hacky, I don't like this */
    #define CONSOLE_KEY_ALNUM(c) #c[0]

    #define CONSOLE_KEY_UP      VK_UP
    #define CONSOLE_KEY_DOWN    VK_DOWN
    #define CONSOLE_KEY_RIGHT   VK_RIGHT
    #define CONSOLE_KEY_LEFT    VK_LEFT
    #define CONSOLE_KEY_ENTER   VK_RETURN
#elif defined(__linux)
    #include <linux/input.h>

    #define CONSOLE_KEY_ALNUM(c) KEY_##c

    #define CONSOLE_KEY_UP      KEY_UP
    #define CONSOLE_KEY_DOWN    KEY_DOWN
    #define CONSOLE_KEY_RIGHT   KEY_RIGHT
    #define CONSOLE_KEY_LEFT    KEY_LEFT
    #define CONSOLE_KEY_ENTER   KEY_ENTER
#endif

/* Notes */
/* Other than alpha numeric keys, Arrow keys, and Enter
   Console API does not define other keys, as they are not
   needed by the game */
/* CONSOLE_KEY_ALNUM should be passed a number 0-9 or
   an upper case character A-Z, WITHOUT using '' */

/* Styled output */

void console_color_foreground_reset();
void console_color_background_reset();
void console_color_foreground(int r, int g, int b);
void console_color_background(int r, int g, int b);
void console_color_switch(int flag); //switch foreground and background colors
void console_bold(int flag);
void console_dim(int flag);
void console_blink(int flag);
void console_underline(int flag);
void console_style_reset();

void console_title(char const *title);

#endif