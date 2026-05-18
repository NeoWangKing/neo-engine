#define NOB_IMPLEMENTATION
#include "include/nob.h"

#define BUILD_FOLDER "build/"
#define SRC_FOLDER   "src/"

void cmd_build(Nob_Cmd *cmd)
{
    cmd_append(cmd, BUILD_FOLDER"main");
}

void cmd_src(Nob_Cmd *cmd)
{
    cmd_append(cmd, SRC_FOLDER"main.c");
    cmd_append(cmd, SRC_FOLDER"engine.c");
}

void cmd_framework(Nob_Cmd *cmd)
{
  cmd_append(cmd, "-framework", "CoreFoundation");
  cmd_append(cmd, "-framework", "CoreGraphics");
  cmd_append(cmd, "-framework", "CoreVideo");
  cmd_append(cmd, "-framework", "IOKit");
  cmd_append(cmd, "-framework", "Cocoa");
  cmd_append(cmd, "-framework", "OpenGL");
}

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    Nob_Cmd cmd = {0};

    cmd_append(&cmd, "cc");
    cmd_append(&cmd, "-Wall", "-Wextra", "-g");

    // raylib include
    cmd_append(&cmd, "-I/Users/neowang/opt/raylib/include");

    cmd_append(&cmd, "-o", BUILD_FOLDER"main");

    cmd_append(&cmd, SRC_FOLDER"main.c");
    cmd_append(&cmd, SRC_FOLDER"engine.c");

    // raylib lib
    cmd_append(&cmd, "-L/Users/neowang/opt/raylib/lib");
    cmd_append(&cmd, "-lraylib");

    cmd_framework(&cmd);

    if (!nob_cmd_run(&cmd)) {
        nob_log(NOB_ERROR, "编译失败！");
        return 1;
    }
    return 0;
}
