#ifndef TIME_H_
#define TIME_H_
#include <chrono>

/*
the time class acts as a global timekeeping function, with detla included
in case i later on want to do anything physics-based. 
the class logic is largely based off of qdbp's engine.

logical ticks are hard-capped at 24 per second at most. in the event that
the program starts running slow, time delta kicks in and can be used to 
"stretch" actions/motions/etc in order to maintain consistent performance,
even if the actual rendering becomes choppy or slow.

the engine updates the clock once per logical cycle.
*/
class time {
public:
  ~time() { }

  static time &get() {
    static time instance;
    return instance;
  }

  //returns elapsed logical ticks since start
  int get_tick() const { return total_ticks; } 
  //returns delta of last logical tick
  double get_delta() const { return delta; };
  //returns milliseconds remaining until next logical tick
  double get_wait() const;


  void update();

  static double get_ms() {
    return
        std::chrono::system_clock::now().time_since_epoch() / 
        std::chrono::milliseconds(1)
    ;
  }

private:
  const int TPS; //ticks per second
  const double T_DELAY; //delay in ms between logical ticks
  int total_ticks; //starts at 0, counts up indefinitely.

  //first_clock tracks the exact moment the clock was first invoked, and does
  //not change.
  //last_clock tracks the most recent clock tick, and changes after every 
  //update.
  const double first_clock;
  double last_clock;

  double delta; //a value representing how fast or slow the last frame was; a
                //value of 1.0 indicates the program is running at normal speed

  time() : 
      TPS(24)
    , T_DELAY(1000.0/(double)TPS)
    , total_ticks(0) 
    , first_clock(get_ms())
    , last_clock(first_clock)
    , delta(0)
  { }
  time(const time&) = delete;
  time &operator=(const time&) = delete;
  
};

#endif
