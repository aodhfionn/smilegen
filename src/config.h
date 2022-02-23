#ifndef CONFIG_H
#define CONFIG_H

// dimensions of output image
const int width = 64;
const int height = 64;

const char* filename = "img";
const char* filepath = "../output/";

// whether or not new output images will overwrite the previous one, or create a new co-existing one instead
const int overwrite = 1;

// whether or not to draw control points (off by default)
const int draw_points = 0;

#endif // CONFIG_H