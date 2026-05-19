#define NOB_IMPLEMENTATION
#include "include/nob.h"

#define BUILD_FOLDER "build/"
#define SRC_FOLDER   "src/"

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

void cmd_build(Nob_Cmd *cmd)
{
    cmd_append(cmd, "-o", BUILD_FOLDER"main");
}

void cmd_src(Nob_Cmd *cmd)
{
    cmd_append(cmd, SRC_FOLDER"main.c");
    cmd_append(cmd, SRC_FOLDER"game.c");
}

void cmd_framework(Nob_Cmd *cmd)
{
    nob_cmd_append(cmd, "-framework", "CoreVideo");
    nob_cmd_append(cmd, "-framework", "Cocoa");
    nob_cmd_append(cmd, "-framework", "IOKit");
}

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    Nob_Cmd cmd = {0};
    cmd_append(&cmd, "clang");
    cmd_append(&cmd, "-Wall", "-Wextra", "-g");

    cmd_append(&cmd, "-Iinclude");
    cmd_cflags_x11(&cmd);
    cmd_cflags_pa(&cmd);

    cmd_build(&cmd);
    cmd_src(&cmd);

    cmd_libs_x11(&cmd);
    cmd_libs_pa(&cmd);
    cmd_append(&cmd, "-lm");
    cmd_framework(&cmd);

    if (!nob_cmd_run_sync(cmd)) {
        nob_log(NOB_ERROR, "编译失败！");
        return 1;
    }
    nob_log(NOB_INFO, "编译成功 → ./build/main");

    // cmd = (Nob_Cmd){0};
    //
    // cmd_append(&cmd, "./build/main");
    //
    // if (!nob_cmd_run_sync(cmd)) {
    //     nob_log(NOB_ERROR, "运行失败！");
    //     return 1;
    // }
    // nob_log(NOB_INFO, "运行成功： ./build/main");

    return 0;
}
