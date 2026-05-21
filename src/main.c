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
#include "game.h"
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#define OLIVEC_IMPLEMENTATION
#include "olive.c"

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
        .maxlength = -1,           // 自动
        .tlength   = 131072,        // 目标缓冲区大小，建议 32768 或 65536
        .prebuf    = -1,           // 自动
        .minreq    = -1,           // 自动
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

                        if (key == XK_Escape) {
                            quit = true;
                            break;
                        }

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

        uint64_t end = nanos_since_unspecified_epoch();

        uint64_t delta = end - begin;

        if (delta < delta_time) {
            struct timespec ts = {
                .tv_sec = 0,
                .tv_nsec = (delta_time - delta),
            };
            nanosleep(&ts, NULL);
        }

        end = nanos_since_unspecified_epoch();

        delta = end - begin;
        int FPS = 1.0/((double)delta/NANOS_PER_SEC);

        Olivec_Canvas oc = {
            .pixels = (uint32_t*)game.display,
            .width = game.display_width,
            .height = game.display_height,
            .stride = game.display_width,
        };

        float font_size = 3;
        olivec_text(oc, temp_sprintf("FPS = %d", FPS), 10, 10, olivec_default_font, font_size, 0xFFAAAAFF);

        XPutImage(display, window, gc, image, 0, 0, 0, 0, game.display_width, game.display_height);
        error = 0;
        size_t audio_size_in_bytes = game.audio_sample_rate/game.target_fps*game.audio_channels*sizeof(*game.audio);
        pa_simple_write(s, game.audio, audio_size_in_bytes, &error);
    }

    return 0;
}
