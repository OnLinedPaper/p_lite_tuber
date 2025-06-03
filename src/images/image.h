#ifndef IMAGE_H_
#define IMAGE_H_

#include "src/renders/render.h"
#include <vector>
#include <string>

//much like qdbp, the "image" class will store a vector of textures in support
//of animation, as well as some other data too. i don't think i'll lazy-load
//this time, since i don't anticipate it'll lead to any sort of significant
//performance increase.
//
//of note is that an image is only meant to hold one animated sprite;
//transitioning between "states" will be accomplished via separate logic.
//this is just for holding and displaying whatever image is currently meant to
//be on the screen - let's keep it simple and sweet.
class image {
public:
  //images are going to have their data stored in a simple txt file. the
  //format will be:
  //
  //  image filename (with relative filepath)
  //  number of frames
  //  animation FPS (will deal with exact calculations later)
  //  width of ONE frame, in pixels
  //  height of ONE frame, in pixels
  //
  //the std::string is the relative filepath of the control file.
  image(const std::string, render *);

  //it's just occurred to me that an outside observer might not understand why
  //i'm deleting the copy constructor and assignment operators. to save time 
  //and memory, i don't want to copy a created image anywhere (nor do i need
  //to) - existing images are drawn from existing instances, and new ones are
  //loaded. neat, performative, modular code, wohoo
  image(const image &) = delete;
  image &operator=(const image&) = delete;

  ~image();

  //render class gets passed in, allowing the image to draw itself. done this
  //way to avoid qdbp's renderer singleton.
  //this MIGHT also let me finally make draw a const function.
  //parameter is x,y coordinates of the image's top left corner.
  void draw(int, int) const;

  int get_w() const { return width; }
  int get_h() const { return height; }
private:

  std::string control_file_path;
  std::string image_file_path;

  //animation stuff
  int frames;
  int fps;

  //normal dimensions
  int width;
  int height;

  //vector of textures to hold each animation's frame
  std::vector<SDL_Texture *> t_vec;

  //pointer to the renderer used for drawing
  //TODO: look VERY closely at this and decide whether or not it's sane. i
  //don't know what might come of storing a pointer to a renderer that is
  //potentially accessed by multiple sources at once, but... maybe it's fine?
  //qdbp's render singleton doesn't have any mutex installed, and it seems to
  //manage without issue.
  render *r;

};

#endif
