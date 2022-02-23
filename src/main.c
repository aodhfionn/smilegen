#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <dirent.h>
#include <png.h>

#include "config.h"

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

struct point
{
    unsigned int x, y;
};

void quit(const char* msg)
{
    printf("%s\n", msg);
    abort();
}

pixel_t* pixel_at(bitmap_t* bitmap, int x, int y)
{
    return bitmap->pixels + bitmap->width * y + x; // returns the pixel at a given x,y coordinate
}

void write_file(bitmap_t* bitmap, const char* fname, const char* fpath)
{
    size_t x, y; // used for iterators

    int pixel_size = 4;
    int depth = 8;

    char* path = strdup(fpath);
    char* name = strdup(fname);
    
    if (!overwrite) // counts the amount of files in the directory, and appends said amount to the end of the filename
    {
        int count = 0;

        DIR* dirp;
        struct dirent* entry;

        dirp = opendir(fpath);
        while ((entry = readdir(dirp)) != NULL)
        {
            if (entry->d_type == DT_REG)
            {
                count++;
            }
        }

        char buf[64];
        sprintf(buf, "%d", count);

        strcat(name, buf);
    }

    strcat(path, name);
    strcat(path, ".png");

    FILE* fp = fopen(path, "wb");
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

void draw(pixel_t* px, int cmode)
{
    px->a = 255;

    begin:
    switch (cmode)
    {
        case 0: // white 
            px->r = 255;
            px->g = 255;
            px->g = 255;
            break;
        case 1: // black
            px->r = 1;
            px->g = 1;
            px->b = 1;
            break;
        case 2: // inverted
            px->r = 255-px->r;
            px->g = 255-px->g;
            px->b = 255-px->b;
            break;
        default:
        {
            int* cm = &cmode;

            if (cmode < 0) // doesnt really work i think but its fine
            {
                *cm += abs(cmode);
                goto begin;
            }

            *cm = cmode%3;
            goto begin;
        }
    }
}

void process(bitmap_t* bitmap, int seed)
{
    srand(seed);
    int noise = rand()%25; // ends up being in the range of -n to n
    
    // tint values
    int
    rt = rand()%255,
    gt = rand()%255,
    bt = rand()%255;

    for (int y = 0; y < bitmap->height; y++)
    {
        for (int x = 0; x < bitmap->width; x++)
        {
            pixel_t* pixel = pixel_at(bitmap, x, y);
            
            pixel->r = rt-((rand()%noise*2)-noise);
            pixel->g = gt-((rand()%noise*2)-noise);
            pixel->b = bt-((rand()%noise*2)-noise);
            // format as struct

            pixel->a = 255;
        }
    }

    int cmode = rand()%3;

    for (int i = 0; i < 2; i++)
    {
        static int
        indent[2] = {8,8},
        radius;

        if (i == 0) 
            radius = (rand()%5)+5;
        
        // make all this stuff relative to the dimensions
        int pos[2] = {-radius-indent[0],-radius-indent[1]};

        for (int y = 0; y <= height; y++)
        {
            for (int x = 0; x <= width; x++)
            {
                if (pow((x+pos[0]), 2) + pow((y+pos[1]), 2) <= pow(radius, 2)) // (x + P_1)^2 + (y + P_2)^2 <= R^2
                {
                    pixel_t* px = pixel_at(bitmap, x, y);

                    draw(px, cmode);
                }
            }
        }

        radius = (rand()%5)+5;
        indent[0] = width - (radius*2 + indent[0]);
    }

    struct point pts[4];

    // point gen

    for (int i = 1; i <= 4; i++)
    {
        const int three_quarters = height-(height/4);

        int xMin = 0, yMin = height/2,
        xMax = width/2, yMax = three_quarters;

        if (i%2)
        {
            xMin = width/2;
            xMax = width; 
        }

        if (i > 2)
        {
            yMin = three_quarters;
            yMax = height;
        }
        
        pts[i-1].x = (rand()%(xMax-xMin))+xMin;
        pts[i-1].y = (rand()%(yMax-yMin))+yMin;

        pixel_t* px = pixel_at(bitmap, pts[i-1].x, pts[i-1].y);

        if (draw_points)
            draw(px, cmode);
    }

    // bezier curve
    
    double x, y, t;
    for (t = 0.0; t <= 1.0; t += 0.01) // bug with rounding precision, will sometimes leave gaps inbetween line
    {
        x = pow(1-t, 3)*pts[0].x + 3*t*pow(1-t, 2)*pts[2].x + 3*pow(t, 2)*(1-t)*pts[3].x + pow(t, 3)*pts[1].x;
        y = pow(1-t, 3)*pts[0].y + 3*t*pow(1-t, 2)*pts[2].y + 3*pow(t, 2)*(1-t)*pts[3].y + pow(t, 3)*pts[1].y;

        // current order: 0231
        // should be 0123 but points are plotted in wrong order (WONT FIX)

        pixel_t* px = pixel_at(bitmap, (int)x, (int)y);
        draw(px, cmode);
    }
}

int main(int argc, char* argv[])
{
    int seed;
    if (argc == 1)
        quit("Please provide a seed");
    else
        seed = atoi(argv[1]);

    bitmap_t bitmap;
    bitmap.width = width;
    bitmap.height = height;
    bitmap.pixels = calloc(sizeof(pixel_t), bitmap.width * bitmap.height);

    process(&bitmap, seed);
    write_file(&bitmap, filename, filepath);

    return 0;
}