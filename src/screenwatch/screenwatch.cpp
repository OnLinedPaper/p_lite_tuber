#include "src/screenwatch/screenwatch.h"
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xproto.h>
#include <iostream>

screenwatch::screenwatch() :
    d(XOpenDisplay(NULL))
{ 
  XSetErrorHandler([](Display *d, XErrorEvent *e) -> int { 
      //this anon function is designed to specifically catch BadWindow errors,
      //which can arise if the focused window is closed after being captured
      //but before its properties are assessed.
      //any other error shows up, and it prints info, then aborts.
      
      //no-op to squash "unused variable"
      if(d) { ;; }

      if(e && e->type == 0) {
        //not sure why this pops up sometimes, but 0 is apparently "No error",
        //so i'm ignoring it.
        return e->type;
      }
      else if(e && e->type == BadWindow) {
        //ignorable in the context i'm using it - tried to get data from a 
        //focused window after it closed.
        std::cerr << "screenwatch caught BadWindow - ignoring." << std::endl; 
        return e->type; 
      }
      else {
        //ok, something ACTUALLY went wrong. print the error and abort.
        char c[2000];
        int len = 2000;
        XGetErrorText(d, e->type, c, len);
        std::cerr << "S-H-I-T! unhandled error " << e->type;
        std::cerr << " in screenwatch! info: " << c << std::endl;
        std::abort();
      }
  });
  //XSetErrorHandler(NULL);
}

screenwatch::~screenwatch() {
  XCloseDisplay(d);
}

void screenwatch::get_focused_screen_data(Window *w, std::string *s) {
  Window w_in;
  int f_return = 0;
  XGetInputFocus(d, &w_in, &f_return);

  get_titled_window_data(&w_in, s);
  //check to see if they want to save the data or not
  if(w != NULL) { *w = w_in; }
}

void screenwatch::get_titled_window_data(Window *w, std::string *s) {
  if(w == NULL) {
    if(s != NULL) {
      *s = "ALERT: Window passed was NULL.";
    }
    return;
  }
  //dummy variables
  unsigned int child_count = 0;
  //Window *child_windows;
  Window *child_windows;

  int status = 0;       //indicates whether window was named
  Window r_w_r = 0;     //holds root window
  Window p_w_r = 0;     //holds parent window
  Window f_window = 0;  //holds focused window

  //this will hold the window names
  XTextProperty t;

  *s = "";

  //get the focused window
  //XGetInputFocus(d, &f_window, &f_return);
  f_window = *w;
  //one-time init for edge cases where the focused window is immediately closed
  r_w_r = XDefaultRootWindow(d);

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
      //*s = "]" + *s;
      if(s != NULL) {
        for(int i=0; i<count_return; i++) {
          *s = list_return[i] + *s;
        }
        //*s = " -> [" + *s;
      }

      //also, return the window
      if(w != NULL) {
        *w = f_window;
      }

      //free the memory
      XFreeStringList(list_return);

      quit = true;
    }

    //root window never seems to be named, so no "else if" needed here
    if(!f_window) {
      //window was closed
      //this line technically could be called before r_w_r is initialized;
      //in practice, the following line trivializes the error. more
      //pertinently, attempting to "fix" it somehow breaks it worse. weird.
      f_window = r_w_r;
    }
    if(f_window == r_w_r) {
      //*s = "[root]" + *s;
      if(s != NULL) { *s = ""; }
      if(w != NULL) { *w = r_w_r; }
      quit = true;
    }
    else {
      //*s = " -> [none]" + *s;
    }

    //now, jump up one window to the parent and do it again
    XQueryTree(d, f_window, &r_w_r, &p_w_r, &child_windows, &child_count);
    if(f_window == p_w_r) {
      //this window somehow was its own parent. hell if i know how this is
      //possible, but cut it off here.
      f_window = r_w_r;
    } else {
      f_window = p_w_r;
    }

    //free the used memory
    XFree(t.value);

    if(child_count != 0) { XFree(child_windows); }
  }
}

float screenwatch::check_titles(std::vector<std::string> &titles) {
  //get current window's title
  std::string focused_title = "";
  this->get_focused_screen_data(NULL, &focused_title);

  //check for a match
  for(std::string title : titles) {
    //std::cout << focused_title << " | " << title << std::endl;
    if(focused_title.find(title) != std::string::npos) { return 1.0; }
  }

  return 0.0;
}

void screenwatch::start_monitoring_screen(const std::string &str) {
  //don't monitor more than one window with the same name
  if(monitored_windows.find(str) != monitored_windows.end()) { return; }

  //insert as the root window
  long unsigned int w = XDefaultRootWindow(d);
  std::string s = str;
  monitored_windows.insert(std::make_pair(s, w));
}

int screenwatch::check_monitored_screen(std::string *s) {
  //return failure if window is not being monitored
  if(monitored_windows.find(*s) == monitored_windows.end()) { 
    *s = "";
    return 1; 
  }

  Window w = monitored_windows.find(*s)->second;

  //does this window have a title yet?
  if(w == XDefaultRootWindow(d)) {
    //no title yet, try to find one
    std::string str = "IN";
    get_focused_screen_data(&w, &str);

    //return failure if the monitored window has no title (yet)
    if(w != XDefaultRootWindow(d) && str.find(*s) != str.npos) {
      //found a window! upate the map
      monitored_windows[*s] = w;
    }
    else {
      *s = "";
      return 2;
    }
  }

  get_titled_window_data(&w, s);
  if(w == XDefaultRootWindow(d)) {
    monitored_windows[*s] = XDefaultRootWindow(d);
    *s = "";
    return 1;
  }

  return 0;
}

