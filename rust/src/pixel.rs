//! The ncpixel API facilitates direct management of the pixels within an
//! ncvisual (ncvisuals keep a backing store of 32-bit RGBA pixels, and render
//! them down to terminal graphics in ncvisual_render()).
//
// - NOTE: The pixel color & alpha components are u8 instead of u32.
//   Because of type enforcing, some runtime checks are now unnecessary.
//
// - NOTE: None of the functions can't fail anymore and don't have to return an error.
//
// functions manually reimplemented: 10
// ------------------------------------------
// (+) implement : 10 /  0
// (#) unit tests:  0 / 10
// ------------------------------------------
// + ncpixel
// + ncpixel_a
// + ncpixel_b
// + ncpixel_g
// + ncpixel_r
// + ncpixel_set_a
// + ncpixel_set_b
// + ncpixel_set_g
// + ncpixel_set_r
// + ncpixel_set_rgb

use crate::NcColor;

// NcPixel (RGBA)
/// 32 bits broken into RGB + 8-bit alpha
///
/// NcPixel has 8 bits of alpha,  more or less linear, contributing
/// directly to the usual alpha blending equation.
///
/// We map the 8 bits of alpha to 2 bits of alpha via a level function:
/// https://nick-black.com/dankwiki/index.php?title=Notcurses#Transparency.2FContrasting
///
/// ## Diagram
///
/// ```txt
/// AAAAAAAA GGGGGGGG BBBBBBBB RRRRRRRR
/// ```
/// `type in C: ncpixel (uint32_t)`
///
// NOTE: the order of the colors is different than in NcChannel.
pub type NcPixel = u32;

/// Get an RGB pixel from RGB values
pub fn ncpixel(r: NcColor, g: NcColor, b: NcColor) -> NcPixel {
    0xff000000 as NcPixel | r as NcPixel | (b as NcPixel) << 8 | (g as NcPixel) << 16
}

/// Extract the 8-bit alpha component from a pixel
pub fn ncpixel_a(pixel: NcPixel) -> NcColor {
    ((pixel & 0xff000000) >> 24) as NcColor
}

/// Extract the 8 bit green component from a pixel
pub fn ncpixel_g(pixel: NcPixel) -> NcColor {
    ((pixel & 0x00ff0000) >> 16) as NcColor
}

/// Extract the 8 bit blue component from a pixel
pub fn ncpixel_b(pixel: NcPixel) -> NcColor {
    ((pixel & 0x0000ff00) >> 8) as NcColor
}

/// Extract the 8 bit red component from a pixel
pub fn ncpixel_r(pixel: NcPixel) -> NcColor {
    (pixel & 0x000000ff) as NcColor
}

/// Set the 8-bit alpha component of a pixel
pub fn ncpixel_set_a(pixel: &mut NcPixel, alpha: NcColor) {
    *pixel = (*pixel & 0x00ffffff) | ((alpha as NcPixel) << 24);
}

/// Set the 8-bit green component of a pixel
pub fn ncpixel_set_g(pixel: &mut NcPixel, green: NcColor) {
    *pixel = (*pixel & 0xff00ffff) | ((green as NcPixel) << 16);
}

/// Set the 8-bit blue component of a pixel
pub fn ncpixel_set_b(pixel: &mut NcPixel, blue: NcColor) {
    *pixel = (*pixel & 0xffff00ff) | ((blue as NcPixel) << 8);
}

/// Set the 8-bit red component of a pixel
pub fn ncpixel_set_r(pixel: &mut NcPixel, red: NcColor) {
    *pixel = (*pixel & 0xffffff00) | red as NcPixel;
}

/// set the RGB values of an RGB pixel
pub fn ncpixel_set_rgb(pixel: &mut NcPixel, red: NcColor, green: NcColor, blue: NcColor) {
    ncpixel_set_r(pixel, red);
    ncpixel_set_g(pixel, green);
    ncpixel_set_b(pixel, blue);
}
