//! Test `NcCell` methods and associated functions.

use crate::NcCell;

use serial_test::serial;

#[test]
#[serial]
fn constructors() {
    let _c1 = NcCell::new();

    let _c2 = NcCell::with_7bitchar('C');

    let _c3 = NcCell::with_all('c', 0, 0);
}
