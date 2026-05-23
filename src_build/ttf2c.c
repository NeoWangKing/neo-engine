#include <stdio.h>
#define NOB_STRIP_PREFIX
#define NOB_IMPLEMENTATION
#include "nob.h"

// #define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
// #define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

String_Builder sb = {0};

#define WIDTH 512
#define HEIGHT 512
unsigned char pixels[WIDTH*HEIGHT];

stbtt_bakedchar cdata[96];

int main(int argc, char **argv)
{
    const char *file_path = "./assets/fonts/JetBrainsMonoNerdFont-Regular.ttf";
    if (!nob_read_entire_file(file_path, &sb)) return 1;
    nob_log(INFO, "read %zu bytes from %s", sb.count, file_path);

    float font_height = 34.0f;
    int first_char = 32;
    int n = stbtt_BakeFontBitmap((const unsigned char*)sb.items, 0,
            font_height,
            pixels, WIDTH, HEIGHT,
            first_char, ARRAY_LEN(cdata),
            cdata);
    nob_log(INFO, "n = %d", n);
    assert(n > 0);

    sb.count = 0;
    sb_appendf(&sb, "unsigned char pixels[] = {\n");
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

    const char *output_h_path = "font.h";
    if (!write_entire_file(output_h_path, sb.items, sb.count)) return 1;
    nob_log(INFO, "generated %s", output_h_path);

    const char *output_path = "font.png";
    if (!stbi_write_png(output_path, WIDTH, n, 1, pixels, WIDTH*sizeof(pixels[0]))) {
        nob_log(ERROR, "could not save file %s", output_path);
        return 1;
    }

    nob_log(INFO, "generated %s", output_path);

    UNUSED(argc);
    UNUSED(argv);

    printf("ttf2png: Hello Neo\n");

    return 0;
}
