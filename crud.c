#include <err.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <X11/extensions/shape.h>

#include "config.h"

enum Cursors {
  Normal,
  Crosshair
};

typedef struct {
  int x;
  int y;
  int width;
  int height;
} Selection;

typedef struct {
  Selection selection;
  Window window;
} WindowSelection;

// static values, these are initialized in setup.
static Cursor cursor[Crosshair + 1];
static Display* display = NULL;
static int screen = 0;
static Window root;

void setup() {
  display = XOpenDisplay(NULL);

  if (!display) {
    printf("Failed to open the display.\n");
    exit(1);
  }

  cursor[Normal] = XCreateFontCursor(display, XC_left_ptr);
  cursor[Crosshair] = XCreateFontCursor(display, XC_crosshair);

  screen = DefaultScreen(display);
  root = RootWindow(display, screen);
}

void preparetodie() {
  XFreeCursor(display, cursor[0]);
  XFreeCursor(display, cursor[1]);

  if (display != NULL)
    XCloseDisplay(display);
}

void switch_cursor(Cursor* cursor) {
  int mask = PointerMotionMask | ButtonPressMask | ButtonReleaseMask;

  XGrabPointer(display, root, True,
               mask, GrabModeAsync, GrabModeAsync,
               None, *cursor, CurrentTime);
}

WindowSelection make_selection(Selection dimensions) {
  XSetWindowAttributes attributes;
  unsigned long value_mask = CWBackPixel | CWOverrideRedirect | CWEventMask;
  Screen* sscreen = ScreenOfDisplay(display, DefaultScreen(display));

  attributes.background_pixel = FOREGROUND_COLOR;
  attributes.override_redirect = true;
  attributes.event_mask = StructureNotifyMask;

  Window window =
    XCreateWindow(display, root, 0, 0, WidthOfScreen(sscreen), HeightOfScreen(sscreen),
                  0, CopyFromParent, InputOutput, CopyFromParent, value_mask, &attributes);

  XRectangle rects[4];

  rects[0].x = dimensions.x - BORDER;
  rects[0].y = dimensions.y - BORDER;
  rects[0].width = BORDER;
  rects[0].height = dimensions.height + BORDER * 2;

  rects[1].x = dimensions.x;
  rects[1].y = dimensions.y - BORDER;
  rects[1].width = dimensions.width + BORDER;
  rects[1].height = BORDER;

  rects[2].x = dimensions.x + dimensions.width;
  rects[2].y = dimensions.y - BORDER;
  rects[2].width = BORDER;
  rects[2].height = dimensions.height + BORDER * 2;

  rects[3].x = dimensions.x;
  rects[3].y = dimensions.y + dimensions.height;
  rects[3].width = dimensions.width + BORDER;
  rects[3].height = BORDER;

  XShapeCombineRectangles(display, window, ShapeBounding, 0, 0, rects, 4, ShapeSet, 0);

  XRectangle rect;

  rect.x = rect.y = rect.width = rect.height = 0;

  XShapeCombineRectangles(display, window, ShapeInput, 0, 0, &rect, 1, ShapeSet, 0);

  XMapWindow(display, window);

  return (WindowSelection) {
    .selection = dimensions,
    .window    = window
  };
}

int destroy_check(Display* display, XEvent* ev, XPointer win) {
  return ev->type == DestroyNotify && ev->xdestroywindow.window == *((Window*)win);
}

void destroy_selection(WindowSelection* sel) {
  XEvent ev;

  XSetWindowBackground(display, sel->window, 0);
  XClearWindow(display, sel->window);
  XDestroyWindow(display, sel->window);
  XIfEvent(display, &ev, &destroy_check, (XPointer)&sel->window);
}

void set_selection(WindowSelection* sel, Selection dimensions) {
  XRectangle rects[4];

  rects[0].x = dimensions.x - BORDER;
  rects[0].y = dimensions.y - BORDER;
  rects[0].width = BORDER;
  rects[0].height = dimensions.height + BORDER * 2;

  rects[1].x = dimensions.x;
  rects[1].y = dimensions.y - BORDER;
  rects[1].width = dimensions.width + BORDER;
  rects[1].height = BORDER;

  rects[2].x = dimensions.x + dimensions.width;
  rects[2].y = dimensions.y - BORDER;
  rects[2].width = BORDER;
  rects[2].height = dimensions.height + BORDER * 2;

  rects[3].x = dimensions.x;
  rects[3].y = dimensions.y + dimensions.height;
  rects[3].width = dimensions.width + BORDER;
  rects[3].height = BORDER;

  sel->selection = dimensions;

  XShapeCombineRectangles(display, sel->window, ShapeBounding, 0, 0, rects, 4, ShapeSet, 0);
}

int main(int argc, char **argv) {
#ifdef __OpenBSD__
  if (pledge("stdio rpath unix prot_exec", NULL) == -1)
    err(1, "pledge");
#endif

  setup();

  // make a selection and make it as small as possible
  WindowSelection ws = make_selection((Selection) {
    .x = 0,
    .y = 0,
    .width = 0,
    .height = 0
  });

  bool done = false;
  bool button_state = false;
  int x = 0, y = 0, width = 0, height = 0;
  int start_x = 0, start_y = 0;
  XEvent event;

  switch_cursor(&cursor[1]);

  XGrabKeyboard(display, root, GrabModeSync, GrabModeAsync, True, CurrentTime);
  
  while (!done) {
    XNextEvent(display, &event);
    switch (event.type) {
    case KeyPress:
      if (XLookupKeysym(&event.xkey, 0) == XK_Escape) return 1;
      break;
      
    case ButtonPress:
      if (event.xbutton.button == Button3)
        return 1;
      button_state = true;
      x = start_x = event.xbutton.x_root;
      y = start_y = event.xbutton.y_root;
      width = height = 0;
      break;

    case MotionNotify:
      if (button_state) {
        set_selection(&ws, (Selection) {
          .x = x,
          .y = y,
          .width = width,
          .height = height
        });

        x = event.xbutton.x_root;
        y = event.xbutton.y_root;

        if (x > start_x) {
          width = x - start_x;
          x = start_x;
        }

        else {
          width = start_x - x;
        }

        if (y > start_y) {
          height = y - start_y;
          y = start_y;
        }

        else {
          height = start_y - y;
        }

        set_selection(&ws, (Selection) {
          .x = x,
          .y = y,
          .width = width,
          .height = height
        });
      }

      break;

    case ButtonRelease:
      if (event.xbutton.x_root == start_x && event.xbutton.y_root == start_y) {
        done = true;
        button_state = false;

        Window hroot, hovered;
        int rx, ry, hx, hy;
        unsigned int m;

        if (XQueryPointer(display, root, &hroot, &hovered, &rx, &ry, &hx, &hy, &m)) {
          XWindowAttributes attrs;
          XGetWindowAttributes(display, hovered, &attrs);
          // update with the selected window's attributes
          set_selection(&ws, (Selection) {
            .x = attrs.x,
            .y = attrs.y,
            .width = attrs.width,
            .height = attrs.height
          });
        }
      }

      else {
        done = true;
        button_state = false;
      }
      break;

    default:
      break;
    }
  }

  destroy_selection(&ws);

  XUngrabKeyboard(display, CurrentTime);
  XUngrabPointer(display, CurrentTime);
  XSync(display, 1);

  Selection* result = &ws.selection;

  printf("W=%d\nH=%d\nX=%d\nY=%d\n", result->width, result->height, result->x, result->y);
  printf("G=%dx%d+%d+%d\n", result->width, result->height, result->x, result->y);

  XFlush(display);

  preparetodie();

  return 0;
}
