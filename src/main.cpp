#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include "src/engine.h"

int main(int argc, char *argv[]) {

  //squash compiler warnings
  if (argc && argv) {}

  engine e;
  e.play();
  return 0;

  int screen_w = 640;
  int screen_h = 480;

  SDL_Window *w = NULL;
  SDL_Surface *ws = NULL;
  SDL_Surface *fs = NULL;

  if(SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cout << "S-H-I-T! couldn't init video! error: " << SDL_GetError() << std::endl;
    return(-1);
  }
  if(! (IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
    std::cout << "S-H-I-T! couldn't init png loading! error: " << SDL_GetError() << std::endl;
  }

  //basics so far: create a window, create a surface for the window, create a surface for the image, load the image

  //main window
  w = SDL_CreateWindow("p_lite pngtuber", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen_w, screen_h, SDL_WINDOW_SHOWN);
  if(w == NULL) {
    std::cout << "S-H-I-T! couldn't create window! error: " << SDL_GetError() << std::endl; 
    return (-2);
  }
  //fill it with blue
  ws = SDL_GetWindowSurface(w);
  SDL_FillRect(ws, NULL, SDL_MapRGB(ws->format, 0x0A, 0xAA, 0xFF));
  
  SDL_Surface *uofs = IMG_Load("resources/wisp_yellow.png");
  if(uofs == NULL) {
    std::cout << "S-H-I-T! couldn't load flame sprite! error: " << SDL_GetError() << std::endl;
  }
  fs = SDL_ConvertSurface(uofs, ws->format, 0);
  if(fs == NULL) {
    std::cout << "S-H-I-T! couldn't optimize flame sprite! error: " << SDL_GetError() << std::endl;
  }

  //SDL_BlitSurface(fs, NULL, ws, NULL);
  SDL_BlitSurface(uofs, NULL, ws, NULL);

  //get everything added and ready to show
  SDL_UpdateWindowSurface(w);

  std::cin.get();

  return(0);
}
