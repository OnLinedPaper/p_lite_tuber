#ifndef DOLL_H_
#define DOLL_H_

#include <string>
#include <list>
#include "src/images/image.h"
#include "src/renders/render.h"
#include "src/actions/action.h"

//dolls are going to be FK-esque rigs. i'm going to try to keep them simple:
//each one has a single image (which may be animated) and two "inputs". 
//
//the first input is an (x,y,a) coordinate. ("a" is "angle"). 
//for the "base" doll this will map to a point on the screen, and for all 
//dollparts this will be the "pin" that attaches it to its parent dollpart.
//the second input will be some means to move or manipulate the dollpart.
//
//the pin input can be stiff or flexible in any orientation, i.e. a mouth
//dollpart is probably rigidly fixed in position and angle, whereas an
//arm dollpart might be rigidly position-fixed but flexibly angle-fixed,
//and a flame might be flexibly position-fixed but rigidly angle-fixed.
//
//the other input... i'm not sure how to describe. probably for audio
//stuff right now, but i bet i could find some other uses for it. this input
//can be tweaked based on thresholds to do different things. 
//note that the other input is expecting values between 0.0 and 1.0. 
//
//currently considering each part having an "output" function that lets it
//provide data to others, but i'm not sure what data it could potentially
//provide aside from its own position/rotation (which should be already
//accessible through getters and setters)
//
//although animated sprites are fine, a dollpart may NOT have more than one
//image, for the sake of tidiness. if users want to, say, make a mouth open or
//close based on audio levels, they need to attach two dollparts to the mouth
//area and toggle their transparency levels based on volume thresholds. this
//might make rigging trickier, but it'll keep code simple.
/*
class dollpart_old {
public:

private:
};
*/
//i'm still trying to decide what to do here. there's a chance i'll just make
//"doll" a handler class that's full of dollparts, which is used to update
//them, pass data along to them, and make sure they draw in the correct order.
//truth be told i feel like there's probably a better way to do this, but i'll
//consider it for the time being...
/*
class doll_old { 
public:

private:
};
*/


/*
dollparts are images that (for the time being) are just plain ol' static
images that can be moved around. 
- the "pin" is the top-left corner of the image. 
- the "parent" is the dollpart used as reference for the pin. (a null parent
  indicates this dollpart is pinned directly to the window.)
  when calculating where to render a dollpart image, first grab the pin of the
  parent, and then add the pin of the dollpart.
- the "scale" is the scale at which the dollpart is drawn. dollparts pinned to
  other dollparts will inherit that part's scale when pinned.

  TODO: "actions"? every dollpart's going to get audio input, and can take 
        actions based on it. each action has an... action... and a threshold.
        perhaps these separate actions have separate thresholds or something?
        actions include deflection (movement) and rendering (toggle
        visibility). 
        speaking makes the doll bounce upon a rising-edge threshold crossover.
        blinking is pseudorandom... maybe based on whether audio is mod-able
        by a certain value...?
        speaking toggles the closed and open mouth rendering(s) based on both
        rising- and falling-edge threshold crossovers
*/
class dollpart {
public:

  //string to load image with
  dollpart(std::string, render *);
  ~dollpart();

  int get_pin_x() const { return pin_x; }
  int get_pin_y() const { return pin_y; }
  int get_draw_x() const { return draw_x; }
  int get_draw_y() const { return draw_y; }
  float get_scale() const { return scale; }
  void set_scale(float s) { scale = s; }
  void add_action(action *a) { actions.push_back(a); }

  //pins this dollpart to the coordinates of another dollpart
  void pin_to(int x, int y, const dollpart *p);

  //update the dollpart. coordinates its movements and actions.
  void update(float input = 0);

  //render to screen. takes into account scale and parent coordiantes.
  void draw() /*const*/;

private:
  int pin_x;    //x coordinate this dollpart is pinned to 
  int pin_y;    //y coordinate this dollpart is pinned to
  int draw_x;   //x coordinate this dollpart is drawn to
  int draw_y;   //y coordinate this dollpart is drawn to
  float scale;  //scale at which dollpart is drawn and pins are adjusted
  image i;      //image to draw

  //TODO: decide whether or not this is a horrible idea. child dollparts
  //holding null references could really screw things up...
  const dollpart *parent;

  std::list<action *> actions;
};

#endif
