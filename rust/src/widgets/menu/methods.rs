//! `NcMenu*` methods and associated functions.

use crate::{
    cstring, ncmenu_create, NcChannelPair, NcInput, NcMenu, NcMenuItem, NcMenuOptions,
    NcMenuSection, NcPlane,
};

/// # `NcMenu` Constructors
impl NcMenu {
    /// `NcMenu` simple constructor
    pub unsafe fn new<'a>(plane: &mut NcPlane) -> &'a mut Self {
        Self::with_options(plane, &NcMenuOptions::new())
    }

    /// `NcMenu` constructor with options
    pub unsafe fn with_options<'a>(plane: &mut NcPlane, options: &NcMenuOptions) -> &'a mut Self {
        &mut *ncmenu_create(plane, options)
    }
}

/// # `NcMenuOptions` Constructors
impl NcMenuOptions {
    /// `NcMenuOptions` simple constructor
    pub fn new() -> Self {
        Self::with_options(&mut [], 0, 0, 0, 0)
    }

    /// `NcMenuOptions` width options
    pub fn with_options(
        sections: &mut [NcMenuSection],
        count: u32,
        headerc: NcChannelPair,
        sectionc: NcChannelPair,
        flags: u64,
    ) -> Self {
        Self {
            // array of 'sectioncount' `MenuSection`s
            sections: sections as *mut _ as *mut NcMenuSection, /// XXX TEST

            // must be positive TODO
            sectioncount: count as i32,

            // styling for header
            headerchannels: headerc,

            // styling for sections
            sectionchannels: sectionc,

            // flag word of NCMENU_OPTION_*
            flags: flags,
        }
    }
}

/// # `NcMenuItem` Constructors
impl NcMenuItem {
    /// `NcMenuItem` simple constructor
    pub fn new(mut desc: i8, shortcut: NcInput) -> Self {
        Self {
            // utf-8 menu item, NULL for horizontal separator
            desc: &mut desc,

            // ´NcInput´ shortcut, all should be distinct
            shortcut,
        }
    }
}

/// # `NcMenuSection` Constructors
impl NcMenuSection {
    /// `NcMenuSection` simple constructor
    pub fn new(name: &str, itemcount: i32, items: &mut [NcMenuItem], shortcut: NcInput) -> Self {
        Self {
            // utf-8 name string
            name: cstring![name] as *mut i8,

            //
            itemcount,

            // array of itemcount `NcMenuItem`s
            items: items as *mut _ as *mut NcMenuItem,

            // shortcut, will be underlined if present in name
            shortcut,
        }
    }
}
