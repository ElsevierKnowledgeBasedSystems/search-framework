#include "type.hpp"
#include <iostream>
//#include <vector>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <cairo-xlib.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <unistd.h>

//This function should give us a new x11 surface to draw on.
cairo_surface_t *create_x11_surface(Display *d, int *x, int *y) {
    Drawable da;
    int screen;
    cairo_surface_t *sfc;
    Screen *scr;

    screen = DefaultScreen(d);
    scr = DefaultScreenOfDisplay(d);
    if (!*x || !*y) {
        *x = WidthOfScreen(scr) / 2;
        *y = HeightOfScreen(scr) / 2;
        da =
            XCreateSimpleWindow(d, DefaultRootWindow(d), 0, 0, *x, *y, 0, 0, 0);
    } else
        da =
            XCreateSimpleWindow(d, DefaultRootWindow(d), 0, 0, *x, *y, 0, 0, 0);

    XSelectInput(d, da, ButtonPressMask | ButtonReleaseMask | KeyPressMask |
                            ButtonMotionMask | StructureNotifyMask);

    // http://www.lemoda.net/c/xlib-wmclose/index.html
    /* "wm_delete_window" is the Atom which corresponds to the delete
           window message sent by the window manager. */
    Atom wm_delete_window;
    wm_delete_window = XInternAtom(d, "WM_DELETE_WINDOW", False);
    /* Set up the window manager protocols. The third argument here is
       meant to be an array, and the fourth argument is the size of
       the array. */

    XSetWMProtocols(d, da, &wm_delete_window, 1);
    XMapWindow(d, da);

    sfc = cairo_xlib_surface_create(d, da, DefaultVisual(d, screen), *x, *y);

    return sfc;
}

struct Drawer {
    Drawer() {
        d = XOpenDisplay(NULL);
        if (d == NULL) {
            fprintf(stderr, "Failed to open display\n");
            exit(-1);
        }
        // create a new cairo surface in an x11 window as well as a cairo_t* to
        // draw on the x11 window with.
        int x=500, y=500;
        surface = create_x11_surface(d, &x, &y);
        cr = cairo_create(surface);
    }

    // http://stackoverflow.com/a/19308254/2725810
    ~Drawer() {
        cairo_destroy(cr);
        cairo_surface_destroy(surface);
        XCloseDisplay(d);
    }

    // Returns true if need to continue or false if quiting
    bool processEvents() {
        XEvent e;

        if (XPending(cairo_xlib_surface_get_display(surface))) {
            XNextEvent(cairo_xlib_surface_get_display(surface), &e);
            switch (e.type) {
            case ButtonPress:
                drag_start_x = e.xbutton.x;
                drag_start_y = e.xbutton.y;
                break;
            case ButtonRelease:
                last_delta_x = 0;
                last_delta_y = 0;
                break;
            case MotionNotify:
                // http://cairographics.org/manual/cairo-Transformations.html#cairo-translate
                cairo_translate(cr, e.xmotion.x - drag_start_x - last_delta_x,
                                e.xmotion.y - drag_start_y - last_delta_y);
                last_delta_x = e.xmotion.x - drag_start_x;
                last_delta_y = e.xmotion.y - drag_start_y;
                break;
            case ConfigureNotify:
                cairo_xlib_surface_set_size(surface, e.xconfigure.width,
                                            e.xconfigure.height);
                break;
            case ClientMessage:
                return false;
            default:
                fprintf(stderr, "Dropping unhandled XEevent.type = %d.\n",
                        e.type);
            }
        }
        return true;
    }

    void draw() {
        cairo_push_group(cr);

        // Clear the background
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_paint(cr);

        cairo_set_source_rgb(cr, 0, 1, 0);
        cairo_move_to(cr, 0, 0);
        cairo_line_to(cr, 256, 256);
        cairo_move_to(cr, 256, 0);
        cairo_line_to(cr, 0, 256);
        cairo_set_line_width(cr, 10.0);
        cairo_stroke(cr);

        cairo_pop_group_to_source(cr);
        cairo_paint(cr);
        cairo_surface_flush(surface);
        XFlush(d);
    }

    void run() {
        while (1) {
            if (!processEvents()) break;
            draw();
            sleep(0.1);
        }
    }

private:
    Display *d;
    cairo_surface_t* surface;
    cairo_t* cr;
    int last_delta_x = 0, last_delta_y = 0;
    int drag_start_x, drag_start_y;
};

int main() {
    Drawer d;
    d.run();
    //std::cout << demangle(typeid(A<decltype(x)>).name()) << std::endl;
    return 0;
}
