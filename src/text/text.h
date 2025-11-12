#ifndef TEXT_H_
#define TEXT_H_

/*
the "text" class serves as a convenient way to create text on the screen. it
is not a TTF wrapper; i'm using pngs and image manipulation to give myself
greater control over how the text is displayed.

due to the surprisingly low-level control needed to manipulate a FIFO in a
non-blocking manner, there's a fair amount of C syntax in the implementation.
can't be helped, unfortunately! as a result, the input strings that come
through the fifo MUST BE NULL-TERMINATED. strings are also capped at 255
characters, but that's easy enough to change; the better question is, will i
ever make a string that long?

the display area for a given text is rectangular: an x,y for the tlc, and a
width,height for the dimensions. it can be scaled, but (for now) can only be
pinned to the window.
TODO: eventually allow this to operate like a dollpart (and pin to one). 

the text is drawn from a FIFO pipe, the name of which is determined on 
initialization of an instance of the class. 

the entire font is controlled by one single document which dictates the size
and animation (if any) of every letter in the set. nonexistent characters are
rendered as a "missing texture". 

the text object checks the FIFO when it is updated, and will continue to 
display the same message until a new one becomes available.
TODO: cool stuff like message glitching/decay
*/

#include <string>
#include <iostream>
#include <map>
#include "src/renders/render.h"

class text {
public:
  ~text();
  text() = delete;
  text(
      const std::string fi_name
    , const std::string fo_name
    , int x
    , int y
    , int w
    , int h
    , render *r
  );

  void update();
  void draw() const;
  std::string get_message() const { return message; }
private:
  const std::string fifo_path_base;
  const std::string fifo_path;

  const std::string font_path_base;
  const std::string font_path;

  //how much data to read at once
  const static int buf_size = 255;
  //buffer to read from
  char buf[buf_size + 1] = { 0 };
  //actual message
  std::string message;

  int pipe;
  bool pipe_open;

  int tlc_x, tlc_y, box_w, box_h;


  //due to the unique nature of the spritesheet and how it's drawn, i am
  //choosing NOT to use an instance of the image class here. text will host
  //its own textures. 
  std::string ss_file_path;
  int ss_frames, ss_fps, ss_width, ss_height, ltr_width, ltr_height;
  SDL_Texture *t;
  render *r;
  bool ss_loaded;
};

#endif
