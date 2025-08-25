#ifndef SCREENWATCH_H_
#define SCREENWATCH_H_

#include <X11/Xlib.h>
#include <string>

/*
this class uses x11 to determine which screen is focused. this can then be
used as an input to an action, i.e. as a way to switch opacity between two
different dolls for two different activities.
*/
class screenwatch {
public:
  screenwatch();
  ~screenwatch();

  //checks which window is focused, determines its title, and repeats this
  //backwards until reaching the root window.
  //this is NOT expecting a pointer, it wants a reference.
  void get_screen_title(std::string *);

private:
  Display *d; //links to default display on init
  

};

#endif
