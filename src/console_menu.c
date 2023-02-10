#include "console_api.common.h"

size_t console_menu(char *prompt, menu_ent *entries, size_t entries_count)
{
    /* Displaying the menu is a loop of:
        - Clearing display
        - Displaying prompt
        - Drawing menu
        - waiting & acting on user input */
    if(!entries_count)
        return 0;
    console_style_reset();

    size_t sel_idx = 0;

    while(1)
    {
        console_clear();

        console_bold(1);
        console_color_foreground(242, 140, 40);
        puts(prompt);
        console_style_reset();

        for(size_t i = 0; i < entries_count; ++i)
        {
            /* The item currently selected is displayed differently */
            if(entries[i].disabled)
                console_dim(1);
            if(i == sel_idx)
            {
                console_color_switch(1);
                printf(
                    " [ %s ] %s\n",
                    entries[i].ent_name,
                    entries[i].ent_detail
                );
                console_color_switch(0);
            }
            else
                printf("   %s\n", entries[i].ent_name);
            console_dim(0);
        }

        int wait_keys[] =
            { CONSOLE_KEY_DOWN, CONSOLE_KEY_UP, CONSOLE_KEY_ENTER };

        int key = console_wait_clicks(wait_keys, 3);

        switch(key)
        {
            case CONSOLE_KEY_UP:
                if(sel_idx == 0)
                    sel_idx = entries_count - 1;
                else
                    --sel_idx;
                break;
            case CONSOLE_KEY_DOWN:
                ++sel_idx;
                break;
            case CONSOLE_KEY_ENTER:
                if(!entries[sel_idx].disabled)
                {
                    console_clear();
                    return entries[sel_idx].ent_val;
                }
                /* If this entry was disabled, just ignore ENTER */
                break;
        }
        // if we were on the first/last entry, we loop back;
        sel_idx %= entries_count;
    }
}
