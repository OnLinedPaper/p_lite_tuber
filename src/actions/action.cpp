#include "src/actions/action.h"
#include <cmath>

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
{ }

act_sinefloat::act_sinefloat(
    float thresh
  , uint32_t t_flags
  , uint32_t a
  , float d
  , float s
) :
    action(thresh, t_flags, action::TYPE_SF)
  , axis(a)
  , deflect(d)
  , speed(s)
  , clock(0)
{ }

void act_sinefloat::update(float input) {
  bool should_update = false;
  switch(type) {
    case UP_CONST:
      if(input >= threshold) { should_update = true; }
      break;
    case DN_CONST:
      if(input < threshold) { should_update = true; }
      break;
  }

  if(!should_update) { return; }
  
  //tick the clock
  clock += 1.0;
  return;
}

float act_sinefloat::get_output() {
  return deflect * std::sin(clock * speed);
}
