#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <iostream>
#include <unistd.h>

int main(void) {

  //get the main display from the X server. the "display" is the set of screens
  //linked to a single user with a single mouse and single keyboard.
  Display *d = NULL;
  d = XOpenDisplay(NULL); //NULL for main display (POSIX-only)
  if(d == NULL) {
    std::cout << "S-H-I-T! default display was null!" << std::endl;
    return -1;
  }

  //get default screen data... just testing if display is ok
  std::cout << "using screen: " << XDefaultScreen(d) << std::endl;
  
  //"windows" are unique from "screens" - a window is an application, and a
  //screen is a piece of physical hardware, i.e. a monitor

  //(this section left for posterity)
  //get the root window from the main display. this window stretches across the
  //display, and covers all the screens. from here, we can determine which 
  //child window has focus.
  //DON'T DESTROY THIS WINDOW 
  //Window w = XDefaultRootWindow(d);


  //dummy variables - we don't use these, just need them for XQueryTree() and XGetInputFocus()
  unsigned int child_count = 0;
  Window *child_windows;
  int f_return;

  
  int status = 0;       //indicates whether window name was found
  Window r_w_r;         //holds root window
  Window p_w_r;         //holds parent window
  Window f_window;      //holds focused window
//  char *name = NULL;    //holds name of focused window
   
  //these will hold the window names
  XTextProperty t;
  std::string search_result = "";

  int i=0;
  while(i<2000) {

    //each cycle, get the focused window and "walk backwards" to the root,
    //recording the names along the way. hoping to search the name string
    //later to guess what i'm tabbed into.

    search_result = "";

    //get the focused window
    XGetInputFocus(d, &f_window, &f_return);

    bool quit = false;
    //this segment searches upwards starting from the focus window.
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
        
        search_result = "]" + search_result;
        for(int j=0; j<count_return; j++) {
          search_result = list_return[j] + search_result;
        }
        search_result = " -> [" + search_result;
        
        XFreeStringList(list_return);
      }

      //root window never seems to be named 
      if(f_window == r_w_r) {
        search_result = "[root]" + search_result;
        quit = true;
      }
      else {
        search_result = " -> [none]" + search_result;
      }
      //now jump up one window to the parent and do it again
      f_window = p_w_r;

      //free the used memory
  //    XFree(name);
      XFree(t.value);
      XFree(child_windows);
    }
 
    usleep(1000);
    std::cout << "\n\n\n\n\n\ncycle: " << i++ << "\n" << search_result << std::endl; //"                                                   \r" << std::flush; //" | child windows: " << child_count << " | focused window: " << (status ? name : "<nameless>") << "                               \r" << std::flush;
  }

  XCloseDisplay(d);
  return 0;
}
