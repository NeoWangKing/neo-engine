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
        cmd_append(cmd, "-framework", "CoreFoundation");
        cmd_append(cmd, "-framework", "CoreGraphics");
        cmd_append(cmd, "-framework", "CoreVideo");
        cmd_append(cmd, "-framework", "Cocoa");
        cmd_append(cmd, "-framework", "IOKit");
        cmd_append(cmd, "-framework", "OpenGL");
    }

    //raylib
#define RAYLIB_INCLUDE "-I/Users/neowang/opt/raylib/include"
#define RAYLIB_LIB     "-L/Users/neowang/opt/raylib/lib"
    void cmd_cflags_raylib(Nob_Cmd *cmd) {
        cmd_append(cmd, RAYLIB_INCLUDE);
    }
    void cmd_libs_raylib(Nob_Cmd *cmd) {
        cmd_append(cmd, RAYLIB_LIB);
        cmd_append(cmd, "-Xlinker", "-multiply_defined", "-Xlinker", "suppress");
        cmd_append(cmd, "-lraylib");
        cmd_append(cmd, "-lm");
    }
#elif defined(__linux__)
    void cmd_cflags_x11(Nob_Cmd *cmd) { (void) cmd; }

    void cmd_libs_x11(Nob_Cmd *cmd)
    {
        cmd_append(cmd, "-lX11");
        cmd_append(cmd, "-lXext"); 
    }

    void cmd_cflags_pa(Nob_Cmd *cmd) { (void) cmd; }

    void cmd_libs_pa(Nob_Cmd *cmd)
    {
        cmd_append(cmd, "-lpulse");
        cmd_append(cmd, "-lpulse-simple");
        cmd_append(cmd, "-lpulse-mainloop-glib");
        cmd_append(cmd, "-lglib-2.0");
        cmd_append(cmd, "-pthread");
    }

    void cmd_framework(Nob_Cmd *cmd) { (void) cmd; }

    //raylib
#define RAYLIB_INCLUDE "-I/Users/neowang/opt/raylib/include"
#define RAYLIB_LIB     "-L/Users/neowang/opt/raylib/lib"
    void cmd_cflags_raylib(Nob_Cmd *cmd) {
        cmd_append(cmd, RAYLIB_INCLUDE);
    }
    void cmd_libs_raylib(Nob_Cmd *cmd) {
        cmd_append(cmd, RAYLIB_LIB);
        cmd_append(cmd, "-lraylib");
        cmd_append(cmd, "-Wl,-allow-multiple-definition");
        cmd_append(cmd, "-lpthread");
        cmd_append(cmd, "-ldl");
        cmd_append(cmd, "-lrt");
        cmd_append(cmd, "-lm");
    }
#elif defined(_WIN32)
    void cmd_cflags_x11(Nob_Cmd *cmd) { (void) cmd; }
    void cmd_libs_x11(Nob_Cmd *cmd) { (void) cmd; }
    void cmd_cflags_pa(Nob_Cmd *cmd) { (void) cmd; }
    void cmd_libs_pa(Nob_Cmd *cmd) { (void) cmd; }
    void cmd_framework(Nob_Cmd *cmd) { (void) cmd; }

    //raylib
#define RAYLIB_INCLUDE "-IC:/raylib/include"
#define RAYLIB_LIB     "-LC:/raylib/lib"
    void cmd_cflags_raylib(Nob_Cmd *cmd) {
        cmd_append(cmd, RAYLIB_INCLUDE);
    }
    void cmd_libs_raylib(Nob_Cmd *cmd) {
        cmd_append(cmd, RAYLIB_LIB);
        cmd_append(cmd, "-lraylib");
        cmd_append(cmd, "-lopengl32");
        cmd_append(cmd, "-lgdi32");
        cmd_append(cmd, "-lwinmm");
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
    cmd_append(cmd, "-I"SRC_FOLDER);
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
            .output = BUILD_FOLDER"includes/stb_truetype.o",
            .macro = "-DSTB_TRUETYPE_IMPLEMENTATION",
        },
        {
            .input = INCLUDE_FOLDER"stb_image_write.h",
            .output = BUILD_FOLDER"includes/stb_image_write.o",
            .macro = "-DSTB_IMAGE_WRITE_IMPLEMENTATION",
        },
        {
            .input = INCLUDE_FOLDER"stb_image.h",
            .output = BUILD_FOLDER"includes/stb_image.o",
            .macro = "-DSTB_IMAGE_IMPLEMENTATION",
        },
        {
            .input = INCLUDE_FOLDER"flag.h",
            .output = BUILD_FOLDER"includes/flag.o",
            .macro = "-DFLAG_IMPLEMENTATION",
        },
        {
            .input = INCLUDE_FOLDER"stb_vorbis.c",
            .output = BUILD_FOLDER"includes/stb_vorbis.o",
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
            nob_log(INFO, "headers: %s up to data", stb_headers[i].output);
        }
    }

    if (!procs_flush(procs)) return false;

    return true;
}

bool rebuild_tools(Nob_Cmd *cmd, Nob_Procs *procs)
{
    struct {
        const char *name;
        const char *src;
        const char *output;
        const char *deps[4];
        const char *extra_libs;
    } tools[] = {
        {
            .name = "ttf2c",
            .src = SRC_BUILD_FOLDER "ttf2c.c",
            .output = BUILD_FOLDER "tools/ttf2c",
            .deps = {
                BUILD_FOLDER "includes/stb_truetype.o",
                BUILD_FOLDER "includes/stb_image_write.o",
                BUILD_FOLDER "includes/flag.o",
                NULL
            },
            .extra_libs = "-lm",
        },
        {
            .name = "obj2c",
            .src = SRC_BUILD_FOLDER "obj2c.c",
            .output = BUILD_FOLDER "tools/obj2c",
            .deps = {
                BUILD_FOLDER "includes/flag.o",
                NULL
            },
            .extra_libs = "-lm",
        },
        {
            .name = "png2c",
            .src = SRC_BUILD_FOLDER "png2c.c",
            .output = BUILD_FOLDER "tools/png2c",
            .deps = {
                BUILD_FOLDER "includes/flag.o",
                BUILD_FOLDER "includes/stb_image.o",
                NULL
            },
            .extra_libs = "-lm",
        },
    };

    for (size_t i = 0; i < ARRAY_LEN(tools); ++i) {
        int rebuild = nob_needs_rebuild1(tools[i].output, tools[i].src);
        if (rebuild < 0) return false;
        if (rebuild) {
            compile_common(cmd);
            cmd_append(cmd, "-o", tools[i].output);
            cmd_append(cmd, tools[i].src);
            for (size_t d = 0; tools[i].deps[d] != NULL; ++d) {
                cmd_append(cmd, tools[i].deps[d]);
            }
            if (tools[i].extra_libs) {
                cmd_append(cmd, tools[i].extra_libs);
            }
            if (!nob_cmd_run(cmd, .async = procs)) return false;
            nob_log(INFO, "Compiled: %s", tools[i].output);
        } else {
            nob_log(INFO, "tools: %s up to date", tools[i].output);
        }
    }
    if (!procs_flush(procs)) return false;
    return true;
}

bool run_tools(Nob_Cmd *cmd, Nob_Procs *procs)
{
    static struct {
        const char *name;
        const char *tools;
        const char *src_dir;
        const char *output_dir;
        const char *file_type;
    } tools[] = {
        {
            .name = "ttf2c",
            .tools = BUILD_FOLDER"tools/ttf2c",
            .src_dir = ASSETS_FOLDER"fonts/",
            .output_dir = BUILD_FOLDER"fonts/",
            .file_type = ".ttf",
        },
        {
            .name = "obj2c",
            .tools = BUILD_FOLDER"tools/obj2c",
            .src_dir = ASSETS_FOLDER"models/",
            .output_dir = BUILD_FOLDER"models/",
            .file_type = ".obj",
        },
        {
            .name = "png2c",
            .tools = BUILD_FOLDER"tools/png2c",
            .src_dir = ASSETS_FOLDER"images/",
            .output_dir = BUILD_FOLDER"images/",
            .file_type = ".png",
        },
    };

    for (size_t i = 0; i < ARRAY_LEN(tools); ++i) {
        if (nob_file_exists(tools[i].src_dir)) {
            Nob_File_Paths files = {0};
            if (!nob_read_entire_dir(tools[i].src_dir, &files)) return false;

            for (size_t j = 0; j < files.count; ++j) {
                const char *file_name = files.items[j];
                const char *ext = strrchr(file_name, '.');
                if (!ext || strcmp(ext, tools[i].file_type) != 0) continue;

                char *input_path = nob_temp_sprintf("%s/%s", tools[i].src_dir, file_name);

                char base_name[256];
                strncpy(base_name, file_name, sizeof(base_name));
                base_name[sizeof(base_name)-1] = '\0';
                char *dot = strrchr(base_name, '.');
                if (dot) *dot = '\0';
                for (char *p = base_name; *p; ++p) {
                    if (!isalnum(*p)) *p = '_';
                }

                char *output_path = nob_temp_sprintf("%s%s.h", tools[i].output_dir, base_name);

                int rebuild = nob_needs_rebuild1(output_path, input_path);
                if (rebuild < 0) return false;
                if (!rebuild) {
                    nob_log(INFO, "%s: %s up to date", tools[i].name, output_path);
                    continue;
                }

                cmd->count = 0;
                cmd_append(cmd, tools[i].tools);
                cmd_append(cmd, "-i", input_path);
                cmd_append(cmd, "-o", output_path);
                cmd_append(cmd, "-n", base_name);
                if (!nob_cmd_run(cmd, .async = procs)) return false;
                nob_log(INFO, "Generated: %s", output_path);
            }
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
    if (!mkdir_if_not_exists(BUILD_FOLDER"includes/")) return 1;
    if (!mkdir_if_not_exists(BUILD_FOLDER"tools/")) return 1;
    if (!mkdir_if_not_exists(BUILD_FOLDER"models/")) return 1;

    if (!rebuild_includes(&cmd, &procs)) return 1;
    if (!rebuild_tools(&cmd, &procs)) return 1;
    if (!run_tools(&cmd, &procs)) return 1;

    // main compile
    compile_common(&cmd);
    cmd_cflags_x11(&cmd);
    cmd_cflags_pa(&cmd);
    cmd_append(&cmd, "-o", BUILD_FOLDER"game.x11pa");
    cmd_append(&cmd, SRC_FOLDER"platform_X11Pulse.c");
    cmd_append(&cmd, SRC_FOLDER"game.c");
    cmd_libs_x11(&cmd);
    cmd_libs_pa(&cmd);
    cmd_append(&cmd, "-lm");
    cmd_framework(&cmd);
    if (!nob_cmd_run(&cmd)) return 1;
    nob_log(NOB_INFO, "Compliled: ./build/game.x11pa");

    // compile_common(&cmd);
    // cmd_cflags_raylib(&cmd);
    // cmd_append(&cmd, "-DSTB_VORBIS_HEADER_ONLY");
    // cmd_append(&cmd, "-o", BUILD_FOLDER"game.rl");
    // cmd_append(&cmd, "-I/Users/neowang/opt/raylib/include");
    // cmd_append(&cmd, SRC_FOLDER"platform_raylib.c");
    // cmd_append(&cmd, SRC_FOLDER"game.c");
    // cmd_libs_raylib(&cmd);
    // cmd_append(&cmd, "-lm");
    // cmd_framework(&cmd);
    // if (!nob_cmd_run(&cmd)) return 1;
    // nob_log(NOB_INFO, "Compliled: ./build/game.rl");

    // // main run
    // cmd_append(&cmd, "./build/main");
    // nob_log(NOB_INFO, "Running: ./build/main");
    // if (!nob_cmd_run(&cmd)) return 1;
    // nob_log(NOB_INFO, "Ran: ./build/main");

    return 0;
}
