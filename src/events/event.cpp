#include "src/events/event.h"

event_v1::event_v1() :
    flags_queued(0)
  , flags_active(0)
{ }

event_v1::~event_v1() { }

void event_v1::queue_flag(int flag) { 
  if(flag <= 0 || flag > 63) { return; }

  flags_queued |= (1 << flag);
}

bool event_v1::check_flag(int flag) { 
  if(flag <= 0 || flag > 63) { return false; }

  return flags_active & (1 << flag);
}

void event_v1::update() { 
  flags_active = flags_queued;
  flags_queued = 0;
}
