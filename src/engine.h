#ifndef ENGINE_H_
#define ENGINE_H_

#include "src/renders/render.h"

class engine {

public:
  engine();
  engine(const engine&) = delete;
  engine &operator=(const engine &) = delete;
  ~engine();

  void play();

private:
  bool init();
  bool is_init;
  void print_debug_swirly() const;
  double last_tick;


  //timekeeping things - last_tick tracks the last logic tick and tick_wait()
  //calculates how long until the next tick.
  //the logic rate is currently hard-capped at 24 ticks per second.
  //logic goes like this: when the engine finishes a loop, it asks tick_wait
  //to evaluate how long that loop took. then, it determines how long it needs
  //to sleep to maintain 24 ticks/sec and returns that. return value of 0 means
  //the last loop was slow, so it'll just immediately jump to the next one.
  //either way, last_tick gets updated with the last tick.
  //TODO: maybe skip frames for optimization? who knows?
  const int TPS = 24;
  const double T_DELAY = 1000.0/(double)TPS;
  double tick_wait();
  void tick_set();
  render r;
};

#endif
