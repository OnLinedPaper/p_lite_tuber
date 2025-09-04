#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <string>
#include <iostream>

int main(void) {

  const int sw = 640;
  const int sh = 480;

  SDL_Window *w = nullptr;
  SDL_Surface *s = nullptr;
  SDL_Renderer *r = nullptr;
  SDL_Texture *t = nullptr;
  int tw = 200;
  int th = 100;

  if(SDL_Init(SDL_INIT_VIDEO) == false) {
    std::cerr << "S_H_I_T! couldn't init video!" << std::endl;
    return -1;
  }

  if(SDL_CreateWindowAndRenderer("oh my god were doomed", sw, sh, 0, &w, &r) == false) {
    std::cerr << "S_H_I_T! couldn't create window!" << std::endl;
    return -3;
  }

  s = SDL_GetWindowSurface(w);
  if(s == nullptr) {
    std::cerr << "couldnt get w surf: " << SDL_GetError() << std::endl; return -6;
  }

  SDL_Surface *osurf = nullptr;
  SDL_Surface *lsurf = IMG_Load("resources/pngs/blend_check.png");
  if(lsurf == nullptr) {
    std::cerr << "couldnt load" << std::endl; return -4;
  }
  osurf = SDL_ConvertSurface(lsurf, s->format);
  if(osurf == nullptr) {
    std::cerr << "couldnt optimize" << std::endl; return -5;
  }

  t = SDL_CreateTextureFromSurface(r, osurf);
  if(t == nullptr) {
    std::cerr<< "couldnt make texture: " << SDL_GetError() << std::endl; return -7;
  }

  bool quit = false;
  SDL_Event e;
  SDL_zero(e);

  SDL_Texture *t2 = IMG_LoadTexture(r, "resources/pngs/blend_check.png");

  SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
  SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
  SDL_SetTextureBlendMode(t2, SDL_BLENDMODE_BLEND);

  while(quit == false) {
    while(SDL_PollEvent(&e) == true) {
      if(e.type == SDL_EVENT_QUIT) { quit = true; }
    }



    SDL_SetRenderDrawColor(r, 0xFF, 0xFF, 0xFF, 0xFF);
    //SDL_SetRenderDrawColor(r, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(r);
    SDL_FRect f1{50, 50, 200, 100};
    SDL_FRect f2{50, 200, 200, 100};
    SDL_FRect f3{50, 250, 200, 100};
    SDL_RenderTexture(r, t2, nullptr, &f1);
    SDL_RenderTexture(r, t2, nullptr, &f2);
    SDL_RenderTexture(r, t2, nullptr, &f3);

    SDL_RenderPresent(r);
  }

  SDL_DestroyWindow(w);
  return 0;
}
