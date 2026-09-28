# c99-console-raytracer
Learning project in C99 with no external dependencies or libraries (other than win32)
A very primitve raytracer that renders to the console (Windows only atm).

- uses a binary volume hierarchy for acceleration
- real time viable for small meshes
- scriptable shaders for surfaces and post processing
- can output to older consoles with ANSI 16 or XTERM 256 palettes.

Some shaders draw the final Ascii cell explicitly.

![Unity_Shaderballs]('images/Screenshot_185455.png')

But most shaders generate a RGB color and use a look-up table to convert it.
This is an example on how to generate a LUT to draw cells that could be displayed on an ANSI terminal:

// First we chose our characters.
// since our ascii sequence is most likely not linear we need to supply some coverage values so we get some okayish interpolations.
// Unfortunately every character takes 3 bytes, since we want to display extended ASCII characters, but this helper function will take a simple string of basic ASCII characters.

float coverage[] = {0.0f, 0.05f, 0.11f, 0.18f, 0.31f, 0.42f, 0.58f, 0.72f, 0.86f, 1.0f};
const AsciiPalette symbols_9 = ascii_palette_from_string(" .:-=+*#%@", coverage);


Color ansi_16_colors[16];
ce_generate_ansi_16_palette(ansi_16_colors); // predefined ANSI 16 Colors
AsciiLUT *lut = calloc(1,sizeof(AsciiLUT));
float color_penalty = 0.05;
float luma_penalty = 0.025;
ce_create_lookup_table(lut, ansi_16_colors, 16, symbols_9, 1.0f, color_penalty, luma_penalty);
ce_save_lut_binary(lut, "models/lut_ansi_16_symbols.bin");
free(lut);
