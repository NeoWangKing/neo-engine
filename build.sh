#!/bin/sh

source ~/opt/raylib.env

FRAMEWORK="
  -framework CoreFoundation
  -framework CoreGraphics
  -framework CoreVideo
  -framework IOKit
  -framework Cocoa
  -framework OpenGL
"

set -xe

clang -Wall -Wextra -g \
  `pkg-config --cflags raylib` \
  -o build/main \
  src/main.c src/engine.c\
  `pkg-config --libs raylib` \
  $FRAMEWORK
