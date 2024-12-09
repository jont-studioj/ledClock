#ifndef UIMenuItem_H
#define UIMenuItem_H

#include "UiMenu.h"

class UiMenu;
//enum MenuItemIdentEnum;

struct UiMenuItem {
    uint8_t itemIdx;
    const char *caption;
    bool selected;                      // the selected option if within an optionGroup (as if a radio button)
                                        // or just plain selected or not (as if a checkbox)
    int8_t optionGroup;                 // <>0 option group of item
    int itemIdent;                      // identifier of the item, can take an enum if needs
    UiMenu *childMenu;                  // pointer to item's child menu (if it needs one)
};

static struct UiMenuItem itemEntryNULL = {
    .caption = "",
    .selected = false,
    .optionGroup = 0,
    .itemIdent = 0,
    .childMenu = NULL
};

#endif // UIMenuItem_H