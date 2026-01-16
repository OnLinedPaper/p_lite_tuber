#include "src/renders/render.h"
#include <iostream>

//init SDL video and png image here
render::render() : 
    w(NULL)
  , r(NULL)
  , w_width(1920)
  , w_height(900)
  , c_r(0x0A)
  , c_g(0xAA)
  , c_b(0xFF) 
{ }

render::~render() {
  SDL_DestroyWindow(w);
  SDL_DestroyRenderer(r);
}

void render::init() {
  init_w();
  init_r();
}

void render::init_w() { 

  int num_displays;
  SDL_DisplayID *displays = SDL_GetDisplays(&num_displays);
  for(int i=0; i<num_displays; i++) {
    //std::cout << displays[i] << std::endl;
  }
  const SDL_DisplayMode *m = SDL_GetCurrentDisplayMode(displays[0]);
  if(m == nullptr) { 
    //throw error and die 
    std::cout << SDL_GetError() << std::endl; 
    std::abort();
  }

  //stick it in the bottom right corner
  w = SDL_CreateWindow(
      "p_lite_pngtuber"
    , w_width
    , w_height
    , 0
  );
  if(w == NULL) {
    //throw an error and die
  }
  SDL_SetWindowPosition(w, m->w - w_width, m->h - w_height);

  SDL_free(displays);
}

void render::init_r() { 
  SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");

  r = SDL_CreateRenderer(w, NULL);
  if(r == NULL) {
    //throw an error and die
  }
  
  //SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_ADD);
  SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
}

void render::show() {
  
  SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
  SDL_RenderPresent(r);
  
  SDL_SetRenderDrawColor(r, c_r, c_g, c_b, 0xFF);
  SDL_RenderClear(r);
  //now, wipe the screen
  //SDL_Surface *ws = SDL_GetWindowSurface(w);
  //SDL_FillRect(ws, NULL, SDL_MapRGB(ws->format, c_r, c_g, c_b));
  //SDL_UpdateWindowSurface(w);
  return;
}
