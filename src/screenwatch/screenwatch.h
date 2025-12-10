#ifndef SCREENWATCH_H_
#define SCREENWATCH_H_

#include <X11/Xlib.h>
#include <string>
#include <vector>
#include <map>

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
  void get_focused_screen_title(std::string *);

  //searches the tree for a window whose title includes the given string,
  //and then replaces the string with that window's complete title.
  //this is NOT expecting a pointer, it wants a reference.
  //TODO: delete? prone to causing double free errors when using
  //XQueryTree and XFree (see zscraps/x11_test3.cpp)
  void get_screen_title_from_partial_title(std::string *);


  //given a vector of strings, check to see if the string appears anywhere in
  //the title of the currently focused window. returns 1 on true, 0 on false.
  //(case sensitive)
  float check_titles(std::vector<std::string> &);

  //given a string, starts monitoring the first focused window which uses
  //that string in its title. a monitored window will have any changes to its
  //title tracked. screenwatch will search for a new window if the monitored
  //one is closed.
  void start_monitoring_screen(const std::string &);
  //given a string to look up, checks to see if there is a window being
  //monitored under that string, and updates the passed string with its
  //value. 
  //returns 0 on success, 1 if no window with that name is being monitored,
  //and 2 if the window is being monitored, but has not yet been found (or was
  //closed and hasn't been reopened yet).
  int check_monitored_screen(std::string *);

  //updates all monitored screens. 
  void update();

private:
  //given a window, get its title. 
  //note that some windows don't have titled - these are usually child windows
  //of some named parent. as far as i've seen, the chain of windows back to 
  //the root only ever has one window with an actual title, so for now, i'll 
  //operate under that assumption.
  //this function expects a reference to the window and to the string, and
  //modifies both: it walks back from the given Window until it finds a named
  //one, then puts its title in the string and the window's id in the
  //variable. 
  //primary use case tends to revolve around getting the title of the focused
  //window; secondary use case is checking a specific window for its title.
  void get_window_title(Window *, std::string *) const;

  Display *d; //links to default display on init
  
  std::map<std::string, Window> monitored_windows;
};

#endif
