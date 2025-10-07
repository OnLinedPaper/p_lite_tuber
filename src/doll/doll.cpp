#include "src/doll/doll.h"
  #include <iostream>
#include <cmath>

dollpart::dollpart(std::string image_path, render *r) :
    pin_x(0)
  , pin_y(0)
  , scale(1.0)
  , i(image_path, r)
  , hidden(false)
  , parent(NULL)
  //, actions //inits itself empty
{ }

dollpart::~dollpart() { 
  //don't bother with iterators, just pull the list apart from the front
  while(!actions.empty()) {
    switch(actions.front()->get_type()) {
      case action::TYPE_SF:
        act_sinefloat *a = (act_sinefloat *)actions.front();
        delete a;
        break;
    }
    //delete actions.front();
    actions.pop_front();
  }

}

void dollpart::pin_to(int x, int y, const dollpart *p) {
  pin_x = x;
  pin_y = y;
  parent = p;
  if(parent != NULL) {
    scale = parent->get_scale();
  }
}

void dollpart::update(float input) {
  //calculates the conditions under which to draw the dollpart, and also 
  //updates the actions as well

  //update the actions
  //TODO: kill the inactive ones. yes, i could recycle them, but i can do that
  //later if i really have to. honestly, since actions aren't resource-heavy,
  //the overhead should be negligible anyway.
  //TODO: also convert this to explicit iterator, or at least find out what
  //ranged for does under the hood.
  for(action *a : actions) {
    a->update(input);
  }

  //calculate draw coordinates based on known values

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

  //now handle the actions
  for(action *a : actions) {
    switch (a->get_type()) {
      case action::TYPE_SF: {
        //sinefloat
        float sf_data = ((act_sinefloat *)a)->get_output();
        switch (((act_sinefloat *)a)->get_axis()) {
          //deflect sf by scale, even if attached to window directly
          case action::AXIS_X:
            draw_x += sf_data * scale;
            break;
          case action::AXIS_Y:
            draw_y += sf_data * scale;
            break;
        }
        break;
      }
      case action::TYPE_HD: {
        //hide
        float hide = ((act_hide *)a)->get_output();
        if(hide == 0) { hidden = false; }
        else { hidden = true; }
        break;
      }
      case action::TYPE_MV: {
        //move
        float mv_data = ((act_move *)a)->get_output();
        switch(((act_move *)a)->get_axis()) {
          case action::AXIS_X:
            draw_x += mv_data * scale;
            break;
          case action::AXIS_Y:
            draw_y += mv_data * scale;
            break;
        }
        break;
      }
    }
  }
//std::cout << pin_x << ", " << pin_y << " | " << draw_x << ", " << draw_y << std::endl;
}

void dollpart::draw() {
  if(hidden) { return; }
 
  i.draw(draw_x, draw_y, scale);
}
