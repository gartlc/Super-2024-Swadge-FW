## ADDING A MODE

1. Make sure your header file has an external `swadgeMode_t`:

```C
#include "swadge2024.h"
extern swadgeMode_t visualizerMode;
```

2. Update `main/modeIncludeList.h`

    Put the header of your mode as one of the includes in this file. For example:
```C
    #include "visualizer.h"
```

3. Update `main/modeIncludeList.c`

    Add your swadgeMode_t pointer to the other list of swadgeMode_t pointers. This adds it to the menu in your Swadge.

```C
swadgeMode_t* const allSwadgeModes[]
    = {&visualizerMode};
```

Also add a menu item with your visualizer mode:

```C
    // Music sub menu
    menu = startSubMenu(menu, "Music");
    addSingleItemToMenu(menu, visualizerMode.modeName);
    menu = endSubMenu(menu);
```

4. Update `main/CMakeLists.txt` in two locations (if you are adding to the file structure):

```cmake
idf_component_register(
    SRCS "modes/music/visualizer/visualizer.c"
    INCLUDE_DIRS "./modes/music/visualizer"
)
```

This ensures that `idf.py build` can find the files for your new mode.

## BUILDING

1. Run export.ps1 from esp in PowerShell:

```bash
~/esp/esp-idf/export.ps1
```

2. cd to swadge directory:

`cd C:/Users/chris/esp/Super-2024-Swadge-FW`

3. Run idf.py (after export):

`idf.py build`

4. Make (USE MSYS2 SHELL):

`cd C:/Users/chris/esp/Super-2024-Swadge-FW`
`make clean all`