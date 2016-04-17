#ifndef VISUALIZATION_UTILITIES
#define VISUALIZATION_UTILITIES

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <cairo-xlib.h>
#include "Colors.h"

cairo_surface_t *create_x11_surface(Display *, Window &, Window &, int *,
                                    int *);

struct Graphics {
    Graphics() {
        /* Prepare the surface for drawer */
        display = XOpenDisplay(NULL);
        if (display == NULL) {
            fprintf(stderr, "Failed to open display\n");
            exit(-1);
        }
        // create a new cairo surface in an x11 window as well as a cairo_t* to
        // draw on the x11 window with.
        surface =
            create_x11_surface(display, w, root, &windowXSize, &windowYSize);
        cr = cairo_create(surface);
    }

    ~Graphics() {
        // http://stackoverflow.com/a/19308254/2725810
        cairo_destroy(cr);
        cairo_surface_destroy(surface);
        XCloseDisplay(display);
        cairo_debug_reset_static_data();
    }
    Display *display{};
    Window w, root;
    cairo_surface_t* surface{};
    cairo_t* cr{};

    int windowXSize = 500, windowYSize = 500;
    double zeroX = 0.0, // beginning of drawing in device coordinates
           zeroY = 0.0; // at last re-drawing
    double margin = 1.0;
    double scale = 1.0;
};

struct PatternLock {
    PatternLock(Graphics &g) : g_(g) {
        p_ = cairo_get_source(g_.cr);
        cairo_pattern_reference(p_);
    }

    ~PatternLock() {
        cairo_set_source(g_.cr, p_); // this does not affect the reference count
        cairo_pattern_destroy(p_); // safe! can still paint!
        // std::cerr << cairo_pattern_get_reference_count (p_) << std::endl;
        cairo_paint(g_.cr);
    }

private:
    Graphics &g_;
    cairo_pattern_t *p_;
};

// translation since the last re-drawing
void deltaTranslate(Graphics &g, double &deltaX, double &deltaY,
                    bool absFlag = true) {
    double curZeroX = 0.0, curZeroY = 0.0;
    cairo_user_to_device(g.cr, &curZeroX, &curZeroY);
    if (absFlag) {
        deltaX = fabs(curZeroX - g.zeroX);
        deltaY = fabs(curZeroY - g.zeroY);
    }
    else {
        deltaX = curZeroX - g.zeroX;
        deltaY = curZeroY - g.zeroY;
    }
}
// Set origin to the current origin, so deltas are 0.
void resetOrigin(Graphics &g) {
    double curZeroX = 0.0, curZeroY = 0.0;
    cairo_user_to_device(g.cr, &curZeroX, &curZeroY);
    g.zeroX = curZeroX;
    g.zeroY = curZeroY;
}

double deltaTranslateX(Graphics &g) {
    double deltaX, deltaY;
    deltaTranslate(g, deltaX, deltaY);
    return deltaX;
}
double deltaTranslateY(Graphics &g) {
    double deltaX, deltaY;
    deltaTranslate(g, deltaX, deltaY);
    return deltaY;
}

bool redraw(Graphics &g) {
    double deltaX, deltaY;
    deltaTranslate(g, deltaX, deltaY);
    if (deltaX >= g.margin * g.windowXSize ||
        deltaY >= g.margin * g.windowYSize)
        return true;
    return false;
}

// Everything drawn between the execution of the constructor and the destructor
// is in a group. This drawing is appended to the current pattern.
struct GroupLock {
    GroupLock(bool flag, Graphics &g, bool clearFlag = false)
        : g_(g), flag_(flag) {
        if (!flag_) return;

        virtualTranslate(); // Translate to enable margins
                              // that are not clipped

        {
            PatternLock lock{g_};    // The constructor saves source as pattern.
                                     // The destructor will:
                                     //    1. Put the saved source
                                     //       as a source inside the group.
                                     //    2. Paint from the source into
                                     //       the temporary surface.
            cairo_push_group(g_.cr); // Re-direct all subsequent drawing into
                                     // temporary surface
            (void)lock;
        }

        if (clearFlag) {
            cairo_set_source_rgb(g_.cr, 0, 0, 0);
            cairo_paint(g_.cr);
        }
    }

    ~GroupLock() {
        if (!flag_) return;
        cairo_pop_group_to_source(g_.cr); // Get the image from the
                                          // temporary surface into the source.
        {
            PatternLock lock{g_};
            (void)lock;        // See above. Only painting
                               // into the real surface
            realTranslate(); // Translating it back
        }
    }

private:
    Graphics &g_;
    bool flag_;
    cairo_pattern_t *p_;
    double zeroX_, zeroY_;
    double origDeltaX_, origDeltaY_;

    void virtualTranslate() {
        deltaTranslate(g_, origDeltaX_, origDeltaY_, false);
        cairo_translate(g_.cr,
                        (-origDeltaX_ + g_.margin * g_.windowXSize) / g_.scale,
                        (-origDeltaY_ + g_.margin * g_.windowYSize) / g_.scale);
    }
    void realTranslate() {
        cairo_translate(g_.cr,
                        (origDeltaX_ - g_.margin * g_.windowXSize) / g_.scale,
                        (origDeltaY_ - g_.margin * g_.windowYSize) / g_.scale);
    }
};

struct RGB {
    static double red(Color color) {
        return (double)(RGBValues[static_cast<int>(color)] >> 16)/255;
    }
    static double green(Color color) {
        return (double)((RGBValues[static_cast<int>(color)] >> 8) % 256)/255;
    }
    static double blue(Color color) {
        return (double)(RGBValues[static_cast<int>(color)] % 256)/255;
    }
};

enum class VertexShape {CIRCLE, SQUARE};
enum class VertexFillPattern {SOLID};

enum class EdgeShape {CIRCLE, SQUARE};
enum class EdgeDash {NONE};

struct VertexStyle {
    VertexShape shape = VertexShape();
    VertexFillPattern pattern = VertexFillPattern();
    static double sizeBase;
    double sizeFactor = 1.0;
    Color fillColor = Color::WARM_GREY;
    Color borderColor  = Color();
    double borderWidthFactor = 0;
    Color emphasisColor = Color();
    double emphasisWidthFactor = 0;
};
double VertexStyle::sizeBase = 10;

struct EdgeStyle {
    bool operator==(const EdgeStyle &rhs) {
        if (shape != rhs.shape) return false;
        if (dash != rhs.dash) return false;
        if (color != rhs.color) return false;
        if (widthFactor != rhs.widthFactor) return false;
        return true;
    };
    EdgeShape shape = EdgeShape();
    EdgeDash dash = EdgeDash();
    Color color = Color::BROWN_GREY;
    static double widthBase;
    double widthFactor = 1.0;
    bool arrow = false;
    int depth = 0;
    static constexpr int maxDepth = 2;
};
double EdgeStyle::widthBase = 5.0;

template<class State>
struct VertexChange {
    using StateSharedPtr = std::shared_ptr<const State>;
    StateSharedPtr s;
    VertexStyle now;
};

template<class State>
struct ArcChange {
    using StateSharedPtr = std::shared_ptr<const State>;
    StateSharedPtr from, to;
    EdgeStyle now;
};

template<class State>
struct VisualChanges {
    std::vector<VertexChange<State> > vChanges;
    std::vector<ArcChange<State> > aChanges;
};

template <class State> struct CurrentStyles {
    using StateSharedPtr = std::shared_ptr<const State>;
    using Graph = StateGraph<State>;
    using VertexDescriptor = typename Graph::VertexDescriptor;
    using EdgeDescriptor = typename Graph::EdgeDescriptor;

    VertexStyle get(StateSharedPtr s) const {
        return vertexStyles_.at(g_.vertex(s));
    }
    EdgeStyle get(StateSharedPtr from, StateSharedPtr to) const {
        return edgeStyles_.at(g_.edge(from, to));
    };
    CurrentStyles(const Graph &g) : g_(g) {}
protected:
    const Graph &g_;
    // -1 means no previous event
    std::unordered_map<VertexDescriptor, VertexStyle> vertexStyles_;

    // EdgeDescriptor cannot be a key for unordered_map
    std::map<EdgeDescriptor, EdgeStyle> edgeStyles_;
};

//This function should give us a new x11 surface to draw on.
cairo_surface_t *create_x11_surface(Display *d, Window &w, Window &root, int *x,
                                    int *y) {
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
                            ButtonMotionMask | PointerMotionMask |
                            ExposureMask | StructureNotifyMask);

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

    root = XDefaultRootWindow(d);
    int revert;
    XGetInputFocus(d, &w, &revert);

    return sfc;
}

// Function to create an X11 keyboard event
// http://www.doctort.org/adam/nerd-notes/x11-fake-keypress-event.html
XKeyEvent createKeyEvent(Display *display, Window &win, Window &winRoot,
                         bool press, int keycode, int keystate) {
    XKeyEvent event;

    event.display     = display;
    event.window      = win;
    event.root        = winRoot;
    event.subwindow   = None;
    event.time        = CurrentTime;
    event.x           = 1;
    event.y           = 1;
    event.x_root      = 1;
    event.y_root      = 1;
    event.same_screen = True;
    event.keycode     = XKeysymToKeycode(display, keycode);
    event.state       = keystate;

    if(press)
        event.type = KeyPress;
    else
        event.type = KeyRelease;

    return event;
}


#endif
