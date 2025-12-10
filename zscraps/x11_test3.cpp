#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <iostream>
#include <unistd.h>
#include <queue>


//plan: recursively search everything for given string, and return the first
//instance found.
int main(void) {
  //get the main display from the X server. the "display" is the set of screens
  //linked to a single user with a single mouse and single keyboard.
  Display *d = NULL;
  d = XOpenDisplay(NULL); //NULL for main display (POSIX-only)
  if(d == NULL) {
    std::cout << "S-H-I-T! default display was null!" << std::endl;
    return -1;
  }


  XSetErrorHandler([](Display *d, XErrorEvent *e) -> int {
    if(d) { ;; }
    if(e && e->type == 0) { return e->type; }
    else if(e && e->type == BadWindow) { std::cerr << "ignoring badwindow" << std::endl; return e->type; }
    else { 
      char c[2000];
      int len = 2000;
      XGetErrorText(d, e->type, c, len);
      std::cerr << "S-H-I-T! unhandled error " << e->type;
      std::cerr << " in screenwatch! info: " << c << std::endl;
      std::abort();
    }
  });
  

  std::string search_str = "FreeTube";
  std::string search_res = "";
  XTextProperty t;
  int status = 0;

  unsigned int child_count = 0;
  Window *child_windows = nullptr;
  Window r_w_r;         //dummy - holds parent window
  Window p_w_r;         //dummy - holds root window
 


  //queue of windows - NO RECURSION
  std::queue<Window> windows;

  Window found_w = XDefaultRootWindow(d);

  int count = 0;

  while(true) {
  bool quit=false;

  //the below function searches the entire tree for a window whose title
  //matches the search string, and then tracks said window, returning its
  //title. 
  //there's no easy way to avoid it: if a window opens or closes at exactly
  //the wrong time, i.e. after XQueryTree is called but before XFree is called,
  //this will crash with a double free or munmap error. there's no way to
  //mitigate it that i'm aware of, so instead, if it loses the window, it'll
  //only search once every couple seconds as opposed to as often as it can.
  //the likelihood this causes a crash is, i think, significantly lower 
  //(though still unfortunately nonzero).
  //WAIT!! BETTER IDEA. just scan the focused window every once in a while!
  if(count++ % 100 == 0) {
    std::cout << "searching... (" << count << ")";
    windows.push(XDefaultRootWindow(d));
    while(!windows.empty() && found_w == XDefaultRootWindow(d)) {

      //get next window
      Window curr_w = windows.front();

      
      //check window's title
      status = XGetTextProperty(d, curr_w, &t, XA_WM_NAME);
      if(status) {
        //named window!
        char **list_r = NULL;
        int count_r = 0;
        XmbTextPropertyToTextList(d, &t, &list_r, &count_r);
        search_res = "";
        for(int j=0; j<count_r; j++) {
          search_res = list_r[j] + search_res;
        }
        if(list_r) { XFreeStringList(list_r); }

        if(search_res.find(search_str) != std::string::npos) {
          //hit!!
          std::cout << " found!" << std::endl;
          //get rid of everything else
          while(!windows.empty()) { windows.pop(); }
          found_w = curr_w;
          quit = true;
        }
        
      }

    
      XFree(t.value);

      if(!quit) {
        //add its children to the queue
        XQueryTree(d, curr_w, &r_w_r, &p_w_r, &child_windows, &child_count);
        for(int i=0; i<child_count; i++) {
          windows.push(child_windows[i]);
        }
        
        //whats probably happening here is x deletes the window then reassigns
        //it elsewhere. no real way to catch this unfortunately.
        if(child_count != 0) { XFree(child_windows); }
      }

      //remove window from queue
      if(!windows.empty()) { windows.pop(); }
    }
    std::cout << std::endl;
  }

  if(found_w != XDefaultRootWindow(d)) {
    status = XGetTextProperty(d, found_w, &t, XA_WM_NAME);
    if(status) {
      //named window!
      char **list_r = NULL;
      int count_r = 0;
      XmbTextPropertyToTextList(d, &t, &list_r, &count_r);
      search_res = "";
      for(int j=0; j<count_r; j++) {
        search_res = list_r[j] + search_res;
      }
      if(list_r) { XFreeStringList(list_r); }

      std::cout << search_res << std::endl;
    }
    else {
      //window must've closed - start looking again
      std::cout << "lost the window." << std::endl;
      found_w = XDefaultRootWindow(d);
    }

    XFree(t.value);
  }

  usleep(40000);
  }

  XCloseDisplay(d);

  return 0;
}
