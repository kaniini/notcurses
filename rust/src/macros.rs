//! Macros

// General Utility Macros ------------------------------------------------------

/// Sleeps for $ms milliseconds.
#[macro_export]
macro_rules! sleep {
    ($ms:expr) => {
        std::thread::sleep(std::time::Duration::from_millis($ms));
    };
}

/// Renders the [Notcurses][crate::Notcurses] object sleeps for $ms milliseconds.
#[macro_export]
macro_rules! rsleep {
    ($nc:expr, $ms:expr) => {
        unsafe {
            crate::notcurses_render($nc);
        }
        std::thread::sleep(std::time::Duration::from_millis($ms));
    };
}

/// Converts `&str` to `*mut CString`, for when `*const c_char` is needed.
#[macro_export]
macro_rules! cstring {
    ($s:expr) => {
        std::ffi::CString::new($s).unwrap().as_ptr();
    };
}
