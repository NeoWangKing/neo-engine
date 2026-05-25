#include <stdio.h>
#define NOB_STRIP_PREFIX
#define NOB_IMPLEMENTATION
#include "nob.h"

#include "stb_truetype.h"
#include "stb_image_write.h"
#include "flag.h"

String_Builder sb = {0};

#define WIDTH 512
#define HEIGHT 512
unsigned char pixels[WIDTH*HEIGHT];
stbtt_bakedchar cdata[95];

void usage(void)
{
    fprintf(stderr, "Usage: %s [OPTIONS]\n", flag_program_name());
    fprintf(stderr, "OPTIONS:\n");
    flag_print_options(stderr);
}

int main(int argc, char **argv)
{
    char **input_path  = flag_str("i", NULL, "Input path (MANDATORY)");
    char **output_path = flag_str("o", NULL, "Output path (MANDATORY)");
    char **name        = flag_str("n", NULL, "Name of the font (MANDATORY)");
    bool *help         = flag_bool("help", true, "Print the help message");

    if (!flag_parse(argc, argv)) {
        usage();
        flag_print_error(stderr);
    }

    if (*help) {
        usage();
        // return 0;
    }

    if (*input_path == NULL) {
        usage();
        fprintf(stderr, "ERROR: no input path is provided\n");
        return 1;
    }

    if (*output_path == NULL) {
        usage();
        fprintf(stderr, "ERROR: no output path is provided\n");
        return 1;
    }

    if (*name == NULL) {
        usage();
        fprintf(stderr, "ERROR: no name is provided\n");
        return 1;
    }

    // const char *file_path = "./assets/fonts/JetBrainsMonoNerdFont-Regular.ttf";
    if (!nob_read_entire_file(*input_path, &sb)) return 1;
    nob_log(INFO, "read %zu bytes from %s", sb.count, *input_path);

    float font_height = 100.0f;
    int first_char = 32;
    int n = stbtt_BakeFontBitmap((const unsigned char*)sb.items, 0,
            font_height,
            pixels, WIDTH, HEIGHT,
            first_char, ARRAY_LEN(cdata),
            cdata);
    nob_log(INFO, "n = %d", n);
    assert(n > 0);

    sb.count = 0;
    sb_appendf(&sb, "#pragma once\n");
    sb_appendf(&sb, "int %s_first_char = %d;\n", *name, first_char);
    sb_appendf(&sb, "stbtt_bakedchar %s_cdata[] = {\n", *name);
    for (int i = 0; i < (int)ARRAY_LEN(cdata); ++i) {
        unsigned short x0,y0,x1,y1; float xoff,yoff,xadvance;
        sb_appendf(&sb, "    {.x0 = %4d, .y0 = %4d, .x1 = %4d, .y1 = %4d, .xoff = %10.6f, .yoff = %10.6f, .xadvance = %10.6f},\n",
                cdata[i].x0, cdata[i].y0, cdata[i].x1, cdata[i].y1, cdata[i].xoff, cdata[i].yoff, cdata[i].xadvance);
        // sb_appendf(&sb, "    {}\n");
    }
    sb_appendf(&sb, "};\n");
    sb_appendf(&sb, "#define %s_width %d\n", *name, WIDTH);
    sb_appendf(&sb, "#define %s_height %d\n", *name, n);
    sb_appendf(&sb, "unsigned char %s_pixels[] = {\n", *name);
    int pixels_count = WIDTH*n;
    for (int i = 0; i < WIDTH*n;) {
        sb_appendf(&sb, "    ");
        int row_size = WIDTH;
        for (int j = 0; j < row_size && i < pixels_count; ++i, ++j) {
            sb_appendf(&sb, "%3u,", pixels[i]);
        }
        sb_appendf(&sb, "\n");
    }
    sb_appendf(&sb, "};\n");

    // .h
    // const char *output_h_path = temp_sprintf("%s.h", *output_path);
    if (!write_entire_file(*output_path, sb.items, sb.count)) {
        nob_log(ERROR, "could not generate file %s", *output_path);
        return 1;
    }
    nob_log(INFO, "generated %s", *output_path);

    // .png
    const char *output_png_path = temp_sprintf("%s.png", *output_path);
    if (!stbi_write_png(output_png_path, WIDTH, n, 1, pixels, WIDTH*sizeof(pixels[0]))) {
        nob_log(ERROR, "could not generate file %s", output_png_path);
        return 1;
    }
    nob_log(INFO, "generated %s", output_png_path);

    printf("ttf2png: Hello Neo\n");

    return 0;
}
