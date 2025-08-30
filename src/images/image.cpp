#include "src/images/image.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <fstream>

image::image(const std::string c_file, render *r) : 
    control_file_path(c_file)
  , image_file_path("")
  , frames(-1)
  , fps(-1) 
  , width(-1)
  , height(-1)
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


  //get the image
  SDL_Surface *raw_surf = IMG_Load(image_file_path.c_str());
  if(raw_surf == NULL) { fail = true; }

  if(fail) {
    //something failed to load. warn the user and stick a default texture in here.
    //TODO: default texture, later. just warn for now and continue.
    //TODO: move this down lower. wait until all image ops have been attempted
    //before conclusively deciding pass or fail.
    std::cerr << "image failed to load!" << std::endl;
  }
  else {
    //load the image here - near-verbatim from qdbp
    
    //first, splice the image up into multiple frames
    SDL_Rect clip;
    clip.x = 0;
    clip.y = 0;
    clip.w = width;
    clip.h = height;
    
    for(int i=0; i<frames; i++) {
      //make a surface to copy the frame onto
      SDL_Surface *surf = SDL_CreateRGBSurface(
          0
        , width
        , height
        , raw_surf->format->BitsPerPixel
        , raw_surf->format->Rmask
        , raw_surf->format->Gmask
        , raw_surf->format->Bmask
        , raw_surf->format->Amask
      );

      //offset for the frame we're grabbing
      clip.x = width * i;
      //copy the surface
      SDL_BlitSurface(raw_surf, &clip, surf, NULL);

      //now, save it to a texture and stick it in the vector
      SDL_Texture *t;
      if(surf == NULL) {
        //TODO: throw an error and deal with it
      }
      else {
        t = SDL_CreateTextureFromSurface(r->get_r(), surf);
        if(t == NULL) {
          //TODO: throw an error and deal with it
        }
        //save the texture
        t_vec.push_back(t);
      }
      //free the surface
      SDL_FreeSurface(surf);
    }
    //free the surface
    SDL_FreeSurface(raw_surf);
  }
}

image::~image() { 
  for(SDL_Texture *t : t_vec) {
    SDL_DestroyTexture(t);
  }
}

void image::draw(int x, int y, float scale) const { 
  //draw the actual image. remember: x/y coordinates refer to the top left 
  //corner of the image.
  //TODO: same shit as qdbp's r_c_o_all, i have no doubt this'll balloon in size
  //as time goes on. at least i don't have to deal with viewport calculations
  //anymore since it's all in one window.
  
  //check to see if it's on the screen; don't draw it if it's not.
  //TODO: adjust check for rotating images later.

  //TODO: determine how to use scale, if at all - likely will apply only to
  //width and height, not to x and y (though this will make coding dollpart
  //coordinates tough... hm. maybe make the doll use scale? worry about it
  //later, scale for now. likely the doll will handle the coordinates and
  //the image can deal with the scale.
  if(
      x > r->get_w() ||
      x + width * scale < 0 ||
      y > r->get_h() ||
      y + height * scale < 0
  ) { return; }

  SDL_Rect dest_r;
  dest_r.x = x;
  dest_r.y = y;
  dest_r.w = width * scale;
  dest_r.h = height * scale;

  //TODO: pivot here, later.

  int frame_to_render = 0;
  //TODO: frame stuff here, sooner rather than later.
  SDL_Texture *t = t_vec[frame_to_render];
  //SDL_SetTextureBlendMode(t, SDL_BLENDMODE_NONE);
  //SDL_SetTextureBlendMode(t, SDL_BLENDMODE_MOD);
  //SDL_SetTextureBlendMode(t, SDL_BLENDMODE_ADD);
  SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND_PREMULTIPLIED);
  SDL_RenderCopyEx(r->get_r(), t, NULL, &dest_r, 0, NULL, SDL_FLIP_NONE);
}
