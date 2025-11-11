#include "src/time/time.h"
#include <algorithm>

void time::update() {
  double elapsed_ms = get_ms() - last_clock;
  delta = elapsed_ms / T_DELAY;
  //delta shouldn't even be below 1.0
  delta = std::max(delta, 1.0);
  //also, try and keep it relatively close to 1.0 so small hiccups don't cause
  //small jitters - delta is for big jumps
  if (delta - 1.1 < 0) { delta = 1.0; }

  //update the clock
  last_clock = get_ms();

  //calculate how many logical ticks elapsed since the program started
  total_ticks = (last_clock - first_clock) / T_DELAY;
}

//returns the number of ms remaining until the next logical tick, or 0 if
//the next tick should've happened already
double time::get_wait() const {
  double remaining_ms = T_DELAY - (get_ms() - last_clock);
  return std::max(remaining_ms, 0.0);
}
