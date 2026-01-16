#include "src/extra/rpoints.h"
#include "src/time/time.h"
#include <cmath>

rpoints::rpoints(int x, int y, int roam, int p, render *r) : 
    pin_x(x)
  , pin_y(y)
  , r(r) 
{ 
  /*
  for(int i=0; i<p; i++) {
    points.push_back(new rpoint(x, y, roam, r));
  }
  */
  points.push_back(new rpoint(x, y, roam,  true, 0   , r));
  points.push_back(new rpoint(x, y, roam, false, 0.33, r));
  points.push_back(new rpoint(x, y, roam, false, 0.66, r));
}
rpoints::~rpoints() { 
  while(!points.empty()) {
    rpoint *rp = points.front();
    delete rp;
    points.pop_front();
  }
}
void rpoints::update() {
  for(rpoint *r : points) { r->update(); }
}

void rpoints::draw() const {
  //draw something at the central pivot point - rect for now
  int col = 0xFF - std::min(((0xFF/16) * (time::get().get_tick() % 24)), 0xFF);
  SDL_SetRenderDrawColor(r->get_r(), col, col, 0x00, 0xFF);
  SDL_FRect cr = { 
      (float)(pin_x - 5)
    , (float)(pin_y - 5)
    , (float)(10)
    , (float)(10) 
  };
  SDL_RenderFillRect(r->get_r(), &cr);

  //now draw the points
  for(rpoint *r : points) { r->draw(); }
}

rpoint::rpoint(int x, int y, int roam, bool g, float clockstart, render *r) : 
    pin_x(x)
  , pin_y(y)
  , roam(roam)
  , main_clock(clockstart * 2 * std::acos(-1))
  , r_clock(0)
  , y_clock(0)
  , p_clock(0)
  , grow(g)
  , grow_guard(false)
  , r(r) 
{ 

}
rpoint::~rpoint() { }
void rpoint::update() {
  main_clock += 0.04;
  main_clock = fmod(main_clock, 2*std::acos(-1)); //stand-in for pi

  draw_x = 0.5 * std::cos(main_clock);
  draw_y = std::sin(main_clock);
  
  //if it reaches the "edge" of the shape, switch from grow to shrink
  float dist = std::sqrt(draw_x * draw_x + draw_y * draw_y);
  float gg_threshold = 0.995;
  if(!grow_guard && dist >= gg_threshold) {
    //it's reached the edge of the shape, toggle how it moves
    grow = !grow;
  }
  //now toggle grow guard, and don't untoggle it until away from edge
  grow_guard = dist >= gg_threshold;
}
void rpoint::draw() const { 
  //size of the rectangle is a combination of clock and distance to origin
  float size = 20.0;
  float dist = std::sqrt(draw_x * draw_x + draw_y * draw_y);

//-----------------------------------------------------------------------------
  //first attempt: use the periodic motion of the point to make it
  //larger and smaller.
  //pros: does a great job emulating distance!
  //cons: no may to manage a point which stays on one "side" of the shape.

/*
  //distance factor: how close the point is to the origin.
  float d_factor = 1 - 0.5 * dist;
  //clock factor: how far along on the clock cycle the point is.
  float c_factor = 0.5 + 0.75 * std::cos(main_clock);
  size = size * d_factor * c_factor;
*/

  //---------------------------------------------------------------------------
  //second attempt: measure the point's distance from the origin, and any time
  //that distance maxes out at "1", assume the point has crossed over to the 
  //"other side" of the shape and begin shrinking it based on proximity to the
  //origin instead of growing it.

  //distance factor: proximity to origin
  float d_factor = 1 - dist;
  size = size + 2 * size * d_factor * (float)(grow ? 1 : -1);

  int b_col = grow_guard ? 0xff : 0x00;
  SDL_SetRenderDrawColor(r->get_r(), 0xFF, 0x00, b_col, 0xFF);
  //clamp smallest size at 0
  size = std::max(size, (float)0);
  SDL_FRect cr = {
      (float)(roam * draw_x + pin_x - size/2)
    , (float)(roam * draw_y + pin_y - size/2)
    , (float)(size)
    , (float)(size)
  };
  SDL_RenderFillRect(r->get_r(), &cr);
}
