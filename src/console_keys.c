#include "console_api.common.h"

int console_key_state(int key)
{
    // The code size difference between Linux and Windows
    // for this function is almost funny
#if defined(_WIN32)
    return GetKeyState(key) & 0x8000;
#elif defined(__linux)
    /*
     * In Linux, getting the state of a keyboard key is
     * a very involved process. Linux does not expose
     * any GetKeyState function that will check if a key
     * is pressed on any keyboard, instead, we need to
     * manually check if any keyboard has `key` pressed.
     * The steps to implement this are:
     *      - Detect connected keyboards
     *        This can be done by searching the directory
     *        /dev/input/by-path for keyboard control files
     *        ending with `kbd`
     *      - For each keyboard detected, check if `key` is pressed
     *      - If no keyboard reports `key` being pressed, then we return 0
     */

    // Step one, iterate over all files in /dev/input/by-path/
    DIR *evdir = opendir(DIR_DEV_INPUT_BY_PATH);

    if(!evdir)
        return -1;

    struct dirent *ent;
    while(1)
    {
        ent = readdir(evdir);

        if(!ent) // We ran out of keyboards to check and none was pressed
        {
            closedir(evdir);
            return 0;
        }

        // For each file in /dev/input/by-path, we check
        //   - it is either a link or char device
        //   - it ends with `kbd`
        //   - if it is a link, check that it is pointing to a char device

        // check that file is link or char device
        if(ent->d_type != DT_LNK && ent->d_type != DT_CHR)
            continue;

        // check that filename ends with `kbd`
        size_t ent_name_len = strlen(ent->d_name);
        if(
           ent_name_len < 3
        || strcmp(ent->d_name + ent_name_len - 3, "kbd")
        )
            continue;

        // open the file, we use Linux system call directly
        // This is mostly because it is painful to create
        // the string for the absolute path by hand
        // instead we use the syscall openat, which
        // takes a directory file number and the relative path
        // of the file to that directory.

        // the file is open in read only mode
        int fkbd = openat(dirfd(evdir), ent->d_name, O_RDONLY);

        if(fkbd < 0)
        {
            // if there was an error opening the file
            // simply skip to the next file
            continue;
        }

        // if the file is a link
        if(ent->d_type == DT_LNK)
        {
            // check that file is a link to a char dev
            struct stat file_info;
            fstat(fkbd, &file_info);

            // fstat returns information about the file
            // if the file was a link, it will resolve the link
            // before and return information about the real file

            if((file_info.st_mode & S_IFMT) != S_IFCHR)
            {
                // this file is not a character device file,
                // as a result, we close and skip to the next device
                close(fkbd);
                continue;
            }
        }

        // Here we are sure that the file
        // is a char dev that is *probably* a keyboard

        // we use ioctl with EVIOCGKEY
        // https://stackoverflow.com/a/4225290
        int ioctl_res =
            ioctl(fkbd, EVIOCGKEY(sizeof(s_cstate.kmap)), s_cstate.kmap);

        // close device
        close(fkbd);

        if(ioctl_res < 0)
        {
            // if we had an error, it probably means that this device was not
            // a keyboard after all, we lost some time opening
            // and ioctl-ing it, but it's not a problem just go
            // to next device
            continue;
        }

        if(s_cstate.kmap[key/8] & (1 << key % 8))
            return 1;
    }
#endif
}


void console_wait_click(int key)
{
    // if key was pressed before call, wait for it to be released first
    if(console_key_state(key))
        while(console_key_state(key));

    /* Wait for the key to be pressed */
    while(!console_key_state(key));

    /* Wait for the key that was pressed to be released */
    while(console_key_state(key));
}

int console_wait_clicks(int *keys, size_t kcount)
{
   int key = -1;

   /* If any of keys was pressed before the function call
      Wait for it to be released first */
    for(size_t i = 0; i < kcount; ++i)
        if(console_key_state(keys[i]))
            while(console_key_state(keys[i]));

    /* Wait for any of keys to be pressed */
    while(key == -1)
    {
        for(size_t i = 0; i < kcount; ++i)
            if(console_key_state(keys[i]))
            {
                /* Found a key that was pressed */
                key = keys[i];
                break;
            }
    }

    /* Wait for the key that was pressed to be released */
    while(!console_key_state(key));

   return key;
}
