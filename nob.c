#define NOB_IMPLEMENTATION
#include "include/nob.h"

#define BUILD_FOLDER "build/"
#define SRC_FOLDER   "src/"
#define INCLUDE_FOLDER   "include/"

void cmd_cflags(Nob_Cmd *cmd)
{
    cmd_append(cmd, "-I/opt/homebrew/include");
}

void cmd_libs(Nob_Cmd *cmd)
{
    cmd_append(cmd, "-L/opt/homebrew/lib");
    // cmd_append(cmd, "-lpulse-simple");
    // cmd_append(cmd, "-lpulse");
    cmd_append(cmd, "-lSDL2");
    cmd_append(cmd, "-lm");
}

void cmd_build(Nob_Cmd *cmd)
{
    cmd_append(cmd, "-o", BUILD_FOLDER"main");
}

void cmd_src(Nob_Cmd *cmd)
{
    cmd_append(cmd, SRC_FOLDER"main.c");
    cmd_append(cmd, SRC_FOLDER"game.c");
    // cmd_append(cmd, INCLUDE_FOLDER"neovin.c");
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
    // cmd_append(&cmd, "-Wno-unused-function");
    // cmd_append(&cmd, "-Wno-unused-variable");

    cmd_append(&cmd, "-Iinclude");
    cmd_cflags(&cmd);

    cmd_build(&cmd);
    cmd_src(&cmd);

    cmd_libs(&cmd);
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
