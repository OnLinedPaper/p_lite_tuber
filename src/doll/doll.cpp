#include "src/doll/doll.h"
#include <cmath>

dollpart::dollpart(std::string image_path, render *r) :
    pin_x(0)
  , pin_y(0)
  , scale(1.0)
  , i(image_path, r)
  , sf_deflect_x(0)
  , sf_speed_x(0)
  , sf_deflect_y(0)
  , sf_speed_y(0)
  , parent(NULL)
{ }

dollpart::~dollpart() { }

void dollpart::pin_to(int x, int y, const dollpart *p) {
  pin_x = x;
  pin_y = y;
  parent = p;
  if(parent != NULL) {
    scale = parent->get_scale();
  }
}

void dollpart::draw() {
  //"s" represents coordinate deflect scale. all dollparts adjust their
  //image size based on scale, but only parts pinned to other parts adjust
  //their coordiantes. 
  float s = 1.0;
  draw_x = 0;
  draw_y = 0;
  if(parent != NULL) {
    //okay, we start from parent coordinates and then scale dimensions
    s = scale;
    draw_x += parent->get_draw_x();
    draw_y += parent->get_draw_y();
  }

  //now deflect based on pin point, adjusting if there's a parent
  draw_x += pin_x * s;
  draw_y += pin_y * s;

  //add some additional deflect based on sf
  static uint32_t sf_clock = 0;
  sf_clock++;
  draw_x += sf_deflect_x * std::sin(sf_clock * sf_speed_x) * s;
  draw_y += sf_deflect_y * std::sin(sf_clock * sf_speed_y) * s;
  
  i.draw(draw_x, draw_y, scale);
}
