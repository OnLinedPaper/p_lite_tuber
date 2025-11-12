#include "src/images/image.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3/SDL_render.h>
#include <iostream>
#include <fstream>
#include "src/time/time.h"

image::image(const std::string c_file, render *r) : 
    control_file_path(c_file)
  , image_file_path("")
  , frames(-1)
  , fps(-1) 
  , width(-1)
  , height(-1)
  , t(NULL)
  , r(r)
{ 
  //if this failbit is set, default to a "missing texture"
  bool fail = false;
  
  //open the file
  std::ifstream f(c_file, std::ios::in);
  if(!f) { fail = true; }

  //get the image data
  if(!fail) {
    f >> image_file_path;
    f >> frames;
    f >> fps;
    f >> width;
    f >> height;
  }
  f.close();

  //check to see if anything failed to load. TODO: reporting, later
  if(
      image_file_path == "" || 
      frames == -1 || 
      fps == -1 || 
      width == -1 || 
      height == -1
  ) { fail = true; }

  //check to see if any settings were invalid
  if(
      frames <= 0 ||
      fps < 0 ||
      width <= 0 ||
      height <= 0
  ) { fail = true; }

  //SDL3: past this point, switching out old method with new one. 
  t = IMG_LoadTexture(r->get_r(), image_file_path.c_str());
}

image::~image() { 
  SDL_DestroyTexture(t);
}

void image::draw(int x, int y, float scale) const { 
  //draw the actual image. remember: x/y coordinates refer to the top left 
  //corner of the image.
  //TODO: same shit as qdbp's r_c_o_all, i have no doubt this'll balloon in size
  //as time goes on. at least i don't have to deal with viewport calculations
  //anymore since it's all in one window.
  
  //check to see if it's on the screen; don't draw it if it's not.
  //TODO: adjust check for rotating images later.

  if(
      x > r->get_w() ||
      x + width * scale < 0 ||
      y > r->get_h() ||
      y + height * scale < 0
  ) { return; }

  //what part of the window to render to
  SDL_FRect dest_r;
  dest_r.x = x;
  dest_r.y = y;
  dest_r.w = width * scale;
  dest_r.h = height * scale;

  //for multi-frame sprites, the entire sheet is loaded into a single 
  //horizontal texture. calculate which portion to render based off of the
  //frames and framerate.
  //note: this is based on global tick clock. since images are shared between
  //instances and may be drawn multiple times per update cycle, there's no way
  //to make them update at consistent fps except with an outside frame of
  //reference.

  //what part of the texture to render from
  //note: source rect refers to raw texture file, and does NOT need to be
  //scaled!
  SDL_FRect src_r;
  src_r.x = 0;
  src_r.y = 0;
  src_r.w = width;
  src_r.h = height;

  if(fps > 0) {
    //calculate how many ticks must pass before the frame changes - note that
    //this (obviously) cannot update frames more than once per update. no i'm
    //not going to "skip" frames.
    int tpu = std::max(time::get().get_TPS() / fps, 1);
    //now calculate which frame to render: divide elapsed ticks by how many 
    //ticks must pass before an update, and then clamp that to the total number
    //of frames in an image. 
    int frame_to_render = (time::get().get_tick() / tpu) % frames;
    //finally, displace the rendering rectangle to match the frame in question.
    src_r.x = width * frame_to_render;
  }

  //TODO: pivot here, later.

  if(false) {
    //draw rect around every image
    SDL_SetRenderDrawColor(r->get_r(), 128,128,128,128);
    SDL_RenderRect(r->get_r(), &dest_r);
  }

  SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawBlendMode(r->get_r(), SDL_BLENDMODE_BLEND);
  SDL_RenderTexture(r->get_r(), t, &src_r, &dest_r);
}
