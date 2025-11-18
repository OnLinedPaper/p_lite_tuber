#include "./src/text/text.h"
#include <filesystem>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <fstream>
#include "src/time/time.h"
#include <cstdlib>

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
  , printme("")
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


  //printme = message;
  if(printme == message) { return; }

  scramble();
}

void text::scramble() {
  //text scramble effect
  const int scc = 26; //scramble char count
  char scramble_chars[scc] = { '!', '#', '$', '%', '&', '*', '+', '-', '/', '\\', '<', '=', '>', '?', '[', ']', '_', '_', '_', '_', '_', '_', '_', '_', '{', '}', };
  int scramble_intensity = 10;
  int keep_intensity = 7;

  long unsigned int i=0;    
  do {
//  for(long unsigned int i=0; i<printme.size(); i++) {
    /*compare each letter of printme to each letter of message:
    - if they match, do nothing
    - if they do not match, either
      - fix: change the character to its actual value (0)
      - keep: do not change the character (scramble_intensity - 1)
      - scramble: change the character to a scrambled one (any other value)
    - if printme is shorter than message, add a new scrambled character on "fix"
    - if printme is longer than message, delete the last character on "fix"*/


    const int FIX = 0;
    const int KEEP = 1;
    const int SCRAMBLE = 2;
    int action = -1;

    //my god, using std::rand()?! the code is not cryptographically secure!!
    int roll = std::rand() % scramble_intensity; 
    action = SCRAMBLE;
    if(roll > scramble_intensity - keep_intensity) { action = KEEP; }
    if(roll == 0) { action = FIX; }


    //checks for max string size
    if(message.size() > 0 && printme.size() > message.size() && i >= message.size()) {
      if(action == FIX) {
        //printme is longer than message, delete one character and stop
        int chars_to_remove = std::max((std::rand() % (printme.size() - message.size()))/3, (size_t)1);
        printme.erase(i, chars_to_remove);
        continue;
      }
    }
    else if (message.size() > 0 && printme.size() < message.size() && i+1 >= printme.size()) {
      if(action == FIX || action == SCRAMBLE) {
        //printme is shorter than message, add one character and stop
        int chars_to_add = std::max(std::rand() % (message.size() - printme.size())/3, (size_t)1);
        for(int j = 0; j < chars_to_add; j++) {
          printme.append(" ");
        }
        continue;
      }
    }

    if(message.size() != printme.size() && action == FIX) { action = KEEP; }

    //checks for normal characters in a string
    if(printme[i] == message[i]) { i++; continue; }
    if(action == FIX) { printme[i] = message[i]; i++; continue; }
    if(action == KEEP) { i++; continue; }
    if(action == SCRAMBLE) { printme[i] = scramble_chars[std::rand() % scc]; }

    i++;
  } while(i < printme.size());

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
  float spacing_horiz = 0.8;
  float spacing_verti = 0.8;

  int c_x = 0; //horizontal displacement per letter
  int c_y = 0; //vertical displacement per newline
  int total_printed = -1;
  int since_newline = -1;
  size_t chars_per_box = box_w / (ltr_width * scale * spacing_horiz);
  size_t wordlen = 0;

  for(char c : printme) {
    total_printed++;
    since_newline++;
    //first, some checks to see if it's a special char
    if(c == ' ') { wordlen = 0; c_x++; continue; }
    if(c == '\n') { c_x = 0; c_y++; since_newline = 0; continue; }
    if((int)c < 32 || (int)c > 126) { c = ' '; }

    //(re)calculate length of next word
    if(wordlen == 0)
    {
      wordlen = printme.find(' ', total_printed);
      if(wordlen == printme.npos) { wordlen = printme.size() - total_printed; }
      else { wordlen = wordlen - total_printed; }
    }
    //calculate the position the end of the word will land at
    size_t wordend = printme.find(' ', total_printed);
    if(wordend == printme.npos) { wordend = printme.size() - 1; }
    else { wordend--; }

    //if it WOULD go past the end, start a new line, unless it's longer than
    //an entire line
    if(
          (since_newline + (wordend - total_printed) >= chars_per_box
      &&  wordend - total_printed < chars_per_box
      && wordlen < chars_per_box)
      ||  (since_newline == (int)chars_per_box && wordlen > chars_per_box)
    ) {
      c_x = 0;
      c_y++;
      since_newline = 0;
    } 

    //TODO: bottom of the box, at some point...?

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

