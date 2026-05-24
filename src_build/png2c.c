#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

// #define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#include "flag.h"

void generate_c_code_from_pixels(FILE *out, uint32_t *data, int x, int y, const char *name)
{
    size_t name_len = strlen(name);
    char *capital_name = malloc(name_len + 1);
    assert(capital_name != NULL && "Buy more RAM, I guess");
    for (size_t i = 0; i < name_len; ++i) {
        capital_name[i] = toupper(name[i]);
    }
    capital_name[name_len] = '\0';

    fprintf(out, "#ifndef %s_H_\n", capital_name);
    fprintf(out, "#define %s_H_\n", capital_name);
    fprintf(out, "size_t %s_width = %d;\n", name, x);
    fprintf(out, "size_t %s_height = %d;\n", name, y);
    fprintf(out, "uint32_t %s_pixels[] = {\n", name);
    size_t length = (size_t)(x * y);
    size_t width = 7;
    for (size_t i = 0; i < (length + width - 1)/width; ++i) {
        fprintf(out, "   ");
        for (size_t j = 0; j < width && i*width + j < length; ++j) {
            fprintf(out, "0x%08X,", data[i*width + j]);
        }
        fprintf(out, "\n");
    }
    fprintf(out, "};\n");
    fprintf(out, "#endif // %s_H_\n", capital_name);
    free(capital_name);
}

bool generate_c_file_from_png(const char *input_file_path, const char *output_file_path, const char *name)
{
    bool result = true;
    FILE *out = NULL;
    uint32_t *data = NULL;

    int x, y;
    data = (uint32_t *)stbi_load(input_file_path, &x, &y, NULL, 4);
    if (data == NULL) {
        fprintf(stderr, "ERROR: Could not load file `%s`: %s\n", input_file_path, stbi_failure_reason());
        return false;
    }

    if (output_file_path) {
        out = fopen(output_file_path, "wb");
        if (out == NULL) {
            fprintf(stderr, "ERROR: could not write to file `%s`: %s\n", output_file_path, strerror(errno));
            result = false;
            goto cleanup;
        }
        generate_c_code_from_pixels(out, data, x, y, name);
    } else {
        generate_c_code_from_pixels(stdout, data, x, y, name);
    }

cleanup:
    if (out) fclose(out);
    if (data) stbi_image_free(data);
    return result;
}

int main(int argc, char **argv)
{
    // 定义命令行选项
    char **input_path  = flag_str("i", NULL, "Input PNG file path (required)");
    char **output_path = flag_str("o", NULL, "Output header file path (optional, default: stdout)");
    char **name        = flag_str("n", "png", "Name of the generated C identifier (default: png)");
    bool  *help        = flag_bool("help", false, "Print this help message");

    // 解析参数
    if (!flag_parse(argc, argv)) {
        flag_print_error(stderr);
        return 1;
    }

    if (*help) {
        fprintf(stderr, "Usage: %s [OPTIONS]\n\n", flag_program_name());
        fprintf(stderr, "Convert PNG image to C header file containing pixel data.\n\n");
        fprintf(stderr, "OPTIONS:\n");
        flag_print_options(stderr);
        // return 0;
    }

    // 检查必需参数
    if (*input_path == NULL) {
        fprintf(stderr, "ERROR: missing required option -i <input.png>\n");
        return 1;
    }

    // 验证名称合法性
    const char *name_str = *name;
    size_t n = strlen(name_str);
    if (n == 0) {
        fprintf(stderr, "ERROR: name cannot be empty\n");
        return 1;
    }
    if (isdigit(name_str[0])) {
        fprintf(stderr, "ERROR: name cannot start with a digit\n");
        return 1;
    }
    for (size_t i = 0; i < n; ++i) {
        if (!isalnum(name_str[i]) && name_str[i] != '_') {
            fprintf(stderr, "ERROR: name can only contain alphanumeric characters and underscores\n");
            return 1;
        }
    }

    if (!generate_c_file_from_png(*input_path, *output_path, name_str)) {
        return 1;
    }

    if (*output_path) {
        nob_log(NOB_INFO, "Generated %s", *output_path);
    }
    return 0;
}
