#include <complex.h>
#define NOB_IMPLEMENTATION
#include "include/nob.h"

#define ASSETS_FOLDER "./assets/"
#define BUILD_FOLDER     "./build/"
#define SRC_FOLDER       "./src/"
#define INCLUDE_FOLDER       "./include/"
#define SRC_BUILD_FOLDER "./src_build/"

#ifdef __APPLE__
void cmd_cflags_x11(Nob_Cmd *cmd)
{
    cmd_append(cmd, "-I/opt/homebrew/Cellar/libx11/1.8.13/include");
    cmd_append(cmd, "-I/opt/homebrew/Cellar/xorgproto/2025.1/include");
    cmd_append(cmd, "-I/opt/homebrew/Cellar/libxcb/1.17.0/include");
    cmd_append(cmd, "-I/opt/homebrew/Cellar/libxau/1.0.12/include");
    cmd_append(cmd, "-I/opt/homebrew/Cellar/libxdmcp/1.1.5/include");
}

void cmd_libs_x11(Nob_Cmd *cmd)
{
    cmd_append(cmd, "-L/opt/homebrew/Cellar/libxext/1.3.7/lib");
    cmd_append(cmd, "-lXext"); 
    cmd_append(cmd, "-L/opt/homebrew/Cellar/libx11/1.8.13/lib");
    cmd_append(cmd, "-lX11");
}

void cmd_cflags_pa(Nob_Cmd *cmd)
{
    cmd_append(cmd, "-I/opt/homebrew/Cellar/pulseaudio/17.0/include");
    cmd_append(cmd, "-I/opt/homebrew/Cellar/glib/2.88.1/include/glib-2.0");
    cmd_append(cmd, "-I/opt/homebrew/Cellar/glib/2.88.1/lib/glib-2.0/include");
    cmd_append(cmd, "-I/opt/homebrew/opt/gettext/include");
    cmd_append(cmd, "-I/opt/homebrew/Cellar/pcre2/10.47_1/include");
}

void cmd_libs_pa(Nob_Cmd *cmd)
{
    cmd_append(cmd, "-D_REENTRANT");
    cmd_append(cmd, "-L/opt/homebrew/Cellar/pulseaudio/17.0/lib");
    cmd_append(cmd, "-lpulse-mainloop-glib");
    cmd_append(cmd, "-L/opt/homebrew/Cellar/glib/2.88.1/lib");
    cmd_append(cmd, "-lglib-2.0");
    cmd_append(cmd, "-L/opt/homebrew/opt/gettext/lib");
    cmd_append(cmd, "-lintl");
    cmd_append(cmd, "-lpulse-simple");
    cmd_append(cmd, "-lpulse");
    cmd_append(cmd, "-pthread");
}

void cmd_framework(Nob_Cmd *cmd)
{
    nob_cmd_append(cmd, "-framework", "CoreVideo");
    nob_cmd_append(cmd, "-framework", "Cocoa");
    nob_cmd_append(cmd, "-framework", "IOKit");
}
#elif __linux__
void cmd_cflags_x11(Nob_Cmd *cmd)
{
    (void) cmd;
}

void cmd_libs_x11(Nob_Cmd *cmd)
{
    cmd_append(cmd, "-lX11");
    cmd_append(cmd, "-lXext"); 
}

void cmd_cflags_pa(Nob_Cmd *cmd)
{
    (void) cmd;
}

void cmd_libs_pa(Nob_Cmd *cmd)
{
    cmd_append(cmd, "-lpulse");
    cmd_append(cmd, "-lpulse-simple");
    cmd_append(cmd, "-lpulse-mainloop-glib");
    cmd_append(cmd, "-lglib-2.0");
    // cmd_append(cmd, "-lintl");
    cmd_append(cmd, "-pthread");
}

void cmd_framework(Nob_Cmd *cmd)
{
    (void) cmd;
}
#else

#error "Unsupported platform"

#endif

void compile_common(Nob_Cmd *cmd)
{
    cmd_append(cmd, "clang");
    cmd_append(cmd, "-Wall", "-Wextra", "-ggdb");
    cmd_append(cmd, "-I"INCLUDE_FOLDER);
    cmd_append(cmd, "-I"BUILD_FOLDER);
    cmd_append(cmd, "-O3");
    cmd_append(cmd, "-Wno-tautological-compare");
    cmd_append(cmd, "-Wno-unused-variable");
    cmd_append(cmd, "-march=native");
}

bool rebuild_includes(Nob_Cmd *cmd, Nob_Procs *procs)
{
    static struct {
        const char *input;
        const char *output;
        const char *macro;
    } stb_headers[] = {
        {
            .input = INCLUDE_FOLDER"stb_truetype.h",
            .output = BUILD_FOLDER"stb_truetype.o",
            .macro = "-DSTB_TRUETYPE_IMPLEMENTATION",
        },
        {
            .input = INCLUDE_FOLDER"stb_image_write.h",
            .output = BUILD_FOLDER"stb_image_write.o",
            .macro = "-DSTB_IMAGE_WRITE_IMPLEMENTATION",
        },
        {
            .input = INCLUDE_FOLDER"flag.h",
            .output = BUILD_FOLDER"flag.o",
            .macro = "-DFLAG_IMPLEMENTATION",
        },
        {
            .input = INCLUDE_FOLDER"stb_vorbis.c",
            .output = BUILD_FOLDER"stb_vorbis.o",
        },
    };

    for (size_t i = 0; i < ARRAY_LEN(stb_headers); ++i) {
        int rebuild = nob_needs_rebuild1(stb_headers[i].output, stb_headers[i].input);
        if (rebuild < 0) return false;
        if (rebuild){
            compile_common(cmd);
            cmd_append(cmd, "-o", stb_headers[i].output);
            cmd_append(cmd, "-x", "c");
            cmd_append(cmd, "-c");
            if (stb_headers[i].macro) cmd_append(cmd, stb_headers[i].macro);
            cmd_append(cmd, stb_headers[i].input);
            if (!nob_cmd_run(cmd, .async = procs)) return false;
        } else {
            nob_log(INFO, "%s up to data", stb_headers[i].output);
        }
    }

    if (!procs_flush(procs)) return false;

    return true;
}

static Nob_Cmd cmd = {0};
static Nob_Procs procs = {0};

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    if (!mkdir_if_not_exists(BUILD_FOLDER)) return 1;
    if (!mkdir_if_not_exists(BUILD_FOLDER"fonts/")) return 1;

    if (!rebuild_includes(&cmd, &procs)) return 1;

    // exit(69);

    // ttf2c compile
    compile_common(&cmd);
    cmd_append(&cmd, "-o", BUILD_FOLDER"ttf2c");
    cmd_append(&cmd, SRC_BUILD_FOLDER"ttf2c.c");
    cmd_append(&cmd, BUILD_FOLDER"stb_truetype.o");
    cmd_append(&cmd, BUILD_FOLDER"stb_image_write.o");
    cmd_append(&cmd, BUILD_FOLDER"flag.o");
    cmd_append(&cmd, "-lm");
    if (!nob_cmd_run(&cmd)) return 1;
    nob_log(NOB_INFO, "Compliled: ./build/ttf2c");

    // ttf2c run
    cmd_append(&cmd, BUILD_FOLDER"ttf2c");
    cmd_append(&cmd, "-i", ASSETS_FOLDER"fonts/JetBrainsMonoNerdFont-Regular.ttf");
    cmd_append(&cmd, "-o", BUILD_FOLDER"fonts/JetBrainsMonoNerdFont_Regular.h");
    cmd_append(&cmd, "-n", "jetbrainsmono_regular");
    nob_log(NOB_INFO, "Running: ./build/ttf2c");
    if (!nob_cmd_run(&cmd)) return 1;
    nob_log(NOB_INFO, "Ran: ./build/ttf2c");

    // main compile
    compile_common(&cmd);
    cmd_cflags_x11(&cmd);
    cmd_cflags_pa(&cmd);
    cmd_append(&cmd, "-o", BUILD_FOLDER"main");
    cmd_append(&cmd, SRC_FOLDER"main.c");
    cmd_append(&cmd, SRC_FOLDER"game.c");
    cmd_libs_x11(&cmd);
    cmd_libs_pa(&cmd);
    cmd_append(&cmd, "-lm");
    cmd_framework(&cmd);
    if (!nob_cmd_run(&cmd)) return 1;
    nob_log(NOB_INFO, "Compliled: ./build/main");

    // // main run
    // cmd_append(&cmd, "./build/main");
    // nob_log(NOB_INFO, "Running: ./build/main");
    // if (!nob_cmd_run(&cmd)) return 1;
    // nob_log(NOB_INFO, "Ran: ./build/main");

    return 0;
}
