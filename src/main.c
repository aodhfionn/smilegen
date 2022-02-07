#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <png.h>

typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} pixel_t;

typedef struct
{
    pixel_t* pixels;
    size_t width;
    size_t height;
} bitmap_t;

void quit(const char* msg)
{
    printf("%s\n", msg);
    abort();
}

pixel_t* pixel_at(bitmap_t* bitmap, int x, int y)
{
    return bitmap->pixels + bitmap->width * y + x; // returns the pixel at a given x,y coordinate
}

void write_file(bitmap_t* bitmap, const char* fname)
{
    size_t x, y; // used for iterators

    int pixel_size = 4;
    int depth = 8;

    FILE* fp = fopen(fname, "wb");
    if (!fp) quit("Failed to open PNG file");

    png_byte** row_pointers = NULL;

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) quit("Failed to create write struct");

    png_infop info = png_create_info_struct(png);
    if (!info) quit("Failed to create info struct");

    if (setjmp(png_jmpbuf(png))) quit("Error: jmp failure");

    png_set_IHDR(
        png,
        info,
        bitmap->width, bitmap->height,
        depth,
        PNG_COLOR_TYPE_RGBA,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT                
    );

    row_pointers = png_malloc(png, bitmap->height * sizeof(png_byte*));
    for (y = 0; y < bitmap->height; ++y)
    {
        png_byte *row = png_malloc(png, sizeof(uint8_t) * bitmap->width * pixel_size);
        row_pointers[y] = row;

        for (x = 0; x < bitmap->width; ++x)
        {
            pixel_t * pixel = pixel_at(bitmap, x, y);
            *row++ = pixel->r;
            *row++ = pixel->g;
            *row++ = pixel->b;
            *row++ = pixel->a; // fix this
        }
    }

    png_init_io(png, fp);
    png_set_rows(png, info, row_pointers);
    png_write_png(png, info, PNG_TRANSFORM_IDENTITY, NULL);

    for (y = 0; y < bitmap->height; y++)
    {
        png_free(png, row_pointers[y]);
    }
    png_free(png, row_pointers);

    fclose(fp);

    if (png && info)
        png_destroy_write_struct(&png, &info);
}

bitmap_t init_bitmap(int w, int h)
{
    bitmap_t bitmap;

    bitmap.width = w;
    bitmap.height = h;
    bitmap.pixels = calloc(sizeof(pixel_t), bitmap.width * bitmap.height);

    return bitmap;
}

void process(bitmap_t* bitmap, int seed)
{
    for (int y = 0; y < bitmap->height; y++)
    {
        for (int x = 0; x < bitmap->width; x++)
        {
            pixel_t* pixel = pixel_at(bitmap, x, y);
            pixel->r = 255;
            pixel->b = 150;

            pixel->a = 255;
        }
    }
}

int main(int argc, char* argv[])
{
    bitmap_t img = init_bitmap(16, 16);

    int seed;
    if (argc == 1)
    {
        quit("Please provide a seed");
    } else
    {
        seed = atoi(argv[1]);
    }
    
    process(&img, seed);
    write_file(&img, "output.png");

    return 0;
}