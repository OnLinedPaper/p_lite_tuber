#include "src/actions/action.h"
#include "src/events/event.h"
#include <cmath>
#include <iostream>
#include "src/time/time.h"

action::action(
    float thresh
  , uint32_t t_flags
  , uint32_t t
  , int e_f
) :
    threshold(thresh)
  , trigger_flags(t_flags)
  , type(t)
  , active(true)
  , e_flags(e_f)
  , last_input(0)
  , is_pulse(false)
{ }

act_sinefloat::act_sinefloat(
    float thresh
  , uint32_t t_flags
  , uint32_t a
  , float d
  , float s
  , uint32_t sf_t
  , uint32_t func
  , int e_f
) :
    action(thresh, t_flags, action::TYPE_SF, e_f)
  , axis(a)
  , deflect(d)
  , speed(s)
  , clock(0)
  , sf_type(sf_t)
  , which_func(func)
{ }

void act_sinefloat::update(float input) {
  //TODO: determine if this can be shipped to action::update()
  bool should_update = false;
  switch(trigger_flags) {
    case UP_CONST:
      if(input >= threshold) { should_update = true; }
      break;
    case DN_CONST:
      if(input < threshold) { should_update = true; }
      break;
    case UP_PULSE:
      if(!is_pulse && input >= threshold && last_input <  threshold) { 
        is_pulse = true;
        should_update = true; 
      }
      break;
    case DN_PULSE:
      if(!is_pulse && input <  threshold && last_input >= threshold) {
        is_pulse = true;
        should_update = true;
      }
      break;
  }

  //always update during a pulse
  if(is_pulse) { should_update = true; }

  //record last input
  last_input = input;

  if(!should_update) { return; }  

  //now for the tricky part. if it's pulsing, it's meant to return after the
  //waveform returns to 0 (or crosses over it).
  //accomplish this by getting the current output, ticking the clock, and then
  //re-getting the output to see if the sign flipped. (0 is positive)
  bool pre_tick = pulse_sign;

  //tick the clock
  clock += 1.0;

  //force an update to get the output
  get_output();

  if(is_pulse && pre_tick != pulse_sign) { is_pulse = false; }  

  return;
}

float act_sinefloat::get_output() {
  float ret = 0;
  switch(which_func) {
    case FUNC_SIN:
      ret = std::sin(clock * speed * time::get().get_delta());
      break;
    case FUNC_COS:
      ret = std::cos(clock * speed * time::get().get_delta());
      break;
  }

  pulse_sign = std::signbit(ret);
  
  switch(sf_type) {
    case SFTYPE_SF:
      break;
    case SFTYPE_BN:
      ret = std::fabs(ret);
  }

  ret *= deflect;
  return ret;
}

act_hide::act_hide(
    float thresh
  , uint32_t t_flags
  , int e_f
) :
    action(thresh, t_flags, action::TYPE_HD, e_f)
  , is_hidden(false)
{ }

void act_hide::update(float input) {
  switch(trigger_flags) {
    case UP_CONST:
      //hide when above threshold
      is_hidden = (input >= threshold ? true : false);
      break;
    case DN_CONST:
      //hide when below threshold
      is_hidden = (input <  threshold ? true : false);
      break;
    case UP_PULSE:
      //always hide, unless threshold crossed up
      if(is_pulse) { is_hidden = true; is_pulse = false; }
      else if(input >= threshold && last_input <  threshold) {
        is_hidden = false;
        is_pulse = true;
      }
      break;
    case DN_PULSE:
      //always hide, unless threshold crossed down
      if(is_pulse) { is_hidden = true; is_pulse = false; }
      else if(input <  threshold && last_input >= threshold) {
        is_hidden = false;
        is_pulse = true;
      }
      break;
  }

  last_input = input;
}

float act_hide::get_output() {
  return (is_hidden ? 1 : 0);
}


act_move::act_move(
    uint32_t a
  , int s
  , int d
  , std::vector<int> c_p
  , int t
  , uint32_t m
  , float thresh
  , uint32_t t_flags
  , int e_f
) :
    action(thresh, t_flags, action::TYPE_MV, e_f)
  , axis(a)
  , src(s)
  , dst(d)
  , c_points(c_p)
  , travel_time(t)
  , mv_type(m)
  , reverse(false)
  , pos(src)
  , elapsed_ticks(0)
{ }

void act_move::update(float input) {
  bool should_update = false;
  switch(trigger_flags) {
    case UP_CONST:
      if(input >= threshold) { should_update = true; }
      break;
    case DN_CONST:
      if(input < threshold) { should_update = true; }
      break;
    case UP_PULSE:
      if(!is_pulse && input >= threshold && last_input <  threshold) { 
        is_pulse = true;
        should_update = true; 
        elapsed_ticks = 0;
      }
      break;
    case DN_PULSE:
      if(!is_pulse && input <  threshold && last_input >= threshold) {
        is_pulse = true;
        should_update = true;
        elapsed_ticks = 0;
      }
      break;
  }

  last_input = input;
  //std::cout << should_update << " " << is_pulse << " " << threshold << " " << input << std::endl;
  
  //what to do when reaching the end of a pulse?
  if(
    (trigger_flags == UP_PULSE || trigger_flags == DN_PULSE) &&
    (is_pulse && elapsed_ticks >= travel_time)
  ) {
    switch(mv_type) {
      case MVTYPE_ST:
        //stay in this exact position
        is_pulse = false;
        event_v1::get().queue_flag(e_flags);
        return;
      case MVTYPE_RP:
        //reset and keep pulsing
        elapsed_ticks = 0;
        break;
      case MVTYPE_RV:
        //stay, but next pulse, go backwards
        reverse = !reverse;
        is_pulse = false;
        break;
    }
  }

  //what to do when reaching the end of the constant?
  if(
    (trigger_flags == UP_CONST || trigger_flags == DN_CONST) &&
    (elapsed_ticks >= travel_time) 
  ) {
    switch(mv_type) {
      case MVTYPE_ST:
        //stay in this exact position, BUT snap to 0 when threshold drops
        if(!should_update) { elapsed_ticks = 0; pos=0; }
        event_v1::get().queue_flag(e_flags);
        return;
      case MVTYPE_RP:
        //reset and keep moving
        elapsed_ticks = 0;
        break;
      case MVTYPE_RV:
        //reverse, reset, and keep moving
        reverse = !reverse;
        elapsed_ticks = 0;
        break;
    }
  }

  if(!(should_update || is_pulse)) {
    elapsed_ticks = 0;
    pos = 0;
    return;
  }

  //bezier curve logic. couple steps:
  //- make a vector of all control points
  //- while vector elements > 1, loop:
  //  - get distance between points, scaled to ticks/travel_time
  //  - write this to the first point, move to the next point pair
  //  - knock the last point off the end of the vector

  elapsed_ticks = std::fmod(elapsed_ticks, travel_time);
  elapsed_ticks += time::get().get_delta();

  //this only applies during EXTREME lag, but if the delta is so bad that the
  //part somehow ticks its entire movement in one go, clamp it at its max
  elapsed_ticks = std::min(elapsed_ticks, travel_time);

  float t_ratio = (float) elapsed_ticks / (float) travel_time;

  std::vector<int> bezier;
  if(!reverse) {
    bezier.push_back(src);
    for(int i : c_points) { bezier.push_back(i); }
    bezier.push_back(dst);
  }
  else {
    //push points in reverse
    bezier.push_back(dst);
    for(int i = c_points.size()-1; i>=0; i--) { 
      bezier.push_back(c_points[i]); 
    }
    bezier.push_back(src);
  }

  while(bezier.size() > 1) {
    //for(int i : bezier) { std::cout << i << " "; }
    //std::cout << std::endl;

    //go through the elements and get the points (math in-place)
    for(unsigned int i=0; i<bezier.size() - 1; i++) {
      bezier[i] = (bezier[i] * (1.0 - t_ratio) + bezier[i+1] * (t_ratio));
    }
    //pop the last element off
    bezier.pop_back();
  }

  //now save this position
  pos = bezier[0];
}

//unique in that instead of returning an input or output, returns an offset
//value, to be added to the position of the given dollpart
float act_move::get_output() {
  return pos;
}
