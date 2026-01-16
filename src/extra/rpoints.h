#ifndef RPOINTS_H_
#define RPOINTS_H_

#include <list>
#include "src/renders/render.h"

//experiment to see if i can simulate something 3d without actually modeling
//a 3d object. picture, say, a d4 tumbling slowly in zero-g. the points of the
//die appear to follow sinusoidal patterns... could these perhaps be used to
//simulate a shape? 
class rpoint {
public:
  rpoint(int, int, int, bool, float, render *);
  ~rpoint();

  void update();
  void draw() const;

private:
  int pin_x;    //the "center" of the shape around which the point pivots
  int pin_y;
  int roam;
  float draw_x;   //the coordinate to draw this point at
  float draw_y;

  float main_clock;   //periodic movement
  float r_clock;      //roll
  float y_clock;      //yaw
  float p_clock;      //pitch
  bool grow;          //true = grow, false = shrink
  bool grow_guard;    //dont toggle grow until we LEAVE the edge

  render *r;
};

class rpoints {
public:
  rpoints(int, int, int, int, render *);
  ~rpoints();

  void update();
  void draw() const;

private:
  int pin_x;
  int pin_y;
  std::list<rpoint *> points;

  render *r;
};

#endif
