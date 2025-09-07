#include "src/actions/action.h"
#include <cmath>
#include <iostream>

action::action(
    float thresh
  , uint32_t t_flags
  , uint32_t t
) :
    threshold(thresh)
  , trigger_flags(t_flags)
  , type(t)
  , active(true)
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
) :
    action(thresh, t_flags, action::TYPE_SF)
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
      ret = std::sin(clock * speed);
      break;
    case FUNC_COS:
      ret = std::cos(clock * speed);
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
) :
    action(thresh, t_flags, action::TYPE_HD)
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
}

float act_hide::get_output() {
  return (is_hidden ? 1 : 0);
}
