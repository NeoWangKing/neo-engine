// #include <stdio.h>
// #include <stdint.h>
// #include <stdlib.h>
// #include <stdbool.h>
#include <ctype.h>
#include <time.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"
#include "game.h"

#define OLIVEC_IMPLEMENTATION
#include "olive.c"

static int warping = 0;

int main(void)
{
    Game game = game_init();

    printf("game.target_fps        = %zu\n", game.target_fps);
    printf("game.display_width     = %zu\n", game.display_width);
    printf("game.display_height    = %zu\n", game.display_height);
    printf("game.audio_sample_rate = %zu\n", game.audio_sample_rate);
    printf("game.audio_channels    = %zu\n", game.audio_channels);

    pa_sample_spec ss = {
        .format = PA_SAMPLE_S16LE,
        .rate = game.audio_sample_rate,
        .channels = game.audio_channels,
    };

    pa_buffer_attr ba = {
        .maxlength = -1,
        .tlength   = 131072,
        .prebuf    = -1,
        .minreq    = -1,
    };

    int error = 0;
    pa_simple *s = pa_simple_new(
            NULL,
            "The Game",
            PA_STREAM_PLAYBACK,
            NULL,
            "audio",
            &ss,
            NULL,
            &ba,
            &error);
    if (s == NULL) {
        fprintf(stderr, "ERROR: pa_simple_new() failed: %s\n", pa_strerror(error));
        return 1;
    }

    Display *display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "ERROR: could not open the default display\n");
        return 1;
    }

    printf("display = %p\n", display);

    Window window = XCreateSimpleWindow(
            display,
            XDefaultRootWindow(display),
            0, 0,
            game.display_width, game.display_height,
            0,
            0,
            0);

    printf("window = %lu\n", window);

    XWindowAttributes wa = {0};
    XGetWindowAttributes(display, window, &wa);

    XImage *image = XCreateImage(
            display,
            wa.visual,
            wa.depth,
            ZPixmap,
            0,
            (char *) game.display,
            game.display_width,
            game.display_height,
            32,
            game.display_width*sizeof(*game.display));

    printf("image = %p\n", image);

    GC gc = XCreateGC(display, window, 0, NULL);

    printf("gc = %p\n", gc);

    Atom wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &wm_delete_window, 1);

    XSelectInput(display, window, KeyPressMask | KeyReleaseMask | PointerMotionMask);

    XStoreName(display, window, "The Game");
    XMapWindow(display, window);
    XDefineCursor(display, window, None);
    XWarpPointer(display, None, window, 0, 0, 0, 0, game.display_width/2, game.display_height/2);
    XGrabPointer(display, window, True, PointerMotionMask, GrabModeAsync, GrabModeAsync, window, None, CurrentTime);

    uint64_t delta_time = NANOS_PER_SEC/game.target_fps;

    bool quit = false;
    while (!quit) {
        uint64_t begin = nanos_since_unspecified_epoch();
        while (XPending(display) > 0) {
            XEvent event = {0};
            XNextEvent(display, &event);
            switch (event.type) {
                case KeyPress:
                    {
                        KeySym key = XLookupKeysym(&event.xkey, 0);
                        if (key >= 'A' && key <= 'Z') key = tolower(key);
                        game_key_down(key);
                        break;
                    }

                case KeyRelease:
                    {
                        KeySym key = XLookupKeysym(&event.xkey, 0);
                        if (key >= 'A' && key <= 'Z') key = tolower(key);
                        game_key_up(key);
                        break;
                    }

                case MotionNotify:
                    {
                        // // 如果是 warp 产生的移动，忽略
                        // if (warping) {
                        //     warping = 0;
                        //     break;
                        // }
                        // int x = event.xmotion.x;
                        // int y = event.xmotion.y;
                        // int cx = game.display_width / 2;
                        // int cy = game.display_height / 2;
                        // // 计算相对于窗口中心的偏移量
                        // int dx = x - cx;
                        // int dy = y - cy;
                        // // 存入 controls（注意 game.controls 是指针）
                        // game.controls->mouse_dx = dx;
                        // game.controls->mouse_dy = dy;
                        // // 将鼠标 warp 回窗口中心
                        // XWarpPointer(display, None, window, 0, 0, 0, 0, cx, cy);
                        // warping = 1;
                        break;
                    }

                case ClientMessage:
                    {
                        if (event.xclient.message_type == XInternAtom(display, "WM_PROTOCOLS", True) &&
                                (Atom)event.xclient.data.l[0] == wm_delete_window) {
                            quit = true;
                        }
                        break;
                    }

                default: {}
            }
        }

        game_update();

        XPutImage(display, window, gc, image, 0, 0, 0, 0, game.display_width, game.display_height);
        error = 0;
        size_t audio_size_in_bytes = game.audio_sample_rate/game.target_fps*game.audio_channels*sizeof(*game.audio);
        pa_simple_write(s, game.audio, audio_size_in_bytes, &error);

        uint64_t end = nanos_since_unspecified_epoch();
        uint64_t delta = end - begin;

        if (delta < delta_time) {
            struct timespec ts = { .tv_sec = 0, .tv_nsec = (delta_time - delta), };
            nanosleep(&ts, NULL);
        }
    }
    return 0;
}
