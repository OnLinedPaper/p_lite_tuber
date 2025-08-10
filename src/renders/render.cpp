#include "src/renders/render.h"
//#include <iostream>

//init SDL video and png image here
render::render() : 
    w(NULL)
  , r(NULL)
  , w_width(960)
  , w_height(720)
  , c_r(0x0A)
  , c_g(0xAA)
  , c_b(0xFF) 
{
  //init_w();
  //init_r();

  //set window size and height
  //TODO: this, later. init_w() locks it to hardcoded values
}

render::~render() {
  SDL_DestroyWindow(w);
  SDL_DestroyRenderer(r);
}

void render::init() {
  init_w();
  init_r();
}

void render::init_w() { 

  SDL_DisplayMode m;
  SDL_zero(m);
  int e = SDL_GetDesktopDisplayMode(0, &m);
  if(e != 0) { 
    //throw error and die 
    //std::cout << SDL_GetError(); 
  }

  //stick it in the bottom right corner
  w = SDL_CreateWindow(
      "p_lite_pngtuber"
    , m.w - w_width
    , m.h - w_height
    , w_width
    , w_height
    , SDL_WINDOW_SHOWN
  );
  if(w == NULL) {
    //throw an error and die
  }
}

void render::init_r() { 
  SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");

  r = SDL_CreateRenderer(w, -1, SDL_RENDERER_ACCELERATED);
  if(r == NULL) {
    //throw an error and die
  }
}

void render::show() {
  
  SDL_RenderPresent(r);
  
  SDL_SetRenderDrawColor(r, c_r, c_g, c_b, 0xFF);
  SDL_RenderClear(r);
  //now, wipe the screen
  //SDL_Surface *ws = SDL_GetWindowSurface(w);
  //SDL_FillRect(ws, NULL, SDL_MapRGB(ws->format, c_r, c_g, c_b));
  //SDL_UpdateWindowSurface(w);
  return;
}
