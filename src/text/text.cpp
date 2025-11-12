#include "./src/text/text.h"
#include <filesystem>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <fstream>
#include "src/time/time.h"

text::text(
    const std::string fi_name
  , const std::string fo_name
  , int x
  , int y
  , int w
  , int h
  , render *r
) :
    fifo_path_base("./resources/fifo/")
  , fifo_path(fifo_path_base + fi_name)
  , font_path_base("./resources/control/")
  , font_path(font_path_base + fo_name + ".txt")
  , message("")
  , pipe(-1)
  , pipe_open(false)
  , tlc_x(x)
  , tlc_y(y)
  , box_w(w)
  , box_h(h)
  , ss_file_path("")
  , ss_frames(-1)
  , ss_fps(-1)
  , ss_width(-1)
  , ss_height(-1)
  , ltr_width(-1)
  , ltr_height(-1)
  , r(r)
  , ss_loaded(false)
{ 
  //first, do the FIFO - this is what gets the message itself

  //check if fifo exists; link if it does, create otherwise
  if(!std::filesystem::is_fifo(fifo_path)) {
    if(mkfifo(fifo_path.c_str(), 0600) == -1) {
      //TODO: throw error here
      return;
    }
  }

  //open fifo in non-blocking mode
  pipe = open(fifo_path.c_str(), O_RDONLY | O_NONBLOCK, 0600);
  if(pipe == -1) { return; }

  //if we got this far, all should be good to go
  pipe_open = true;


  //next, load the spritesheet with the characters
  bool fail = false;
  std::ifstream f(font_path, std::ios::in);
  if(!f) { fail = true; }

  //load the spritesheet data
  if(!fail) {
    f >> ss_file_path;
    f >> ss_frames;
    f >> ss_fps;
    f >> ss_width;
    f >> ss_height;
    f >> ltr_width;
    f >> ltr_height;
  }
  f.close();

  //check to see if anything failed to load. TODO: reporting, later
  if(
        ss_file_path == ""
    ||  ss_frames == -1
    ||  ss_fps == -1
    ||  ss_width == -1
    ||  ss_height == -1
    ||  ltr_width == -1
    ||  ltr_height == -1
  ) { fail = true; }

  //check to see if any settings were bad
  if (
        ss_frames <= 0
    ||  ss_fps < 0
    ||  ss_width <= 0
    ||  ss_height <= 0
    ||  ltr_width <= 0
    || ltr_height <= 0
  ) { fail = true; }

  t = IMG_LoadTexture(r->get_r(), ss_file_path.c_str());
}

text::~text() {
  //close fifo
  SDL_DestroyTexture(t);
  close(pipe);
}

void text::update() {
  //don't read from empty pipe
  if(!pipe_open) { return; }

  //read from the buffer
  int bytes_read = 1;
  bool reading = false;

  //repeat until entire buffer is empty (for very long strings)
  while(bytes_read > 0) {
    bytes_read = read(pipe, buf, buf_size);
    
    if(bytes_read > 0) {
      //data was in the pipe! wipe it if this is our first read; else, append
      if(!reading) { message.clear(); reading = true; }
      //null-terminate the string
      buf[bytes_read] = '\0';
      message += buf;
    }
    //"clear" the buffer
    buf[0] = '\0';
  }
}

/*
the spritesheet for the letters goes from left-to-right for each printable
ASCII character, and top-to-bottom for different frames. 
it's 95 characters wide. the first character, SPACE, has ascii val 32, and the
last printable character, tilde, has ascii val 126.

each letter in the message is converted into its ascii value, looked up, and
printed. letters are printed starting at the top left corner of the given box,
and will indent when they reach the end.

TODO: tons of stuff. wordbreaks, scrolling text, you name it.
*/
void text::draw() const {
  float scale = 0.15;
  int c_x = 0; //horizontal displacement per letter
  int c_y = 0; //vertical displacement per newline

  float spacing_horiz = 0.8;
  float spacing_verti = 0.8;

  for(char c : message) {
    //first, some checks to see if it's a special char
    if(c == ' ') { c_x++; continue; }
    if(c == '\n') { c_x = 0; c_y++; continue; }

    //what part of window to render to
    SDL_FRect dest_r;
    dest_r.x = tlc_x + (c_x * ltr_width * scale * spacing_horiz);
    dest_r.y = tlc_y + (c_y * ltr_height * scale * spacing_verti);
    dest_r.w = ltr_width * scale;
    dest_r.h = ltr_height * scale;

    //what part of the texture to render from
    SDL_FRect src_r;
    src_r.x = ltr_width * ((int)c - 32); //offset to letter value
    src_r.y = 0;
    src_r.w = ltr_width;
    src_r.h = ltr_height;

    c_x++; //increment by one letter

    if(ss_fps > 0) {
      //multiframe (copied from image.cpp)
      int tpu = std::max(time::get().get_TPS() / ss_fps, 1);
      int frame_to_render = (time::get().get_tick() / tpu) % ss_frames;
      src_r.y = ltr_height * frame_to_render;
    }


    //dye the letters
    SDL_SetTextureColorMod(t, 84, 78, 93); //grey

    //draw them!
    SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawBlendMode(r->get_r(), SDL_BLENDMODE_BLEND);
    SDL_RenderTexture(r->get_r(), t, &src_r, &dest_r);
  }

  if(true) {
    //draw rect around every image
    SDL_SetRenderDrawColor(r->get_r(), 128,128,128,128);
    SDL_FRect boundary_r {(float)tlc_x, (float)tlc_y, (float)box_w, (float)box_h};
    SDL_RenderRect(r->get_r(), &boundary_r);
  }
}

