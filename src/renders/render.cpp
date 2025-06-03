#include "src/renders/render.h"

//init SDL video and png image here
render::render() : 
    w(NULL)
  , r(NULL)
  , w_width(640)
  , w_height(480)
  , c_r(0x0A)
  , c_g(0xAA)
  , c_b(0xFF) 
{
  if(SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
    //throw an error and die
  }

  if(!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
    //throw an error and die
  }

  init_w();
  init_r();

  //set window size and height
  //TODO: this, later. init_w() locks it to hardcoded values
}

render::~render() {
  SDL_DestroyWindow(w);
  SDL_DestroyRenderer(r);
}

void render::init_w() { 
  w = SDL_CreateWindow("p_lite_pngtuber", 0, 0, 640, 480, SDL_WINDOW_SHOWN);
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
  
  //now, wipe the screen
  //SDL_Surface *ws = SDL_GetWindowSurface(w);
  //SDL_FillRect(ws, NULL, SDL_MapRGB(ws->format, c_r, c_g, c_b));
  //SDL_UpdateWindowSurface(w);
  return;
}
