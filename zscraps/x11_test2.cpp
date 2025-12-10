#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <iostream>
#include <unistd.h>

Window o_window;

int main(void) {

  

  //get the main display from the X server. the "display" is the set of screens
  //linked to a single user with a single mouse and single keyboard.
  Display *d = NULL;
  d = XOpenDisplay(NULL); //NULL for main display (POSIX-only)
  if(d == NULL) {
    std::cout << "S-H-I-T! default display was null!" << std::endl;
    return -1;
  }

  o_window = XDefaultRootWindow(d); //either the window we caught or the root window

  int f_return;
  Window f_window;

  unsigned int child_count = 0;
  Window *child_windows;
  Window r_w_r;         //holds root window
  Window p_w_r;         //holds parent window
 
  int status = 0;
  XTextProperty t;
  std::string search_str = "FreeTube";
  std::string search_result = "";


  int pcount = 0;

  while(true) {
    if(!o_window || o_window == XDefaultRootWindow(d)) {
      //no window found yet
      std::cout << pcount++ << " searching..." << std::endl;

      search_result = "";
      XGetInputFocus(d, &f_window, &f_return);
      bool quit=false;
      while(!quit) {
        //get the children and parent of this window
        XQueryTree(d, f_window, &r_w_r, &p_w_r, &child_windows, &child_count);
        //check the name of this window
        status = XGetTextProperty(d, f_window, &t, XA_WM_NAME);

        if(status) {
          //this window is named!
          //need to extract the value now
          char **list_return;
          int count_return = 0;
          //this LOOKS like it's leaking but it's actually fine - the memory is still reachable,
          //doesn't grow with time, etc. it just wasn't freed by x11
          XmbTextPropertyToTextList(d, &t, &list_return, &count_return);
          
          search_result = "";
          for(int j=0; j<count_return; j++) {
            search_result = list_return[j] + search_result;
          }
          if(search_result.find(search_str) != std::string::npos) {
            //window title matches
            o_window = f_window;
            quit = true;
          }
          
          XFreeStringList(list_return);
        }
        //root window never seems to be named 
        if(f_window == r_w_r) {
          quit = true;
        }
        //now jump up one window to the parent and do it again
        f_window = p_w_r;

        //free the used memory
    //    XFree(name);
        XFree(t.value);
        XFree(child_windows);
   
 
      }
    }
    else {
      status = XGetTextProperty(d, o_window, &t, XA_WM_NAME);
      if(status) {
        //this window is named!
        //need to extract the value now
        char **list_return;
        int count_return = 0;
        //this LOOKS like it's leaking but it's actually fine - the memory is still reachable,
        //doesn't grow with time, etc. it just wasn't freed by x11
        XmbTextPropertyToTextList(d, &t, &list_return, &count_return);
        
        search_result = "";
        for(int j=0; j<count_return; j++) {
          search_result = list_return[j] + search_result;
        }
        if(search_result.find(search_str) != std::string::npos) {
          //window title matches
          o_window = f_window;
        }
        std::cout << pcount++ << " found: " << search_result << std::endl;
        
        XFreeStringList(list_return);
      }
      XFree(t.value);
    }

    usleep(40000);
  }


  XCloseDisplay(d);
  return 0;
}
