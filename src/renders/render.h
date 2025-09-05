#ifndef RENDER_H_
#define RENDER_H_

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3/SDL_video.h>

//as a test for myself, i am going to deviate from qdbp's architecture and go
//out of my way to avoid singletons. to render an image, i'm going to
//have to refactor how images are stored and handled, and how they access
//the renderer to be drawn.

//perhaps i will make the renderer hidden-ish behind an image handler, and 
//expose that to the engine instead?

//sdl_rendercopyex is what's used to render, don't forget! there's examples
//in qdbp's image.cpp

class render {
public:
  render();
  render(const render &) = delete;
  render &operator=(const render &) = delete;
  ~render();

  void init();

  SDL_Renderer *get_r() { return r; };

  int get_w() const { return w_width; }
  int get_h() const { return w_height; }

  //"render" was already taken by the class name and "draw" doesn't fit, so
  //the function that updates the window is just going to be called "draw".
  //note that after updating, the screen is wiped.
  void show();

private:

  SDL_Window *w;
  SDL_Renderer *r;

  int w_width, w_height;

  void init_w();
  void init_r();

  uint8_t c_r, c_g, c_b;

};

#endif
