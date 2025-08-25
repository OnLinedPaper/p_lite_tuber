#include "src/screenwatch/screenwatch.h"
#include <X11/Xutil.h>
#include <X11/Xatom.h>

screenwatch::screenwatch() :
    d(XOpenDisplay(NULL))
{ }

screenwatch::~screenwatch() {
  XCloseDisplay(d);
}

void screenwatch::get_screen_title(std::string *s) {
  //dummy variables
  unsigned int child_count = 0;
  Window *child_windows;
  int f_return = 0;

  int status = 0;       //indicates whether window was named
  Window r_w_r;         //holds root window
  Window p_w_r;         //holds parent window
  Window f_window;      //holds focused window

  //this will hold the window names
  XTextProperty t;

  *s = "";

  //get the focused window
  XGetInputFocus(d, &f_window, &f_return);

  bool quit = false;
  while(!quit) {
    //check to see if this window is named
    status = XGetTextProperty(d, f_window, &t, XA_WM_NAME);

    if(status) {
      //this window is named! extract the value
      char **list_return = NULL;
      int count_return = 0;

      //(for future me: this doesn't leak)
      XmbTextPropertyToTextList(d, &t, &list_return, &count_return);

      //log and assemble the returned strings
      *s = "]" + *s;
      for(int i=0; i<count_return; i++) {
        *s = list_return[i] + *s;
      }
      *s = " -> [" + *s;

      //free the memory
      XFreeStringList(list_return);
    }

    //root window never seems to be named, so no "else if" needed here
    if(f_window == r_w_r) {
      *s = "[root]" + *s;
      quit = true;
    }
    else {
      *s = " -> [none]" + *s;
    }

    //now, jump up one window to the parent and do it again
    XQueryTree(d, f_window, &r_w_r, &p_w_r, &child_windows, &child_count);
    f_window = p_w_r;

    //free the used memory
    XFree(t.value);
    XFree(child_windows);
  }
}
