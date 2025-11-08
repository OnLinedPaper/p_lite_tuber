#include "src/time/time.h"
#include <algorithm>

void time::update() {
  double elapsed_ms = get_ms() - last_clock;
  delta = elapsed_ms / T_DELAY;

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
