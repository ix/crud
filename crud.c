#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#include <X11/Xlib.h>
#include <X11/cursorfont.h>

#include "config.h"

enum Cursors {
  Normal,
  Crosshair
};

typedef struct {
  int x;
  int y;
  int w;
  int h;
} Selection;

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

Selection perform_selection () {
  int x = 0, y = 0;
  int width = 0, height = 0;
  int start_x = 0, start_y = 0;
  XEvent event;
  GC sel_gc;
  XGCValues sel_gv;
  bool done = false;
  bool button_state = false;

  // grab the region data from X11
  switch_cursor(&cursor[1]);

  sel_gv.function = GXxor;
  sel_gv.subwindow_mode = IncludeInferiors;
  sel_gv.line_width = BORDER;
  sel_gv.foreground = FOREGROUND_COLOR;
  sel_gc = XCreateGC(display, root, GCFunction | GCSubwindowMode | GCForeground | GCLineWidth, &sel_gv);

  while (!done) {
    XNextEvent(display, &event);

    switch (event.type) {
    case ButtonPress:
      button_state = true;
      x = start_x = event.xbutton.x_root;
      y = start_y = event.xbutton.y_root;
      width = height = 0;
      break;

    case MotionNotify:
      if (button_state) {
        XDrawRectangle(display, root, sel_gc, x, y, width, height);

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

        XDrawRectangle(display, root, sel_gc, x, y, width, height);
        XFlush(display);
      }
      break;

    case ButtonRelease:
      done = true;
      button_state = false;
      break;

    default:
      break;
    }
  }

  XDrawRectangle(display, root, sel_gc, x, y, width, height);
  XFlush(display);

  XUngrabPointer(display, CurrentTime);
  XFreeGC(display, sel_gc);
  XSync(display, 1);

  return (Selection) {
    .x = x,
    .y = y,
    .w = width,
    .h = height,
  };
}

int main(int argc, char **argv) {
  Selection drawn;
  setup();

  drawn = perform_selection();

  printf("W=%d\nH=%d\nX=%d\nY=%d\n", drawn.w, drawn.h, drawn.x, drawn.y);
  printf("G=%dx%d+%d+%d\n", drawn.w, drawn.h, drawn.x, drawn.y);

  XFlush(display);

  preparetodie();

  // hopefully this is enough time for the rectangles to disappear
  // when drawing them over surfaces with fast redraws (videos spring to mind)
  usleep(50000);

  return 0;
}
