#ifndef NEOVIN_C_
#define NEOVIN_C_

#include <float.h>
#ifndef NEOVIN_C_IMPLEMENTATION
#define NEOVIN_C_IMPLEMENTATION
#endif

#include <math.h>
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#ifndef NEOVINCDEF
#define NEOVINCDEF static inline
#endif

// Anti-aliasing
#ifndef NVC_AA_RES
#define NVC_AA_RES 4
#endif
#define NVC_AA_PAD (1./NVC_AA_RES)

#define NVC_SWAP(T, a, b) do { T t = a; a = b; b = t; } while (0)
#define NVC_SIGN(T, x) ((T)((x) > 0) - (T)((x) < 0))
#define NVC_ABS(T, x) (NVC_SIGN(T, x)*(x))

typedef struct {
    int width, height;
    const char *glyphs;
} NVC_Font;

#define NVC_DEFAULT_FONT_HEIGHT 8
#define NVC_DEFAULT_FONT_WIDTH 5
static char default_font_glyphs[128][NVC_DEFAULT_FONT_HEIGHT][NVC_DEFAULT_FONT_WIDTH] = {
    [' '] = {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
    },
    ['!'] = {
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0},
    },
    ['"'] = {
        {0, 1, 0, 1, 0},
        {0, 1, 0, 1, 0},
        {1, 0, 1, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
    },
    ['#'] = {
        {0, 1, 0, 1, 0},
        {0, 1, 0, 1, 0},
        {1, 1, 1, 1, 1},
        {0, 1, 0, 1, 0},
        {1, 1, 1, 1, 1},
        {0, 1, 0, 1, 0},
        {0, 1, 0, 1, 0},
        {0, 0, 0, 0, 0},
    },
    ['$'] = {
        {0, 0, 1, 0, 0},
        {0, 1, 1, 1, 1},
        {1, 0, 0, 0, 0},
        {0, 1, 1, 1, 0},
        {0, 0, 0, 0, 1},
        {1, 1, 1, 1, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0},
    },
    ['%'] = {
        {1, 0, 0, 0, 1},
        {1, 0, 0, 1, 0},
        {0, 0, 0, 1, 0},
        {0, 0, 1, 0, 0},
        {0, 1, 0, 0, 0},
        {0, 1, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {0, 0, 0, 0, 0},
    },
    ['&'] = {
        {0, 0, 1, 0, 0},
        {0, 1, 0, 1, 0},
        {0, 0, 1, 0, 0},
        {0, 1, 1, 0, 1},
        {1, 0, 1, 1, 0},
        {1, 0, 0, 1, 0},
        {0, 1, 1, 0, 1},
        {0, 0, 0, 0, 0},
    },
    ['\''] = {
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 1, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
    },
    ['('] = {
        {0, 0, 0, 1, 1},
        {0, 0, 1, 0, 0},
        {0, 1, 0, 0, 0},
        {0, 1, 0, 0, 0},
        {0, 1, 0, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 0, 1, 1},
        {0, 0, 0, 0, 0},
    },
    [')'] = {
        {1, 1, 0, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 0, 1, 0},
        {0, 0, 0, 1, 0},
        {0, 0, 0, 1, 0},
        {0, 0, 1, 0, 0},
        {1, 1, 0, 0, 0},
        {0, 0, 0, 0, 0},
    },
    ['*'] = {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 1, 0, 1, 0},
        {0, 0, 1, 0, 0},
        {0, 1, 0, 1, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
    },
    ['+'] = {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 1, 1, 1, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
    },
    [','] = {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 1, 0, 0, 0},
        {0, 0, 0, 0, 0},
    },
    ['-'] = {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 1, 1, 1, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
    },
    ['.'] = {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0},
    },
    ['/'] = {
        {0, 0, 0, 0, 1},
        {0, 0, 0, 1, 0},
        {0, 0, 0, 1, 0},
        {0, 0, 1, 0, 0},
        {0, 1, 0, 0, 0},
        {0, 1, 0, 0, 0},
        {1, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
    },
    ['0'] = {
        {0, 1, 1, 1, 0},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 1, 1},
        {1, 0, 1, 0, 1},
        {1, 1, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {0, 1, 1, 1, 0},
        {0, 0, 0, 0, 0},
    },
    ['1'] = {
        {0, 0, 1, 0, 0},
        {0, 1, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0},
    },
    ['2'] = {
        {0, 1, 1, 1, 0},
        {1, 0, 0, 0, 1},
        {0, 0, 0, 0, 1},
        {0, 0, 0, 1, 0},
        {0, 0, 1, 0, 0},
        {0, 1, 0, 0, 0},
        {1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0},
    },
    ['3'] = {
        {0, 1, 1, 1, 0},
        {1, 0, 0, 0, 1},
        {0, 0, 0, 0, 1},
        {0, 0, 1, 1, 0},
        {0, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {0, 1, 1, 1, 0},
        {0, 0, 0, 0, 0},
    },
    ['4'] = {
        {0, 0, 0, 1, 1},
        {0, 0, 1, 0, 1},
        {0, 1, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 1, 1, 1, 1},
        {0, 0, 0, 0, 1},
        {0, 0, 0, 0, 1},
        {0, 0, 0, 0, 0},
    },
    ['5'] = {
        {1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0},
        {1, 1, 1, 1, 0},
        {0, 0, 0, 0, 1},
        {0, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {0, 1, 1, 1, 0},
        {0, 0, 0, 0, 0},
    },
    ['6'] = {
        {0, 0, 1, 1, 0},
        {0, 1, 0, 0, 0},
        {1, 0, 0, 0, 0},
        {1, 1, 1, 1, 0},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {0, 1, 1, 1, 0},
        {0, 0, 0, 0, 0},
    },
    ['7'] = {
        {1, 1, 1, 1, 1},
        {0, 0, 0, 0, 1},
        {0, 0, 0, 0, 1},
        {0, 0, 0, 1, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0},
    },
    ['8'] = {
        {0, 1, 1, 1, 0},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {0, 1, 1, 1, 0},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {0, 1, 1, 1, 0},
        {0, 0, 0, 0, 0},
    },
    ['9'] = {
        {0, 1, 1, 1, 0},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {0, 1, 1, 1, 1},
        {0, 0, 0, 0, 1},
        {0, 0, 0, 1, 0},
        {0, 1, 1, 0, 0},
        {0, 0, 0, 0, 0},
    },
    [':'] = {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0},
    },
    [';'] = {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 1, 0, 0, 0},
        {0, 0, 0, 0, 0},
    },
    ['<'] = {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 1, 0},
        {0, 0, 1, 0, 0},
        {0, 1, 0, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 0, 1, 0},
        {0, 0, 0, 0, 0},
    },
    ['='] = {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 1, 1, 1, 0},
        {0, 0, 0, 0, 0},
        {0, 1, 1, 1, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
    },
    ['>'] = {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 1, 0, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 0, 1, 0},
        {0, 0, 1, 0, 0},
        {0, 1, 0, 0, 0},
        {0, 0, 0, 0, 0},
    },
    ['?'] = {
        {0, 1, 1, 1, 0},
        {1, 0, 0, 0, 1},
        {0, 0, 0, 0, 1},
        {0, 0, 0, 1, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0},
    },
    ['@'] = {
        {0, 1, 1, 1, 0},
        {1, 0, 0, 0, 1},
        {1, 0, 1, 1, 1},
        {1, 1, 0, 1, 1},
        {1, 0, 1, 1, 1},
        {1, 0, 0, 0, 0},
        {0, 1, 1, 1, 0},
        {0, 0, 0, 0, 0},
    },
    ['['] = {
        {0, 1, 1, 1, 1},
        {0, 1, 0, 0, 0},
        {0, 1, 0, 0, 0},
        {0, 1, 0, 0, 0},
        {0, 1, 0, 0, 0},
        {0, 1, 0, 0, 0},
        {0, 1, 1, 1, 1},
        {0, 0, 0, 0, 0},
    },
    ['\\'] = {
        {1, 0, 0, 0, 0},
        {0, 1, 0, 0, 0},
        {0, 1, 0, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 0, 1, 0},
        {0, 0, 0, 1, 0},
        {0, 0, 0, 0, 1},
        {0, 0, 0, 0, 0},
    },
    [']'] = {
        {1, 1, 1, 1, 0},
        {0, 0, 0, 1, 0},
        {0, 0, 0, 1, 0},
        {0, 0, 0, 1, 0},
        {0, 0, 0, 1, 0},
        {0, 0, 0, 1, 0},
        {1, 1, 1, 1, 0},
        {0, 0, 0, 0, 0},
    },
    ['^'] = {
        {0, 0, 1, 0, 0},
        {0, 1, 0, 1, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
    },
    ['_'] = {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0},
    },
    ['`'] = {
        {0, 1, 0, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
    },
    ['{'] = {
        {0, 0, 0, 1, 1},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 1, 0, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 0, 1, 1},
        {0, 0, 0, 0, 0},
    },
    ['|'] = {
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0},
    },
    ['}'] = {
        {1, 1, 0, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 0, 1, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {1, 1, 0, 0, 0},
        {0, 0, 0, 0, 0},
    },
    ['~'] = {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 1, 0, 0, 0},
        {1, 0, 1, 0, 1},
        {0, 0, 0, 1, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
    },
    ['a'] = {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 1, 1, 1, 0},
        {0, 0, 0, 0, 1},
        {0, 1, 1, 1, 1},
        {1, 0, 0, 0, 1},
        {0, 1, 1, 1, 1},
        {0, 0, 0, 0, 0},
    },
    ['b'] = {
        {1, 0, 0, 0, 0},
        {1, 0, 0, 0, 0},
        {1, 0, 1, 1, 0},
        {1, 1, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 1, 1, 1, 0},
        {0, 0, 0, 0, 0},
    },
    ['c'] = {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 1, 1, 1, 0},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 0},
        {1, 0, 0, 0, 1},
        {0, 1, 1, 1, 0},
        {0, 0, 0, 0, 0},
    },
    ['d'] = {
        {0, 0, 0, 0, 1},
        {0, 0, 0, 0, 1},
        {0, 1, 1, 0, 1},
        {1, 0, 0, 1, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {0, 1, 1, 1, 1},
        {0, 0, 0, 0, 0},
    },
    ['e'] = {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 1, 1, 1, 0},
        {1, 0, 0, 0, 1},
        {1, 1, 1, 1, 0},
        {1, 0, 0, 0, 0},
        {0, 1, 1, 1, 1},
        {0, 0, 0, 0, 0},
    },
    ['f'] = {
        {0, 0, 1, 1, 0},
        {0, 1, 0, 0, 0},
        {1, 1, 1, 1, 1},
        {0, 1, 0, 0, 0},
        {0, 1, 0, 0, 0},
        {0, 1, 0, 0, 0},
        {0, 1, 0, 0, 0},
        {0, 0, 0, 0, 0},
    },
    ['g'] = {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 1, 1, 1, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {0, 1, 1, 1, 1},
        {0, 0, 0, 0, 1},
        {1, 1, 1, 1, 0},
    },
    ['h'] = {
        {1, 0, 0, 0, 0},
        {1, 0, 0, 0, 0},
        {1, 0, 1, 1, 0},
        {1, 1, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {0, 0, 0, 0, 0},
    },
    ['i'] = {
        {0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0},
    },
    ['j'] = {
        {0, 0, 0, 1, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 1, 0},
        {0, 0, 0, 1, 0},
        {0, 0, 0, 1, 0},
        {0, 0, 0, 1, 0},
        {0, 1, 0, 1, 0},
        {0, 0, 1, 0, 0},
    },
    ['k'] = {
        {1, 0, 0, 0, 0},
        {1, 0, 0, 0, 0},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 1, 0},
        {1, 1, 1, 0, 0},
        {1, 0, 0, 1, 0},
        {1, 0, 0, 0, 1},
        {0, 0, 0, 0, 0},
    },
    ['l'] = {
        {0, 1, 0, 0, 0},
        {0, 1, 0, 0, 0},
        {0, 1, 0, 0, 0},
        {0, 1, 0, 0, 0},
        {0, 1, 0, 0, 0},
        {0, 1, 0, 0, 0},
        {0, 0, 1, 1, 0},
        {0, 0, 0, 0, 0},
    },
    ['m'] = {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {1, 1, 0, 1, 0},
        {1, 0, 1, 0, 1},
        {1, 0, 1, 0, 1},
        {1, 0, 1, 0, 1},
        {1, 0, 1, 0, 1},
        {0, 0, 0, 0, 0},
    },
    ['n'] = {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {1, 1, 1, 1, 0},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {0, 0, 0, 0, 0},
    },
    ['o'] = {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 1, 1, 1, 0},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {0, 1, 1, 1, 0},
        {0, 0, 0, 0, 0},
    },
    ['p'] = {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {1, 0, 1, 1, 0},
        {1, 1, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 1, 1, 1, 0},
        {1, 0, 0, 0, 0},
        {1, 0, 0, 0, 0},
    },
    ['q'] = {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 1, 1, 0, 1},
        {1, 0, 0, 1, 1},
        {1, 0, 0, 0, 1},
        {0, 1, 1, 1, 1},
        {0, 0, 0, 0, 1},
        {0, 0, 0, 0, 1},
    },
    ['r'] = {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {1, 0, 1, 1, 0},
        {1, 1, 0, 0, 1},
        {1, 0, 0, 0, 0},
        {1, 0, 0, 0, 0},
        {1, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
    },
    ['s'] = {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 1, 1, 1, 1},
        {1, 0, 0, 0, 0},
        {0, 1, 1, 1, 0},
        {0, 0, 0, 0, 1},
        {1, 1, 1, 1, 0},
        {0, 0, 0, 0, 0},
    },
    ['t'] = {
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {1, 1, 1, 1, 1},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 0, 1, 1},
        {0, 0, 0, 0, 0},
    },
    ['u'] = {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {0, 1, 1, 1, 1},
        {0, 0, 0, 0, 0},
    },
    ['v'] = {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {0, 1, 0, 1, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0},
    },
    ['w'] = {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {1, 0, 1, 0, 1},
        {1, 0, 1, 0, 1},
        {1, 0, 1, 0, 1},
        {1, 0, 1, 0, 1},
        {0, 1, 1, 1, 1},
        {0, 0, 0, 0, 0},
    },
    ['x'] = {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {1, 0, 0, 0, 1},
        {0, 1, 0, 1, 0},
        {0, 0, 1, 0, 0},
        {0, 1, 0, 1, 0},
        {1, 0, 0, 0, 1},
        {0, 0, 0, 0, 0},
    },
    ['y'] = {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {0, 1, 1, 1, 1},
        {0, 0, 0, 0, 1},
        {1, 1, 1, 1, 0},
    },
    ['z'] = {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {1, 1, 1, 1, 1},
        {0, 0, 0, 1, 0},
        {0, 0, 1, 0, 0},
        {0, 1, 0, 0, 0},
        {1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0},
    },
    ['A'] = {
        {0, 1, 1, 1, 0},
        {1, 0, 0, 0, 1},
        {1, 1, 1, 1, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {0, 0, 0, 0, 0},
    },
    ['B'] = {
        {1, 1, 1, 1, 0},
        {1, 0, 0, 0, 1},
        {1, 1, 1, 1, 0},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 1, 1, 1, 0},
        {0, 0, 0, 0, 0},
    },
    ['C'] = {
        {0, 1, 1, 1, 0},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 0},
        {1, 0, 0, 0, 0},
        {1, 0, 0, 0, 0},
        {1, 0, 0, 0, 1},
        {0, 1, 1, 1, 0},
        {0, 0, 0, 0, 0},
    },
    ['D'] = {
        {1, 1, 1, 1, 0},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 1, 1, 1, 0},
        {0, 0, 0, 0, 0},
    },
    ['E'] = {
        {1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0},
        {1, 1, 1, 0, 0},
        {1, 0, 0, 0, 0},
        {1, 0, 0, 0, 0},
        {1, 0, 0, 0, 0},
        {1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0},
    },
    ['F'] = {
        {1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0},
        {1, 1, 1, 0, 0},
        {1, 0, 0, 0, 0},
        {1, 0, 0, 0, 0},
        {1, 0, 0, 0, 0},
        {1, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
    },
    ['G'] = {
        {0, 1, 1, 1, 1},
        {1, 0, 0, 0, 0},
        {1, 0, 1, 1, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {0, 1, 1, 1, 1},
        {0, 0, 0, 0, 0},
    },
    ['H'] = {
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 1, 1, 1, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {0, 0, 0, 0, 0},
    },
    ['I'] = {
        {1, 1, 1, 1, 1},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0},
    },
    ['J'] = {
        {1, 1, 1, 1, 1},
        {0, 0, 0, 0, 1},
        {0, 0, 0, 0, 1},
        {0, 0, 0, 0, 1},
        {0, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {0, 1, 1, 1, 0},
        {0, 0, 0, 0, 0},
    },
    ['K'] = {
        {1, 0, 0, 0, 1},
        {1, 0, 0, 1, 0},
        {1, 1, 1, 0, 0},
        {1, 0, 0, 1, 0},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {0, 0, 0, 0, 0},
    },
    ['L'] = {
        {1, 0, 0, 0, 0},
        {1, 0, 0, 0, 0},
        {1, 0, 0, 0, 0},
        {1, 0, 0, 0, 0},
        {1, 0, 0, 0, 0},
        {1, 0, 0, 0, 0},
        {1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0},
    },
    ['M'] = {
        {1, 0, 0, 0, 1},
        {1, 1, 0, 1, 1},
        {1, 0, 1, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {0, 0, 0, 0, 0},
    },
    ['N'] = {
        {1, 0, 0, 0, 1},
        {1, 1, 0, 0, 1},
        {1, 0, 1, 0, 1},
        {1, 0, 0, 1, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {0, 0, 0, 0, 0},
    },
    ['O'] = {
        {0, 1, 1, 1, 0},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {0, 1, 1, 1, 0},
        {0, 0, 0, 0, 0},
    },
    ['P'] = {
        {1, 1, 1, 1, 0},
        {1, 0, 0, 0, 1},
        {1, 1, 1, 1, 0},
        {1, 0, 0, 0, 0},
        {1, 0, 0, 0, 0},
        {1, 0, 0, 0, 0},
        {1, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
    },
    ['Q'] = {
        {0, 1, 1, 1, 0},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 1, 0},
        {0, 1, 1, 0, 1},
        {0, 0, 0, 0, 0},
    },
    ['R'] = {
        {1, 1, 1, 1, 0},
        {1, 0, 0, 0, 1},
        {1, 1, 1, 1, 0},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {0, 0, 0, 0, 0},
    },
    ['S'] = {
        {0, 1, 1, 1, 1},
        {1, 0, 0, 0, 0},
        {0, 1, 1, 1, 0},
        {0, 0, 0, 0, 1},
        {0, 0, 0, 0, 1},
        {0, 0, 0, 0, 1},
        {1, 1, 1, 1, 0},
        {0, 0, 0, 0, 0},
    },
    ['T'] = {
        {1, 1, 1, 1, 1},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0},
    },
    ['U'] = {
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {0, 1, 1, 1, 0},
        {0, 0, 0, 0, 0},
    },
    ['V'] = {
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {0, 1, 0, 1, 0},
        {0, 1, 0, 1, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0},
    },
    ['W'] = {
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 1, 0, 1},
        {1, 1, 0, 1, 1},
        {1, 0, 0, 0, 1},
        {0, 0, 0, 0, 0},
    },
    ['X'] = {
        {1, 0, 0, 0, 1},
        {0, 1, 0, 1, 0},
        {0, 0, 1, 0, 0},
        {0, 1, 0, 1, 0},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {0, 0, 0, 0, 0},
    },
    ['Y'] = {
        {1, 0, 0, 0, 1},
        {0, 1, 0, 1, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0},
    },
    ['Z'] = {
        {1, 1, 1, 1, 1},
        {0, 0, 0, 0, 1},
        {0, 0, 0, 1, 0},
        {0, 0, 1, 0, 0},
        {0, 1, 0, 0, 0},
        {1, 0, 0, 0, 0},
        {1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0},
    },
};

static NVC_Font NVC_default_font = {
    .glyphs = &default_font_glyphs[0][0][0],
    .width = NVC_DEFAULT_FONT_WIDTH,
    .height = NVC_DEFAULT_FONT_HEIGHT,
};

// COLOR
typedef enum {
    COMP_RED = 0,
    COMP_GREEN,
    COMP_BLUE,
    COMP_ALPHA,
    COUNT_COMP
} COLOR;

NEOVINCDEF void     NVC_Unpack_RGBA32(uint32_t color, uint8_t comp[COUNT_COMP]);
NEOVINCDEF uint8_t  NVC_Red(uint32_t pixel);
NEOVINCDEF uint8_t  NVC_Green(uint32_t pixel);
NEOVINCDEF uint8_t  NVC_Blue(uint32_t pixel);
NEOVINCDEF uint8_t  NVC_Alpha(uint32_t pixel);
NEOVINCDEF uint32_t NVC_RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
NEOVINCDEF uint32_t NVC_Pack_RGBA32(uint8_t comp[COUNT_COMP]);
NEOVINCDEF void     NVC_Modi_Color(uint32_t *color, COLOR index, uint8_t value);
NEOVINCDEF void     NVC_Transparent_Color(uint32_t *color, float alpha);
NEOVINCDEF void     NVC_Bright_Color(uint32_t *color, float rate);
NEOVINCDEF uint32_t NVC_Mix_Colors_Alpha(uint32_t color_b, uint32_t color_t);
NEOVINCDEF uint32_t NVC_Mix_Colors3(uint32_t c1, uint32_t c2, uint32_t c3, float u1, float u2, float u3, float det);
NEOVINCDEF uint32_t NVC_Blend_Color_Normal(uint32_t color_b, uint32_t color_t);
NEOVINCDEF uint32_t NVC_Blend_Color_Darken(uint32_t color_b, uint32_t color_t);
NEOVINCDEF uint32_t NVC_Blend_Color_Multiply(uint32_t color_b, uint32_t color_t);

typedef struct {
    float x;
    float y;
} Vec2D;
#define Vec2D(x, y) ((Vec2D){ (float)(x), (float)(y) })

typedef struct {
    float x;
    float y;
    float z;
} Vec3D;
#define Vec3D(x, y, z) ((Vec3D){ (float)(x), (float)(y), (float)(z) })

// Vec2D
NEOVINCDEF float NVC_Vec2D_Length(Vec2D vec);
NEOVINCDEF float NVC_Vec2D_Angle(Vec2D vec);
NEOVINCDEF Vec2D NVC_Vec2D_Plus(Vec2D vec1, Vec2D vec2);
NEOVINCDEF Vec2D NVC_Vec2D_Minus(Vec2D vec1, Vec2D vec2);
NEOVINCDEF float NVC_Vec2D_Dot(Vec2D vec1, Vec2D vec2);
NEOVINCDEF Vec3D NVC_Vec2D_Cross(Vec2D vec1, Vec2D vec2);

// Vec3D
NEOVINCDEF void NVC_ROTATE_POINT(Vec3D *p, Vec3D refp, Vec3D axis, float angle);
NEOVINCDEF float NVC_Vec3D_Length(Vec3D vec);
NEOVINCDEF Vec3D NVC_Vec3D_Plus(Vec3D vec1, Vec3D vec2);
NEOVINCDEF Vec3D NVC_Vec3D_Minus(Vec3D vec1, Vec3D vec2);
NEOVINCDEF float NVC_Vec3D_Dot(Vec3D vec1, Vec3D vec2);
NEOVINCDEF Vec3D NVC_Vec3D_Cross(Vec3D vec1, Vec3D vec2);

// Canvas 2D
typedef struct {
    int height;
    int width;
    int stride;
    uint32_t *pixels;
} NVC_Canvas;

#define NVC_CANVAS(data, w, h, s) (NVC_Canvas) { \
    .height = (h), \
    .width = (w), \
    .stride = (s), \
    .pixels = (data) \
}
#define NVC_CANVAS_NULL ((NVC_Canvas) {0})
#define NVC_PIXEL(oc, x, y) (oc).pixels[(int)(y)*(oc).stride + (int)(x)]

NEOVINCDEF NVC_Canvas NVC_Make_Canvas(uint32_t *pixels, int width, int height, int stride);
NEOVINCDEF NVC_Canvas NVC_Make_SubCanvas(NVC_Canvas src, Vec2D p, Vec2D s);
NEOVINCDEF void NVC_Set_Pixel(NVC_Canvas oc, Vec2D p, uint32_t color);
NEOVINCDEF void NVC_Draw_Pixel(NVC_Canvas oc, Vec2D p, uint32_t color);

NEOVINCDEF void NVC_Text(NVC_Canvas oc, const char *text, Vec2D tp, NVC_Font font, float ts, uint32_t color);

NEOVINCDEF void NVC_Sprite_Copy(NVC_Canvas oc, int x, int y, int w, int h, NVC_Canvas sprite);
NEOVINCDEF void NVC_Sprite_Blend(NVC_Canvas oc, int x, int y, int w, int h, NVC_Canvas sprite);

typedef NVC_Canvas NVC_Texture;
#define NVC_Texture(t, w, h) (NVC_Texture) { .pixels = (t), .width = (w), .height = (h) }
NEOVINCDEF void NVC_Draw_Texture(NVC_Canvas oc, NVC_Texture texture, Vec2D p, Vec2D s);

typedef struct {
    int x1, x2;
    int y1, y2;
    int ox1, ox2;
    int oy1, oy2;
} NVC_Normalized_Range;

NEOVINCDEF bool  NVC_ISIN_CANVAS(NVC_Canvas oc, Vec2D p);
NEOVINCDEF bool  NVC_NORM_RANGE(NVC_Canvas oc, Vec2D p, Vec2D s, NVC_Normalized_Range *nr);
NEOVINCDEF float NVC_DIST_TO_LINE(Vec2D p, Vec2D p1, Vec2D p2);
NEOVINCDEF bool  NVC_IS_IN_TRIANGLE(Vec2D p, Vec2D p1, Vec2D p2, Vec2D p3);

NEOVINCDEF void NVC_Set_Background(NVC_Canvas oc, uint32_t color);
NEOVINCDEF void NVC_Fill_Background(NVC_Canvas oc, uint32_t color);

NEOVINCDEF void NVC_Fill_Rectangle(NVC_Canvas oc, Vec2D p, Vec2D s, uint32_t color);
NEOVINCDEF void NVC_Draw_Rectangle(NVC_Canvas oc, Vec2D p, Vec2D s, float thick, uint32_t color);

NEOVINCDEF void NVC_Fill_Circle(NVC_Canvas oc, Vec2D p, float r, uint32_t color);
NEOVINCDEF void NVC_Draw_Point(NVC_Canvas oc, Vec2D p, float r, uint32_t color);
NEOVINCDEF void NVC_Draw_Circle(NVC_Canvas oc, Vec2D p, float r, float thick, uint32_t color);
NEOVINCDEF void NVC_Fill_Ellipse(NVC_Canvas oc, Vec2D p, Vec2D size, uint32_t color);
NEOVINCDEF void NVC_Draw_Ellipse(NVC_Canvas oc, Vec2D p, Vec2D size, float thick, uint32_t color);

NEOVINCDEF void NVC_Draw_Line(NVC_Canvas oc, Vec2D p1, Vec2D p2, uint32_t color);
NEOVINCDEF void NVC_Draw_Line_Ex(NVC_Canvas oc, Vec2D p1, Vec2D p2, float thick, uint32_t color);

NEOVINCDEF void NVC_BARYCENTRIC(Vec2D p1, Vec2D p2, Vec2D p3, Vec2D p, float *u1, float *u2, float *u3, float *det);
NEOVINCDEF void NVC_Fill_Triangle(NVC_Canvas oc, Vec2D p1, Vec2D p2, Vec2D p3, uint32_t color);
NEOVINCDEF void NVC_Draw_Triangle(NVC_Canvas oc, Vec2D p1, Vec2D p2, Vec2D p3, float thick, uint32_t color);
NEOVINCDEF void NVC_Fill_Triangle_C3(NVC_Canvas oc, Vec2D p1, Vec2D p2, Vec2D p3, uint32_t c1, uint32_t c2, uint32_t c3);

typedef struct {
    uint32_t *pixel;
    uint32_t *front;
    uint32_t *space;
    uint32_t *envir;
    float    *depth;
    int width;
    int height;
    int stride;
} NVC_Canvas_3D;
#define NVC_CANVAS_3D(pixel_data, front_data, space_data, envir_data, depth_data, w, h, s) (NVC_Canvas_3D) { \
    .pixel = (pixel_data), \
    .front = (front_data), \
    .space = (space_data), \
    .envir = (envir_data), \
    .depth = (depth_data), \
    .width = (w), \
    .height = (h), \
    .stride = (s) \
}
#define NVC_CANVAS_3D_NULL ((NVC_Canvas_3D) {0})
#define NVC_PIXEL_3D(oc, x, y) (oc).pixel[(int)(y)*(oc).stride + (int)(x)]
#define NVC_FRONT_3D(oc, x, y) (oc).front[(int)(y)*(oc).stride + (int)(x)]
#define NVC_SPACE_3D(oc, x, y) (oc).space[(int)(y)*(oc).stride + (int)(x)]
#define NVC_ENVIR_3D(oc, x, y) (oc).envir[(int)(y)*(oc).stride + (int)(x)]
#define NVC_DEPTH_3D(oc, x, y) (oc).depth[(int)(y)*(oc).stride + (int)(x)]

NEOVINCDEF void NVC_Clear_Pixel_3D(NVC_Canvas_3D oc);
NEOVINCDEF void NVC_Clear_Front_3D(NVC_Canvas_3D oc);
NEOVINCDEF void NVC_Clear_Space_3D(NVC_Canvas_3D oc);
NEOVINCDEF void NVC_Clear_Envir_3D(NVC_Canvas_3D oc);
NEOVINCDEF void NVC_Clear_Depth_3D(NVC_Canvas_3D oc);
NEOVINCDEF void NVC_Clear_3D(NVC_Canvas_3D oc);
NEOVINCDEF void NVC_Update_Canvas_3D(NVC_Canvas_3D oc);

NEOVINCDEF void NVC_Set_Pixel_3D(NVC_Canvas_3D oc, Vec2D p, uint32_t color);
NEOVINCDEF void NVC_Draw_Pixel_3D(NVC_Canvas_3D oc, Vec3D p, uint32_t color);

NEOVINCDEF void NVC_Set_Envir_3D(NVC_Canvas_3D oc, Vec2D p, uint32_t color);
NEOVINCDEF void NVC_Fill_Envir_3D(NVC_Canvas_3D oc, uint32_t color);

NEOVINCDEF void NVC_Fill_Circle_3D(NVC_Canvas_3D oc, Vec3D p, float r, uint32_t color);
NEOVINCDEF void NVC_Point_3D(NVC_Canvas_3D oc, Vec3D p, float r, uint32_t color);

NEOVINCDEF void NVC_Draw_Line_3D(NVC_Canvas_3D oc, Vec3D p1, Vec3D p2, uint32_t color);
NEOVINCDEF void NVC_Draw_Line_3D_Ex(NVC_Canvas_3D oc, Vec3D p1, Vec3D p2, float thick, uint32_t color);

NEOVINCDEF void NVC_Fill_Triangle_3D(NVC_Canvas_3D oc, Vec3D p1, Vec3D p2, Vec3D p3, uint32_t color);
NEOVINCDEF void NVC_Fill_Triangle_3D_C3(NVC_Canvas_3D oc, Vec3D p1, Vec3D p2, Vec3D p3, uint32_t c1, uint32_t c2, uint32_t c3);

NEOVINCDEF void NVC_Draw_Cube(NVC_Canvas_3D oc, Vec3D pc, float size, uint32_t color);
NEOVINCDEF void NVC_Draw_Dot(NVC_Canvas_3D oc, Vec3D pc, float size, uint32_t color);

#endif // NEOVIN_C_

#ifdef NEOVIN_C_IMPLEMENTATION

NEOVINCDEF void NVC_ROTATE_POINT(Vec3D *p, Vec3D refp, Vec3D axis, float angle)
{
    float vx = p->x - refp.x;
    float vy = p->y - refp.y;
    float vz = p->z - refp.z;

    float len_dir = sqrtf(axis.x * axis.x + axis.y * axis.y + axis.z * axis.z);
    if (len_dir < 0.0001f) return;
    float ux = axis.x / len_dir;
    float uy = axis.y / len_dir;
    float uz = axis.z / len_dir;

    float cos_a = cosf(angle);
    float sin_a = sinf(angle);
    float one_minus_cos = 1.0f - cos_a;

    float cx = uy * vz - uz * vy;
    float cy = uz * vx - ux * vz;
    float cz = ux * vy - uy * vx;

    float dot = ux * vx + uy * vy + uz * vz;

    float rx = vx * cos_a + cx * sin_a + ux * dot * one_minus_cos;
    float ry = vy * cos_a + cy * sin_a + uy * dot * one_minus_cos;
    float rz = vz * cos_a + cz * sin_a + uz * dot * one_minus_cos;

    p->x = refp.x + rx;
    p->y = refp.y + ry;
    p->z = refp.z + rz;
}

NEOVINCDEF NVC_Canvas NVC_Make_Canvas(uint32_t *pixels, int width, int height, int stride)
{
    return NVC_CANVAS(pixels, width, height, stride);
}

NEOVINCDEF NVC_Canvas NVC_Make_SubCanvas(NVC_Canvas src, Vec2D p, Vec2D s)
{
    int x0 = p.x;
    int y0 = p.y;
    int x1 = x0 + s.x;
    int y1 = y0 + s.y;
    if (x0 > x1) NVC_SWAP(int, x0, x1);
    if (y0 > y1) NVC_SWAP(int, y0, y1);

    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > src.width)  x1 = src.width;
    if (y1 > src.height) y1 = src.height;

    return NVC_CANVAS(src.pixels + y0*src.stride + x0, x1 - x0, y1 - y0, src.stride);
}

NEOVINCDEF void NVC_Set_Pixel(NVC_Canvas oc, Vec2D p, uint32_t color)
{
    // 将相对坐标转换为绝对坐标进行边界检查
    int ax = (int)p.x;
    int ay = (int)p.y;
    if (ax < 0 || ax >= oc.width || ay < 0 || ay >= oc.height) return;
    // 使用 NVC_PIXEL 进行写入
    NVC_PIXEL(oc, p.x, p.y) = color;
}

NEOVINCDEF void NVC_Draw_Pixel(NVC_Canvas oc, Vec2D p, uint32_t color)
{
    int ax = (int)p.x;
    int ay = (int)p.y;
    if (ax < 0 || ax >= oc.width || ay < 0 || ay >= oc.height) return;
    uint32_t* pixel = &NVC_PIXEL(oc, p.x, p.y);
    *pixel = NVC_Mix_Colors_Alpha(*pixel, color);
}

NEOVINCDEF void NVC_Text(NVC_Canvas oc, const char *text, Vec2D tp, NVC_Font font, float ts, uint32_t color)
{
    for (int i = 0; *text; ++i, ++text) {
        int gx = tp.x + i*(ts/font.height)*(font.width+1);
        int gy = tp.y;
        const char *glyph = &font.glyphs[(*text)*sizeof(char)*font.width*font.height];
        for (int dy = 0; (float)dy < ts; ++dy) {
            int y = gy + dy;
            for (int dx = 0; (float)dx < ts*font.width/font.height; ++dx) {
                int x = gx + dx;
                int count = 0;
                for (int sy = 0; sy < NVC_AA_RES; ++sy) {
                    float ry = (dy + NVC_AA_PAD*0.5 + sy*NVC_AA_PAD)*font.height/ts;
                    for (int sx = 0; sx < NVC_AA_RES; ++sx) {
                        float rx = (dx + NVC_AA_PAD*0.5 + sx*NVC_AA_PAD)*font.height/ts;
                        if (rx >= font.width || ry >= font.height) continue;
                        if (glyph[(int)ry*font.width + (int)rx]) count += 1;
                    }
                }
                if (count > 0) {
                    uint32_t pixel_color = color;
                    NVC_Transparent_Color(&pixel_color, ((float)count/(NVC_AA_RES*NVC_AA_RES)));
                    NVC_Draw_Pixel(oc, Vec2D(x, y), pixel_color);
                }
            }
        }
    }
}

NEOVINCDEF void NVC_Sprite_Copy(NVC_Canvas oc, int x, int y, int w, int h, NVC_Canvas sprite)
{
    for (int dy = y; dy < y + h; ++y) {
        if (dy < 0 || dy >= oc.height) continue;
        for (int dx = x; dx < x + w; ++x) {
            if (dx < 0 || dx >= oc.width) continue;
            int ax = sprite.width*(dx - x)/w;
            int ay = sprite.height*(dy - y)/h;
            NVC_Set_Pixel(oc, Vec2D(x, y), NVC_PIXEL(sprite, ax, ay));
        }
    }
}

NEOVINCDEF void NVC_Sprite_Blend(NVC_Canvas oc, int x, int y, int w, int h, NVC_Canvas sprite)
{
    for (int dy = y; dy < y + h; ++y) {
        if (dy < 0 || dy >= oc.height) continue;
        for (int dx = x; dx < x + w; ++x) {
            if (dx < 0 || dx >= oc.width) continue;
            int ax = sprite.width*(dx - x)/w;
            int ay = sprite.height*(dy - y)/h;
            NVC_Draw_Pixel(oc, Vec2D(x, y), NVC_PIXEL(sprite, ax, ay));
        }
    }
}

// NEOVINCDEF NVC_Texture NVC_Load_Texture(const char *file_path)
// {
//     int tw, th;
//     uint32_t *texture;
//     texture = (uint32_t*) stbi_load(png_file_path, &tw, &th, NULL, 4);
//     if (texture == NULL) {
//         fprintf(stderr, "ERROR: could not read file %s: %s\n", png_file_path, strerror(errno));
//     }
//     return NVC_Texture(texture, tw, th);
// }

NEOVINCDEF void NVC_Draw_Texture(NVC_Canvas oc, NVC_Texture texture, Vec2D p, Vec2D s)
{
    int x0 = p.x;
    int y0 = p.y;
    int x1 = x0 + s.x;
    int y1 = y0 + s.y;
    float w = NVC_ABS(float, s.x);
    float h = NVC_ABS(float, s.y);
    if (x0 > x1) NVC_SWAP(int, x0, x1);
    if (y0 > y1) NVC_SWAP(int, y0, y1);
    for (int y = y0; y < y1; ++y) {
        for (int x = x0; x < x1; ++x) {
            if (NVC_ISIN_CANVAS(oc, Vec2D(x, y))) {
                int tx = (float)((x - x0)*texture.width)/w;
                int ty = (float)((y - y0)*texture.height)/h;
                NVC_PIXEL(oc, x, y) = NVC_PIXEL(NVC_CANVAS(texture.pixels, texture.width, texture.height, texture.width), tx, ty);
            }
        }
    }
}

NEOVINCDEF bool NVC_ISIN_CANVAS(NVC_Canvas oc, Vec2D p)
{
    float ax = p.x;
    float ay = p.y;
    if (ax >= 0 && ax < oc.width && ay >= 0 && ay < oc.height) return true;
    return false;
}

NEOVINCDEF bool NVC_NORM_RANGE(NVC_Canvas oc, Vec2D p, Vec2D s, NVC_Normalized_Range *nr)
{
    float x_min = fminf(p.x, p.x + s.x);
    float x_max = fmaxf(p.x, p.x + s.x);
    float y_min = fminf(p.y, p.y + s.y);
    float y_max = fmaxf(p.y, p.y + s.y);

    nr->x1 = (int)floorf(x_min);
    nr->y1 = (int)floorf(y_min);
    nr->x2 = (int)ceilf(x_max) - 1;
    nr->y2 = (int)ceilf(y_max) - 1;

    nr->ox1 = floorf(p.x);
    nr->ox2 = floorf(p.x + s.x);
    nr->oy1 = floorf(p.y);
    nr->oy2 = floorf(p.y + s.y);

    if (nr->x1 >= (int)oc.width)  return false;
    if (nr->x2 < 0)              return false;
    if (nr->y1 >= (int)oc.height) return false;
    if (nr->y2 < 0)              return false;

    if (nr->x1 < 0)                    nr->x1 = 0;
    if (nr->x2 >= (int)oc.width)       nr->x2 = (int)oc.width - 1;
    if (nr->y1 < 0)                    nr->y1 = 0;
    if (nr->y2 >= (int)oc.height)      nr->y2 = (int)oc.height - 1;

    return true;
}

NEOVINCDEF float NVC_Vec2D_Length(Vec2D vec)
{
    return sqrtf(vec.x*vec.x + vec.y*vec.y);
}

NEOVINCDEF float NVC_Vec2D_Angle(Vec2D vec)
{
    return atan2f(vec.y, vec.x);
}

NEOVINCDEF Vec2D NVC_Vec2D_Plus(Vec2D vec1, Vec2D vec2)
{
    return Vec2D(vec1.x + vec2.x, vec1.y + vec2.y);
}

NEOVINCDEF Vec2D NVC_Vec2D_Minus(Vec2D vec1, Vec2D vec2)
{
    return Vec2D(vec1.x - vec2.x, vec1.y - vec2.y);
}

NEOVINCDEF float NVC_Vec2D_Dot(Vec2D vec1, Vec2D vec2)
{
    return (vec1.x*vec2.x + vec1.y*vec2.y);
}

NEOVINCDEF Vec3D NVC_Vec2D_Cross(Vec2D vec1, Vec2D vec2)
{
    return Vec3D(0, 0, vec1.x*vec2.y - vec1.y*vec2.x);
}

NEOVINCDEF float NVC_Vec3D_Length(Vec3D vec)
{
    return sqrtf(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
}

NEOVINCDEF Vec3D NVC_Vec3D_Plus(Vec3D vec1, Vec3D vec2)
{
    return Vec3D(vec1.x + vec2.x, vec1.y + vec2.y, vec1.z + vec2.z);
}

NEOVINCDEF Vec3D NVC_Vec3D_Minus(Vec3D vec1, Vec3D vec2)
{
    return Vec3D(vec1.x - vec2.x, vec1.y - vec2.y, vec1.z - vec2.z);
}

NEOVINCDEF float NVC_Vec3D_Dot(Vec3D vec1, Vec3D vec2)
{
    return (vec1.x*vec2.x + vec1.y*vec2.y + vec1.z*vec2.z);
}

NEOVINCDEF Vec3D NVC_Vec3D_Cross(Vec3D vec1, Vec3D vec2)
{
    return Vec3D(vec1.y*vec2.z - vec1.z*vec2.y, vec1.z*vec2.x - vec1.x*vec2.z, vec1.x*vec2.y - vec1.y*vec2.x);
}

NEOVINCDEF void NVC_Unpack_RGBA32(uint32_t color, uint8_t comp[COUNT_COMP])
{
    for (int i = 0; i < COUNT_COMP; ++i) {
        comp[i] = color&0xFF;
        color >>= 8;
    }
}

NEOVINCDEF uint8_t NVC_Red(uint32_t pixel) { return (pixel >> 0) & 0xFF; }
NEOVINCDEF uint8_t NVC_Green(uint32_t pixel) { return (pixel >> 8) & 0xFF; }
NEOVINCDEF uint8_t NVC_Blue(uint32_t pixel) { return (pixel >> 16) & 0xFF; }
NEOVINCDEF uint8_t NVC_Alpha(uint32_t pixel) { return (pixel >> 24) & 0xFF; }
NEOVINCDEF uint32_t NVC_RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    uint32_t result = 0;
    result |= r<<(8*0);
    result |= g<<(8*1);
    result |= b<<(8*2);
    result |= a<<(8*3);
    return result;
}

NEOVINCDEF uint32_t NVC_Pack_RGBA32(uint8_t comp[COUNT_COMP])
{
    uint32_t result = 0;
    for (int i = 0; i < COUNT_COMP; ++i) {
        result |= comp[i]<<(8*i);
    }
    return result;
}

NEOVINCDEF void NVC_Modi_Color(uint32_t *color, COLOR index, uint8_t value)
{
    uint8_t comp[COUNT_COMP];
    NVC_Unpack_RGBA32(*color, comp);
    comp[index] = value;
    *color = NVC_Pack_RGBA32(comp);
}

NEOVINCDEF void NVC_Transparent_Color(uint32_t *color, float alpha)
{
    uint8_t comp[COUNT_COMP];
    NVC_Unpack_RGBA32(*color, comp);
    comp[COMP_ALPHA] *= alpha;
    *color = NVC_Pack_RGBA32(comp);
}

NEOVINCDEF void NVC_Bright_Color(uint32_t *color, float rate)
{
    if (rate < 0.0f) rate = 0.0f;
    uint8_t comp[COUNT_COMP];
    NVC_Unpack_RGBA32(*color, comp);
    for (int i = 0; i < COMP_ALPHA; ++i) {
        float scaled = (float)comp[i] * rate;
        if (scaled > 255.0f) scaled = 255.0f;
        if (scaled < 0.0f)   scaled = 0.0f;
        comp[i] = (uint8_t)scaled;
    }
    *color = NVC_Pack_RGBA32(comp);
}

NEOVINCDEF uint32_t NVC_Mix_Colors_Alpha(uint32_t color_b, uint32_t color_t)
{
    uint8_t comp_b[COUNT_COMP];
    NVC_Unpack_RGBA32(color_b, comp_b);

    uint8_t comp_t[COUNT_COMP];
    NVC_Unpack_RGBA32(color_t, comp_t);

    uint8_t comp_f[COUNT_COMP];

    comp_f[COMP_ALPHA] = comp_t[COMP_ALPHA] + comp_b[COMP_ALPHA]*(255 - comp_t[COMP_ALPHA])/255;
    if (comp_f[COMP_ALPHA] == 0) return 0x00000000;

    for (int i = 0; i < COMP_ALPHA; ++i) {
        uint32_t sum = (uint32_t)comp_t[i]*comp_t[COMP_ALPHA]*255
            + (uint32_t)comp_b[i]*comp_b[COMP_ALPHA]*(255 - comp_t[COMP_ALPHA]);
        comp_f[i] = (uint8_t)(sum/ (255*comp_f[COMP_ALPHA]));
    }

    return NVC_Pack_RGBA32(comp_f);
}

NEOVINCDEF uint32_t NVC_Mix_Colors3(uint32_t c1, uint32_t c2, uint32_t c3, float u1, float u2, float u3, float det)
{
    int64_t r1 = NVC_Red(c1), g1 = NVC_Green(c1), b1 = NVC_Blue(c1), a1 = NVC_Alpha(c1);
    int64_t r2 = NVC_Red(c2), g2 = NVC_Green(c2), b2 = NVC_Blue(c2), a2 = NVC_Alpha(c2);
    int64_t r3 = NVC_Red(c3), g3 = NVC_Green(c3), b3 = NVC_Blue(c3), a3 = NVC_Alpha(c3);

    int64_t r = r1*u1 + r2*u2 + r3*u3;
    int64_t g = g1*u1 + g2*u2 + g3*u3;
    int64_t b = b1*u1 + b2*u2 + b3*u3;
    int64_t a = a1*u1 + a2*u2 + a3*u3;
    float t = 1.f/det;

    return NVC_RGBA(r*t, g*t, b*t, a*t);
}

NEOVINCDEF uint32_t NVC_Blend_Color_Normal(uint32_t color_b, uint32_t color_t)
{
    uint8_t comp_b[COUNT_COMP];
    NVC_Unpack_RGBA32(color_b, comp_b);

    uint8_t comp_t[COUNT_COMP];
    NVC_Unpack_RGBA32(color_t, comp_t);

    uint8_t comp_f[COUNT_COMP];

    comp_f[COMP_ALPHA] = 255;
    for (int i = 0; i < COMP_ALPHA; ++i) {
        comp_f[i] = comp_t[i];
    }

    return NVC_Pack_RGBA32(comp_f);
}

NEOVINCDEF uint32_t NVC_Blend_Color_Darken(uint32_t color_b, uint32_t color_t)
{
    uint8_t comp_b[COUNT_COMP];
    NVC_Unpack_RGBA32(color_b, comp_b);

    uint8_t comp_t[COUNT_COMP];
    NVC_Unpack_RGBA32(color_t, comp_t);

    uint8_t comp_f[COUNT_COMP];

    comp_f[COMP_ALPHA] = 255;
    for (int i = 0; i < COMP_ALPHA; ++i) {
        comp_f[i] = comp_t[i];
        if (comp_b[i] < comp_t[i]) comp_f[i] = comp_b[i];
    }

    return NVC_Pack_RGBA32(comp_f);
}

NEOVINCDEF uint32_t NVC_Blend_Color_Multiply(uint32_t color_b, uint32_t color_t)
{
    uint8_t comp_b[COUNT_COMP];
    NVC_Unpack_RGBA32(color_b, comp_b);

    uint8_t comp_t[COUNT_COMP];
    NVC_Unpack_RGBA32(color_t, comp_t);

    uint8_t comp_f[COUNT_COMP];

    comp_f[COMP_ALPHA] = 255;
    for (int i = 0; i < COMP_ALPHA; ++i) {
        comp_f[i] = comp_t[i] * comp_b[i] / 255;
    }

    return NVC_Pack_RGBA32(comp_f);
}

NEOVINCDEF void NVC_Set_Background(NVC_Canvas oc, uint32_t color)
{
    for (int y = 0; y < oc.height; ++y) {
        for (int x = 0; x < oc.width; ++x) {
            NVC_Set_Pixel(oc, Vec2D(x, y), color);
        }
    }
}

NEOVINCDEF void NVC_Fill_Background(NVC_Canvas oc, uint32_t color)
{
    for (int y = 0; y < oc.height; ++y) {
        for (int x = 0; x < oc.width; ++x) {
            NVC_Draw_Pixel(oc, Vec2D(x, y), color);
        }
    }
}

NEOVINCDEF void NVC_Fill_Rectangle(NVC_Canvas oc, Vec2D p, Vec2D s, uint32_t color)
{
    float x_min = p.x;
    float x_max = x_min + s.x;
    if (x_min > x_max) NVC_SWAP(float, x_min, x_max);
    float y_min = p.y;
    float y_max = y_min + s.y;
    if (y_min > y_max) NVC_SWAP(float, y_min, y_max);

    NVC_Normalized_Range nr;
    NVC_NORM_RANGE(oc, Vec2D(x_min, y_min), Vec2D(x_max - x_min, y_max - y_min), &nr);

    const float d = 0.5f;

    for (int y = nr.y1; y <= nr.y2; ++y) {
        for (int x = nr.x1; x <= nr.x2; ++x) {
            if (y > y_min + d && y < y_max - d && x > x_min + d && x < x_max - d) {
                NVC_Draw_Pixel(oc, Vec2D(x, y), color);
                continue;
            }
            int count = 0;
            for (int sy = 0; sy < NVC_AA_RES; ++sy) {
                float ay = y + NVC_AA_PAD*0.5 + sy*NVC_AA_PAD;
                for (int sx = 0; sx < NVC_AA_RES; ++sx) {
                    float ax = x + NVC_AA_PAD*0.5 + sx*NVC_AA_PAD;
                    if (ay >= y_min && ay <= y_max && ax >= x_min && ax <= x_max) count += 1;
                }
            }
            if (count > 0) {
                uint32_t pixel_color = color;
                NVC_Transparent_Color(&pixel_color, ((float)count/(NVC_AA_RES*NVC_AA_RES)));
                NVC_Draw_Pixel(oc, Vec2D(x, y), pixel_color);
            }
        }
    }
}

NEOVINCDEF void NVC_Draw_Rectangle(NVC_Canvas oc, Vec2D p, Vec2D s, float thick, uint32_t color)
{
    float x0 = p.x, x1 = p.x + s.x;
    if (x0 > x1) NVC_SWAP(float, x0, x1);
    float y0 = p.y, y1 = p.y + s.y;
    if (y0 > y1) NVC_SWAP(float, y0, y1);

    float outer_x0 = x0 - thick * 0.5f;
    float outer_y0 = y0 - thick * 0.5f;
    float outer_x1 = x1 + thick * 0.5f;
    float outer_y1 = y1 + thick * 0.5f;

    float inner_x0 = x0 + thick * 0.5f;
    float inner_y0 = y0 + thick * 0.5f;
    float inner_x1 = x1 - thick * 0.5f;
    float inner_y1 = y1 - thick * 0.5f;

    NVC_Normalized_Range bnr;
    NVC_NORM_RANGE(oc, Vec2D(outer_x0, outer_y0),
            Vec2D(outer_x1 - outer_x0, outer_y1 - outer_y0),
            &bnr);

    for (int y = bnr.y1; y <= bnr.y2; ++y) {
        for (int x = bnr.x1; x <= bnr.x2; ++x) {
            int count = 0;
            for (int sy = 0; sy < NVC_AA_RES; ++sy) {
                float ay = y + NVC_AA_PAD * 0.5f + sy * NVC_AA_PAD;
                for (int sx = 0; sx < NVC_AA_RES; ++sx) {
                    float ax = x + NVC_AA_PAD * 0.5f + sx * NVC_AA_PAD;
                    // 子像素在外矩形内部，且不在内矩形内部 → 属于边框
                    if (ax >= outer_x0 && ax <= outer_x1 && ay >= outer_y0 && ay <= outer_y1) {
                        if (!(ax >= inner_x0 && ax <= inner_x1 && ay >= inner_y0 && ay <= inner_y1)) {
                            count++;
                        }
                    }
                }
            }
            if (count > 0) {
                uint32_t pixel_color = color;
                NVC_Transparent_Color(&pixel_color, (float)count / (NVC_AA_RES * NVC_AA_RES));
                NVC_Draw_Pixel(oc, Vec2D(x, y), pixel_color);
            }
        }
    }
}

NEOVINCDEF void NVC_Fill_Circle(NVC_Canvas oc, Vec2D p, float r, uint32_t color)
{
    r = NVC_ABS(float ,r);
    NVC_Normalized_Range nr;
    NVC_NORM_RANGE(oc, Vec2D(p.x-r-1, p.y-r-1), Vec2D(2*r+4, 2*r+4), &nr);

    const float d = 2*0.7071f;

    for (int y = nr.y1; y <= nr.y2; ++y) {
        float dy = y - p.y;
        for (int x = nr.x1; x <= nr.x2; ++x) {
            float dx = x - p.x;
            float dist = dx*dx + dy*dy;
            if (dist >= (r + d)*(r + d)) continue;
            if (dist <= (r - d)*(r - d)) {
                NVC_Draw_Pixel(oc, Vec2D(x, y), color);
                continue;
            }
            int count = 0;
            for (int sy = 0; sy < NVC_AA_RES; ++sy) {
                float ay = y + NVC_AA_PAD*0.5 + sy*NVC_AA_PAD;
                float ady = ay - p.y;
                for (int sx = 0; sx < NVC_AA_RES; ++sx) {
                    float ax = x + NVC_AA_PAD*0.5 + sx*NVC_AA_PAD;
                    float adx = ax - p.x;
                    float adist = adx*adx + ady*ady;
                    if (adist <= r*r) count += 1;
                }
            }
            if (count > 0) {
                uint32_t pixel_color = color;
                NVC_Transparent_Color(&pixel_color, ((float)count/(NVC_AA_RES*NVC_AA_RES)));
                NVC_Draw_Pixel(oc, Vec2D(x, y), pixel_color);
            }
        }
    }
}

NEOVINCDEF void NVC_Draw_Point(NVC_Canvas oc, Vec2D p, float r, uint32_t color)
{
    NVC_Fill_Circle(oc, p, r, color);
}

NEOVINCDEF void NVC_Draw_Circle(NVC_Canvas oc, Vec2D p, float r, float thick, uint32_t color)
{
    float r_out = NVC_ABS(float ,r) + thick*0.5f;
    float r_in = r_out - thick;
    if (r_in > r_out) NVC_SWAP(float, r_in, r_out);
    if (r_in <= 0) {
        NVC_Fill_Circle(oc, p, r, color);
        return;
    }

    NVC_Normalized_Range nr;
    NVC_NORM_RANGE(oc, Vec2D(p.x-r_out-1, p.y-r_out-1), Vec2D(2*r_out+4, 2*r_out+4), &nr);

    const float d = 2*0.7071f;

    for (int y = nr.y1; y <= nr.y2; ++y) {
        float dy = y - p.y;
        for (int x = nr.x1; x <= nr.x2; ++x) {
            float dx = x - p.x;
            float dist = dx*dx + dy*dy;
            if (dist >= (r_out + d)*(r_out + d) || dist <= (r_in - d)*(r_in - d)) continue;
            if (dist <= (r_out - d)*(r_out - d) && dist >= (r_in + d)*(r_in + d)) {
                NVC_Draw_Pixel(oc, Vec2D(x, y), color);
                continue;
            }
            int count = 0;
            for (int sy = 0; sy < NVC_AA_RES; ++sy) {
                float ay = y + NVC_AA_PAD*0.5 + sy*NVC_AA_PAD;
                float ady = ay - p.y;
                for (int sx = 0; sx < NVC_AA_RES; ++sx) {
                    float ax = x + NVC_AA_PAD*0.5 + sx*NVC_AA_PAD;
                    float adx = ax - p.x;
                    float adist = adx*adx + ady*ady;
                    if (adist <= r_out*r_out && adist >= r_in*r_in) count += 1;
                }
            }
            if (count > 0) {
                uint32_t pixel_color = color;
                NVC_Transparent_Color(&pixel_color, ((float)count/(NVC_AA_RES*NVC_AA_RES)));
                NVC_Draw_Pixel(oc, Vec2D(x, y), pixel_color);
            }
        }
    }
}

NEOVINCDEF void NVC_Fill_Ellipse(NVC_Canvas oc, Vec2D p, Vec2D size, uint32_t color)
{
    float a = size.x / 2.0f;
    float b = size.y / 2.0f;
    float cx = p.x + a;
    float cy = p.y + b;

    NVC_Normalized_Range nr;
    NVC_NORM_RANGE(oc, p, size, &nr);
    int x_min = (int)floorf(p.x);
    int y_min = (int)floorf(p.y);
    int x_max = (int)ceilf(p.x + size.x);
    int y_max = (int)ceilf(p.y + size.y);
    if (x_min < 0) x_min = 0;
    if (y_min < 0) y_min = 0;
    if (x_max >= oc.width)  x_max = oc.width - 1;
    if (y_max >= oc.height) y_max = oc.height - 1;

    for (int y = y_min; y <= y_max; ++y) {
        for (int x = x_min; x <= x_max; ++x) {
            int count = 0;
            for (int sy = 0; sy < NVC_AA_RES; ++sy) {
                float ay = y + NVC_AA_PAD * 0.5f + sy * NVC_AA_PAD;
                float dy = ay - cy;
                for (int sx = 0; sx < NVC_AA_RES; ++sx) {
                    float ax = x + NVC_AA_PAD * 0.5f + sx * NVC_AA_PAD;
                    float dx = ax - cx;
                    if ((dx*dx)/(a*a) + (dy*dy)/(b*b) <= 1.0f)
                        count++;
                }
            }
            if (count > 0) {
                uint32_t pixel_color = color;
                NVC_Transparent_Color(&pixel_color, (float)count / (NVC_AA_RES * NVC_AA_RES));
                NVC_Draw_Pixel(oc, Vec2D(x, y), pixel_color);
            }
        }
    }
}

NEOVINCDEF void NVC_Draw_Ellipse(NVC_Canvas oc, Vec2D p, Vec2D size, float thick, uint32_t color)
{
    (void)oc;
    (void)p;
    (void)size;
    (void)thick;
    (void)color;
}

NEOVINCDEF void NVC_Draw_Line(NVC_Canvas oc, Vec2D p1, Vec2D p2, uint32_t color)
{
    int x1 = (int)p1.x;
    int y1 = (int)p1.y;
    int x2 = (int)p2.x;
    int y2 = (int)p2.y;

    int dx = NVC_ABS(int, x2 - x1);
    int dy = -NVC_ABS(int, y2 - y1);
    int sx = NVC_SIGN(int, x2 - x1);
    int sy = NVC_SIGN(int, y2 - y1);
    int err = dx + dy;

    while (1) {
        // 只在画布范围内画点
        NVC_Draw_Pixel(oc, Vec2D(x1, y1), color);

        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
    }
}

NEOVINCDEF float NVC_DIST_TO_LINE(Vec2D p, Vec2D p1, Vec2D p2)
{
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    float len = sqrtf(dx*dx + dy*dy);
    float ux = dx / len;
    float uy = dy / len;
    float v1x = p.x - p1.x;
    float v1y = p.y - p1.y;
    float v2x = p.x - p2.x;
    float v2y = p.y - p2.y;
    float dot1 = v1x*ux + v1y*uy;
    float dot2 = v2x*ux + v2y*uy;
    if (dot1 < 0) {
        return sqrtf(v1x*v1x + v1y*v1y);
    }
    if (dot2 > 0) {
        return sqrtf(v2x*v2x + v2y*v2y);
    }
    float nx = v1x - ux*dot1;
    float ny = v1y - uy*dot1;
    return sqrtf(nx*nx + ny*ny);
}

NEOVINCDEF void NVC_Draw_Line_Ex(NVC_Canvas oc, Vec2D p1, Vec2D p2, float thick, uint32_t color)
{
    float half = thick * 0.5;
    int x_min = (int)floorf(fminf(p1.x, p2.x) - half);
    int y_min = (int)floorf(fminf(p1.y, p2.y) - half);
    int x_max = (int)ceilf(fmaxf(p1.x, p2.x) + half);
    int y_max = (int)ceilf(fmaxf(p1.y, p2.y) + half);

    const float d = 2*0.7071f;

    NVC_Normalized_Range nr;
    NVC_NORM_RANGE(oc, Vec2D(x_min, y_min), Vec2D(x_max - x_min, y_max - y_min), &nr);

    for (int x = nr.x1; x <= nr.x2; ++x) {
        for (int y = nr.y1; y <= nr.y2; ++y) {
            if (NVC_DIST_TO_LINE(Vec2D(x+0.5f, y+0.5f), p1, p2) > (half+d)) continue;
            if (NVC_DIST_TO_LINE(Vec2D(x+0.5f, y+0.5f), p1, p2) < (half-d)) {
                NVC_Draw_Pixel(oc, Vec2D(x, y), color);
                continue;
            }
            int count = 0;
            for (int sy = 0; sy < NVC_AA_RES; ++sy) {
                float ay = y + NVC_AA_PAD*0.5 + sy*NVC_AA_PAD;
                for (int sx = 0; sx < NVC_AA_RES; ++sx) {
                    float ax = x + NVC_AA_PAD*0.5 + sx*NVC_AA_PAD;
                    if (NVC_DIST_TO_LINE(Vec2D(ax, ay), p1, p2) <= half) {
                        count += 1;
                    }
                }
            }
            if (count > 0) {
                uint32_t pixel_color = color;
                NVC_Transparent_Color(&pixel_color, ((float)count/(NVC_AA_RES*NVC_AA_RES)));
                NVC_Draw_Pixel(oc, Vec2D(x, y), pixel_color);
            }
        }
    }

}

NEOVINCDEF bool NVC_IS_IN_TRIANGLE(Vec2D p, Vec2D p1, Vec2D p2, Vec2D p3)
{
    float u1, u2, u3, det;
    NVC_BARYCENTRIC(p1, p2, p3, Vec2D(p.x, p.y), &u1, &u2, &u3, &det);
    if (fabsf(det) < 1e-7f) return false;
    return !(u1 < 0 || u2 < 0 || u3 < 0);
}

NEOVINCDEF void NVC_BARYCENTRIC(Vec2D p1, Vec2D p2, Vec2D p3, Vec2D p, float *u1, float *u2, float *u3, float *det)
{
    *det = ((p1.x - p3.x)*(p2.y - p3.y) - (p2.x - p3.x)*(p1.y - p3.y));
    *u1  = ((p2.y - p3.y)*(p.x - p3.x) + (p3.x - p2.x)*(p.y - p3.y));
    *u2  = ((p3.y - p1.y)*(p.x - p3.x) + (p1.x - p3.x)*(p.y - p3.y));
    *u3  = *det - *u1 - *u2;
}


NEOVINCDEF void NVC_Fill_Triangle(NVC_Canvas oc, Vec2D p1, Vec2D p2, Vec2D p3, uint32_t color)
{
    int x_min = (int)floor(fminf(p1.x, fminf(p2.x, p3.x)));
    int y_min = (int)floor(fminf(p1.y, fminf(p2.y, p3.y)));
    int x_max = (int)ceil(fmaxf(p1.x, fmaxf(p2.x, p3.x)));
    int y_max = (int)ceil(fmaxf(p1.y, fmaxf(p2.y, p3.y)));
    NVC_Normalized_Range nr;
    NVC_NORM_RANGE(oc, Vec2D(x_min, y_min), Vec2D(x_max - x_min, y_max - y_min), &nr);
    for (int y = nr.y1; y <= nr.y2; ++y) {
        for (int x = nr.x1; x <= nr.x2; ++x) {
            int count = 0;
            for (int sy = 0; sy < NVC_AA_RES; ++sy) {
                float ay = y + NVC_AA_PAD*0.5 + sy*NVC_AA_PAD;
                for (int sx = 0; sx < NVC_AA_RES; ++sx) {
                    float ax = x + NVC_AA_PAD*0.5 + sx*NVC_AA_PAD;
                    float u1, u2, u3, det;
                    NVC_BARYCENTRIC(p1, p2, p3, Vec2D(ax, ay), &u1, &u2, &u3, &det);
                    if (fabsf(det) <= 0.f) continue;
                    if (u1/det < -1e-3f || u2/det < -1e-3f || u3/det < -1e-3f) continue;
                    count += 1;
                }
            }
            if (count > 0) {
                uint32_t pixel_color = color;
                NVC_Transparent_Color(&pixel_color, ((float)count/(NVC_AA_RES*NVC_AA_RES)));
                NVC_Draw_Pixel(oc, Vec2D(x, y), pixel_color);
            }
        }
    }
}

NEOVINCDEF void NVC_Draw_Triangle(NVC_Canvas oc, Vec2D p1, Vec2D p2, Vec2D p3, float thick, uint32_t color)
{
    NVC_Draw_Line_Ex(oc, p1, p2, thick, color);
    NVC_Draw_Line_Ex(oc, p2, p3, thick, color);
    NVC_Draw_Line_Ex(oc, p3, p1, thick, color);
}

NEOVINCDEF void NVC_Fill_Triangle_C3(NVC_Canvas oc, Vec2D p1, Vec2D p2, Vec2D p3, uint32_t c1, uint32_t c2, uint32_t c3)
{
    int x_min = (int)floor(fminf(p1.x, fminf(p2.x, p3.x)));
    int y_min = (int)floor(fminf(p1.y, fminf(p2.y, p3.y)));
    int x_max = (int)ceil(fmaxf(p1.x, fmaxf(p2.x, p3.x)));
    int y_max = (int)ceil(fmaxf(p1.y, fmaxf(p2.y, p3.y)));
    NVC_Normalized_Range nr;
    NVC_NORM_RANGE(oc, Vec2D(x_min, y_min), Vec2D(x_max - x_min, y_max - y_min), &nr);
    for (int y = nr.y1; y <= nr.y2; ++y) {
        for (int x = nr.x1; x <= nr.x2; ++x) {
            int count = 0;
            uint32_t final_color = 0;
            int total_r = 0, total_g = 0, total_b = 0, total_a = 0;
            for (int sy = 0; sy < NVC_AA_RES; ++sy) {
                float ay = y + NVC_AA_PAD*0.5 + sy*NVC_AA_PAD;
                for (int sx = 0; sx < NVC_AA_RES; ++sx) {
                    float ax = x + NVC_AA_PAD*0.5 + sx*NVC_AA_PAD;
                    float u1, u2, u3, det;
                    NVC_BARYCENTRIC(p1, p2, p3, Vec2D(ax, ay), &u1, &u2, &u3, &det);
                    if (fabsf(det) < 0.f) continue;
                    if (u1/det < -1e-3f || u2/det < -1e-3f || u3/det < -1e-3f) continue;
                    uint32_t sub_color = NVC_Mix_Colors3(c1, c2, c3, u1, u2, u3, det);
                    total_r += NVC_Red(sub_color);
                    total_g += NVC_Green(sub_color);
                    total_b += NVC_Blue(sub_color);
                    total_a += NVC_Alpha(sub_color);
                    count++;
                }
            }
            if (count) {
                final_color = NVC_RGBA(total_r/count, total_g/count, total_b/count, total_a/count);
                NVC_Transparent_Color(&final_color, (float)count / (NVC_AA_RES*NVC_AA_RES));
                NVC_Draw_Pixel(oc, Vec2D(x, y), final_color);
            }
        }
    }
}

NEOVINCDEF void NVC_Clear_Pixel_3D(NVC_Canvas_3D oc)
{
    for (int i = 0; i < oc.width * oc.height; ++i) oc.pixel[i] = 0;
}

NEOVINCDEF void NVC_Clear_Front_3D(NVC_Canvas_3D oc)
{
    for (int i = 0; i < oc.width * oc.height; ++i) oc.front[i] = 0;
}

NEOVINCDEF void NVC_Clear_Space_3D(NVC_Canvas_3D oc)
{
    for (int i = 0; i < oc.width * oc.height; ++i) oc.space[i] = 0;
}

NEOVINCDEF void NVC_Clear_Envir_3D(NVC_Canvas_3D oc)
{
    for (int i = 0; i < oc.width * oc.height; ++i) oc.envir[i] = 0;
}

NEOVINCDEF void NVC_Clear_Depth_3D(NVC_Canvas_3D oc)
{
    for (int i = 0; i < oc.width * oc.height; ++i) oc.depth[i] = FLT_MAX;
}

NEOVINCDEF void NVC_Clear_3D(NVC_Canvas_3D oc)
{
    NVC_Clear_Pixel_3D(oc);
    NVC_Clear_Front_3D(oc);
    NVC_Clear_Space_3D(oc);
    NVC_Clear_Envir_3D(oc);
    NVC_Clear_Depth_3D(oc);
}

NEOVINCDEF void NVC_Update_Canvas_3D(NVC_Canvas_3D oc)
{
    NVC_Clear_Pixel_3D(oc);
    for (int y = 0; y < oc.height; ++y) {
        for (int x = 0; x < oc.width; ++x) {
            uint32_t *pixel = &NVC_PIXEL_3D(oc, x, y);
            uint32_t *front = &NVC_FRONT_3D(oc, x, y);
            uint32_t *space = &NVC_SPACE_3D(oc, x, y);
            uint32_t *env   = &NVC_ENVIR_3D(oc, x, y);
            *pixel = NVC_Mix_Colors_Alpha(*pixel, *env);
            *pixel = NVC_Mix_Colors_Alpha(*pixel, *space);
            *pixel = NVC_Mix_Colors_Alpha(*pixel, *front);
        }
    }
}

NEOVINCDEF void NVC_Set_Pixel_3D(NVC_Canvas_3D oc, Vec2D p, uint32_t color)
{
    int x = p.x;
    int y = p.y;
    if (x < 0 || x >= oc.width || y < 0 || y >= oc.height) return;
    NVC_FRONT_3D(oc, x, y) = color;
}

NEOVINCDEF void NVC_Draw_Pixel_3D(NVC_Canvas_3D oc, Vec3D p, uint32_t color)
{
    if (p.z <= 1) return;
    int x = p.x;
    int y = p.y;
    if (x < 0 || x >= oc.width || y < 0 || y >= oc.height) return;
    uint32_t *space = &NVC_SPACE_3D(oc, x, y);
    float    *depth = &NVC_DEPTH_3D(oc, x, y);
    uint32_t tmp_color = color;
    if (p.z > 1) NVC_Bright_Color(&tmp_color, 1.0f/p.z);
    if (p.z <= *depth) {
        *space = NVC_Mix_Colors_Alpha(*space, tmp_color);
        *depth = p.z;
    } else {
        // 处理一些半透明的情形
        *space = NVC_Mix_Colors_Alpha(tmp_color, *space);
    }
}

NEOVINCDEF void NVC_Set_Envir_3D(NVC_Canvas_3D oc, Vec2D p, uint32_t color)
{
    int x = p.x;
    int y = p.y;
    if (x < 0 || x >= oc.width || y < 0 || y >= oc.height) return;
    uint32_t *env = &NVC_ENVIR_3D(oc, x, y);
    *env = color;
}

NEOVINCDEF void NVC_Fill_Envir_3D(NVC_Canvas_3D oc, uint32_t color)
{
    for (int y = 0; y < oc.height; ++y) {
        for (int x = 0; x < oc.width; ++x) {
            uint32_t *env = &NVC_ENVIR_3D(oc, x, y);
            *env = color;
        }
    }
}

NEOVINCDEF void NVC_Fill_Circle_3D(NVC_Canvas_3D oc, Vec3D p, float r, uint32_t color)
{
    if (p.z <= 1) return;
    r = NVC_ABS(float ,r)/p.z;
    float cx = (1.f + p.x/p.z)*oc.width/2;
    float cy = (1.f - p.y/p.z)*oc.height/2;
    NVC_Normalized_Range nr;
    NVC_Canvas oc_2d = NVC_CANVAS(oc.pixel, oc.width, oc.height, oc.stride);
    NVC_NORM_RANGE(oc_2d, Vec2D(cx-r-1, cy-r-1), Vec2D(2*r+4, 2*r+4), &nr);

    const float d = sqrtf(2.f);

    for (int y = nr.y1; y <= nr.y2; ++y) {
        float dy = y - cy;
        for (int x = nr.x1; x <= nr.x2; ++x) {
            float dx = x - cx;
            float dist = dx*dx + dy*dy;
            if (dist >= (r + d)*(r + d)) continue;
            if (dist <= (r - d)*(r - d)) {
                NVC_Draw_Pixel_3D(oc, Vec3D(x, y, p.z), color);
                continue;
            }
            int count = 0;
            for (int sy = 0; sy < NVC_AA_RES; ++sy) {
                float ay = y + NVC_AA_PAD*0.5 + sy*NVC_AA_PAD;
                float ady = ay - cy;
                for (int sx = 0; sx < NVC_AA_RES; ++sx) {
                    float ax = x + NVC_AA_PAD*0.5 + sx*NVC_AA_PAD;
                    float adx = ax - cx;
                    float adist = adx*adx + ady*ady;
                    if (adist <= r*r) count += 1;
                }
            }
            if (count > 0) {
                uint32_t pixel_color = color;
                NVC_Transparent_Color(&pixel_color, ((float)count/(NVC_AA_RES*NVC_AA_RES)));
                NVC_Draw_Pixel_3D(oc, Vec3D(x, y, p.z), pixel_color);
            }
        }
    }
}

NEOVINCDEF void NVC_Point_3D(NVC_Canvas_3D oc, Vec3D p, float r, uint32_t color)
{
    NVC_Fill_Circle_3D(oc, p, r, color);
}

NEOVINCDEF void NVC_Draw_Line_3D(NVC_Canvas_3D oc, Vec3D p1, Vec3D p2, uint32_t color)
{
    if (p1.z < 1.0f && p2.z < 1.0f) return;

    if (p1.z < 1.0f) {
        p1.x += (p2.x - p1.x) * (1 - p1.z) / (p2.z - p1.z);
        p1.y += (p2.y - p1.y) * (1 - p1.z) / (p2.z - p1.z);
        p1.z = 1.f;
    }
    if (p2.z < 1.0f) {
        p2.x += (p1.x - p2.x) * (1 - p2.z) / (p1.z - p2.z);
        p2.y += (p1.y - p2.y) * (1 - p2.z) / (p1.z - p2.z);
        p2.z = 1.f;
    }
    Vec2D s1 = Vec2D((1.0f + p1.x / p1.z) * oc.width  / 2.0f,
                     (1.0f - p1.y / p1.z) * oc.height / 2.0f);
    Vec2D s2 = Vec2D((1.0f + p2.x / p2.z) * oc.width  / 2.0f,
                     (1.0f - p2.y / p2.z) * oc.height / 2.0f);

    int x1 = (int)s1.x;
    int y1 = (int)s1.y;
    int x2 = (int)s2.x;
    int y2 = (int)s2.y;

    int dx = NVC_ABS(int, x2 - x1);
    int dy = -NVC_ABS(int, y2 - y1);
    int sx = NVC_SIGN(int, x2 - x1);
    int sy = NVC_SIGN(int, y2 - y1);
    int err = dx + dy;

    float invZ1 = 1.0f / p1.z;
    float invZ2 = 1.0f / p2.z;

    float seg_len2 = (s2.x - s1.x)*(s2.x - s1.x) + (s2.y - s1.y)*(s2.y - s1.y);
    if (seg_len2 < 0.0001f) {
        NVC_Draw_Pixel_3D(oc, Vec3D((float)x1, (float)y1, p1.z), color);
        return;
    }

    while (1) {
        float t;
        if (NVC_ABS(int, x2 - x1) >= NVC_ABS(int, y2 - y1)) {
            t = (float)(x1 - (int)s1.x) / (s2.x - s1.x);
        } else {
            t = (float)(y1 - (int)s1.y) / (s2.y - s1.y);
        }
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;

        float invZ = invZ1 + t * (invZ2 - invZ1);
        float z = 1.0f / invZ;

        NVC_Draw_Pixel_3D(oc, Vec3D((float)x1, (float)y1, z), color);

        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
    }
}

NEOVINCDEF void NVC_Draw_Line_3D_Ex(NVC_Canvas_3D oc, Vec3D p1, Vec3D p2, float thick, uint32_t color)
{
    if (p1.z < 1.0f && p2.z < 1.0f) return;

    if (p1.z < 1.0f) {
        p1.x += (p2.x - p1.x) * (1 - p1.z) / (p2.z - p1.z);
        p1.y += (p2.y - p1.y) * (1 - p1.z) / (p2.z - p1.z);
        p1.z = 1.f;
    }
    if (p2.z < 1.0f) {
        p2.x += (p1.x - p2.x) * (1 - p2.z) / (p1.z - p2.z);
        p2.y += (p1.y - p2.y) * (1 - p2.z) / (p1.z - p2.z);
        p2.z = 1.f;
    }

    Vec2D s1 = Vec2D((1.0f + p1.x / p1.z) * oc.width  / 2.0f,
                     (1.0f - p1.y / p1.z) * oc.height / 2.0f);
    Vec2D s2 = Vec2D((1.0f + p2.x / p2.z) * oc.width  / 2.0f,
                     (1.0f - p2.y / p2.z) * oc.height / 2.0f);

    float half = thick * 0.5f;
    int x_min = (int)floorf(fminf(s1.x, s2.x) - half);
    int y_min = (int)floorf(fminf(s1.y, s2.y) - half);
    int x_max = (int)ceilf(fmaxf(s1.x, s2.x) + half);
    int y_max = (int)ceilf(fmaxf(s1.y, s2.y) + half);

    float invZ1 = 1.0f / p1.z;
    float invZ2 = 1.0f / p2.z;

    NVC_Normalized_Range nr;
    NVC_Canvas oc_2d = NVC_CANVAS(oc.pixel, oc.width, oc.height, oc.stride);
    NVC_NORM_RANGE(oc_2d, Vec2D(x_min, y_min), Vec2D(x_max - x_min, y_max - y_min), &nr);

    const float d = 2 * 0.7071f;

    for (int y = nr.y1; y <= nr.y2; ++y) {
        for (int x = nr.x1; x <= nr.x2; ++x) {
            float cx = x + 0.5f;
            float cy = y + 0.5f;
            float dist = NVC_DIST_TO_LINE(Vec2D(cx, cy), s1, s2);

            if (dist > (half + d)) continue;
            if (dist < (half - d)) {
                float t;
                Vec2D diff = NVC_Vec2D_Minus(s2, s1);
                if (NVC_ABS(float, diff.x) >= NVC_ABS(float, diff.y))
                    t = (cx - s1.x) / diff.x;
                else
                    t = (cy - s1.y) / diff.y;
                if (t < 0.0f) t = 0.0f;
                if (t > 1.0f) t = 1.0f;
                float invZ = invZ1 + t * (invZ2 - invZ1);
                float z = 1.0f / invZ;
                NVC_Draw_Pixel_3D(oc, Vec3D((float)x, (float)y, z), color);
                continue;
            }

            int count = 0;
            for (int sy = 0; sy < NVC_AA_RES; ++sy) {
                float ay = y + NVC_AA_PAD * 0.5f + sy * NVC_AA_PAD;
                for (int sx = 0; sx < NVC_AA_RES; ++sx) {
                    float ax = x + NVC_AA_PAD * 0.5f + sx * NVC_AA_PAD;
                    if (NVC_DIST_TO_LINE(Vec2D(ax, ay), s1, s2) <= half) {
                        count++;
                    }
                }
            }
            if (count > 0) {
                float t;
                Vec2D diff = NVC_Vec2D_Minus(s2, s1);
                if (NVC_ABS(float, diff.x) >= NVC_ABS(float, diff.y)) t = (cx - s1.x) / diff.x;
                else t = (cy - s1.y) / diff.y;
                if (t < 0.0f) t = 0.0f;
                if (t > 1.0f) t = 1.0f;
                float invZ = invZ1 + t * (invZ2 - invZ1);
                float z = 1.0f / invZ;

                uint32_t pixel_color = color;
                NVC_Transparent_Color(&pixel_color, (float)count / (NVC_AA_RES * NVC_AA_RES));
                NVC_Draw_Pixel_3D(oc, Vec3D((float)x, (float)y, z), pixel_color);
            }
        }
    }
}

NEOVINCDEF void NVC_Fill_Triangle_3D(NVC_Canvas_3D oc, Vec3D p1, Vec3D p2, Vec3D p3, uint32_t color)
{
    if (p1.z <= 0 && p2.z <= 0 && p3.z <= 0) return;

    Vec2D s1 = Vec2D((1.f + p1.x / p1.z) * oc.width / 2, (1.f - p1.y / p1.z) * oc.height / 2);
    Vec2D s2 = Vec2D((1.f + p2.x / p2.z) * oc.width / 2, (1.f - p2.y / p2.z) * oc.height / 2);
    Vec2D s3 = Vec2D((1.f + p3.x / p3.z) * oc.width / 2, (1.f - p3.y / p3.z) * oc.height / 2);

    int x_min = (int)floor(fminf(s1.x, fminf(s2.x, s3.x)));
    int y_min = (int)floor(fminf(s1.y, fminf(s2.y, s3.y)));
    int x_max = (int)ceil(fmaxf(s1.x, fmaxf(s2.x, s3.x)));
    int y_max = (int)ceil(fmaxf(s1.y, fmaxf(s2.y, s3.y)));

    NVC_Normalized_Range nr;
    NVC_Canvas oc_2d = NVC_CANVAS(oc.pixel, oc.width, oc.height, oc.stride);
    NVC_NORM_RANGE(oc_2d, Vec2D(x_min, y_min), Vec2D(x_max - x_min, y_max - y_min), &nr);

    for (int y = nr.y1; y <= nr.y2; ++y) {
        for (int x = nr.x1; x <= nr.x2; ++x) {
            float z;
            int count = 0;
            float u1, u2, u3, det;
            uint32_t final_color = 0;
            int total_r = 0, total_g = 0, total_b = 0, total_a = 0;
            for (int sy = 0; sy < NVC_AA_RES; ++sy) {
                float ay = y + NVC_AA_PAD*0.5 + sy*NVC_AA_PAD;
                for (int sx = 0; sx < NVC_AA_RES; ++sx) {
                    float ax = x + NVC_AA_PAD*0.5 + sx*NVC_AA_PAD;
                    NVC_BARYCENTRIC(s1, s2, s3, Vec2D(ax, ay), &u1, &u2, &u3, &det);
                    if (fabsf(det) < 0.f) continue;
                    if (u1/det < -1e-3f || u2/det < -1e-3f || u3/det < -1e-3f) continue;
                    float invZ = u1 / p1.z + u2 / p2.z + u3 / p3.z;
                    z = 1.0f / invZ;
                    total_r += NVC_Red(color);
                    total_g += NVC_Green(color);
                    total_b += NVC_Blue(color);
                    total_a += NVC_Alpha(color);
                    count += 1;
                }
            }
            if (count > 0) {
                NVC_BARYCENTRIC(s1, s2, s3, Vec2D(x + 0.5f, y + 0.5f), &u1, &u2, &u3, &det);
                float invZ = u1 / p1.z + u2 / p2.z + u3 / p3.z;
                z = det / invZ;
                final_color = NVC_RGBA(total_r/count, total_g/count, total_b/count, total_a/count);
                NVC_Transparent_Color(&final_color, ((float)count/(NVC_AA_RES*NVC_AA_RES)));
                NVC_Draw_Pixel_3D(oc, Vec3D((float)x, (float)y, z), color);
            }
        }
    }
}

NEOVINCDEF void NVC_Draw_Triangle_3D(NVC_Canvas_3D oc, Vec3D p1, Vec3D p2, Vec3D p3, float thick, uint32_t color)
{
    NVC_Draw_Line_3D_Ex(oc, p1, p2, thick, color);
    NVC_Draw_Line_3D_Ex(oc, p2, p3, thick, color);
    NVC_Draw_Line_3D_Ex(oc, p3, p1, thick, color);
}

NEOVINCDEF void NVC_Fill_Triangle_3D_C3(NVC_Canvas_3D oc, Vec3D p1, Vec3D p2, Vec3D p3, uint32_t c1, uint32_t c2, uint32_t c3)
{
    if (p1.z <= 0 && p2.z <= 0 && p3.z <= 0) return;

    Vec2D s1 = Vec2D((1.f + p1.x / p1.z) * oc.width / 2, (1.f - p1.y / p1.z) * oc.height / 2);
    Vec2D s2 = Vec2D((1.f + p2.x / p2.z) * oc.width / 2, (1.f - p2.y / p2.z) * oc.height / 2);
    Vec2D s3 = Vec2D((1.f + p3.x / p3.z) * oc.width / 2, (1.f - p3.y / p3.z) * oc.height / 2);

    int x_min = (int)floor(fminf(s1.x, fminf(s2.x, s3.x)));
    int y_min = (int)floor(fminf(s1.y, fminf(s2.y, s3.y)));
    int x_max = (int)ceil(fmaxf(s1.x, fmaxf(s2.x, s3.x)));
    int y_max = (int)ceil(fmaxf(s1.y, fmaxf(s2.y, s3.y)));

    NVC_Normalized_Range nr;
    NVC_Canvas oc_2d = NVC_CANVAS(oc.pixel, oc.width, oc.height, oc.stride);
    NVC_NORM_RANGE(oc_2d, Vec2D(x_min, y_min), Vec2D(x_max - x_min, y_max - y_min), &nr);

    for (int y = nr.y1; y <= nr.y2; ++y) {
        for (int x = nr.x1; x <= nr.x2; ++x) {
            float z;
            float u1, u2, u3, det;
            int count = 0;
            uint32_t final_color = 0;
            int total_r = 0, total_g = 0, total_b = 0, total_a = 0;
            for (int sy = 0; sy < NVC_AA_RES; ++sy) {
                float ay = y + NVC_AA_PAD*0.5 + sy*NVC_AA_PAD;
                for (int sx = 0; sx < NVC_AA_RES; ++sx) {
                    float ax = x + NVC_AA_PAD*0.5 + sx*NVC_AA_PAD;
                    NVC_BARYCENTRIC(s1, s2, s3, Vec2D(ax, ay), &u1, &u2, &u3, &det);
                    if (fabsf(det) < 1e-7f) continue;
                    if (u1/det < -1e-3f || u2/det < -1e-3f || u3/det < -1e-3f) continue;
                    float w1 = u1 / p1.z, w2 = u2 / p2.z, w3 = u3 / p3.z;
                    float sumW = w1 + w2 + w3;
                    total_r += (uint8_t)((w1 * NVC_Red(c1) + w2 * NVC_Red(c2) + w3 * NVC_Red(c3)) / sumW + 0.5f);
                    total_g += (uint8_t)((w1 * NVC_Green(c1) + w2 * NVC_Green(c2) + w3 * NVC_Green(c3)) / sumW + 0.5f);
                    total_b += (uint8_t)((w1 * NVC_Blue(c1) + w2 * NVC_Blue(c2) + w3 * NVC_Blue(c3)) / sumW + 0.5f);
                    total_a += (uint8_t)((w1 * NVC_Alpha(c1) + w2 * NVC_Alpha(c2) + w3 * NVC_Alpha(c3)) / sumW + 0.5f);
                    count += 1;
                }
            }
            if (count > 0) {
                NVC_BARYCENTRIC(s1, s2, s3, Vec2D(x + 0.5f, y + 0.5f), &u1, &u2, &u3, &det);
                float invZ = u1 / p1.z + u2 / p2.z + u3 / p3.z;
                z = det / invZ;
                if (z < 1e-6) continue;
                final_color = NVC_RGBA(total_r/count, total_g/count, total_b/count, total_a/count);
                NVC_Transparent_Color(&final_color, ((float)count/(NVC_AA_RES*NVC_AA_RES)));
                NVC_Draw_Pixel_3D(oc, Vec3D((float)x, (float)y, z), final_color);
            }
        }
    }
}

NEOVINCDEF void NVC_Draw_Cube(NVC_Canvas_3D oc, Vec3D pc, float size, uint32_t color)
{
    float h = size * 0.5f;
    Vec3D v[8] = {
        { pc.x - h, pc.y - h, pc.z - h },
        { pc.x + h, pc.y - h, pc.z - h },
        { pc.x + h, pc.y + h, pc.z - h },
        { pc.x - h, pc.y + h, pc.z - h },
        { pc.x - h, pc.y - h, pc.z + h },
        { pc.x + h, pc.y - h, pc.z + h },
        { pc.x + h, pc.y + h, pc.z + h },
        { pc.x - h, pc.y + h, pc.z + h },
    };

    // 前面
    NVC_Draw_Line_3D_Ex(oc, v[0], v[1], 2.0f, color);
    NVC_Draw_Line_3D_Ex(oc, v[1], v[2], 2.0f, color);
    NVC_Draw_Line_3D_Ex(oc, v[2], v[3], 2.0f, color);
    NVC_Draw_Line_3D_Ex(oc, v[3], v[0], 2.0f, color);
    // 后面
    NVC_Draw_Line_3D_Ex(oc, v[4], v[5], 2.0f, color);
    NVC_Draw_Line_3D_Ex(oc, v[5], v[6], 2.0f, color);
    NVC_Draw_Line_3D_Ex(oc, v[6], v[7], 2.0f, color);
    NVC_Draw_Line_3D_Ex(oc, v[7], v[4], 2.0f, color);
    // 连接前后
    NVC_Draw_Line_3D_Ex(oc, v[0], v[4], 2.0f, color);
    NVC_Draw_Line_3D_Ex(oc, v[1], v[5], 2.0f, color);
    NVC_Draw_Line_3D_Ex(oc, v[2], v[6], 2.0f, color);
    NVC_Draw_Line_3D_Ex(oc, v[3], v[7], 2.0f, color);
}

NEOVINCDEF void NVC_Draw_Dot(NVC_Canvas_3D oc, Vec3D pc, float size, uint32_t color)
{
    NVC_Draw_Cube(oc, pc, 0.01*size, color);
}

#endif // NEOVIN_C_IMPLEMENTATION

// TODO: Font loading
// TODO: Texture loading
// TODO: 3D drawing
// TODO：Drawing ellipse
