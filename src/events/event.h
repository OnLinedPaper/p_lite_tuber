#ifndef EVENT_H_
#define EVENT_H_

/*
broadly speaking, the event class is for tracking of global events - usually
used when an action finishes its action. 

event_v1:
this is the first iteration of the event class, and will likely NOT be
permanent - it's mostly a proof-of-concept. it's a global-space singleton that
may be accessed from anywhere.
the core of the class is a pair of 64-bit bitflags. adding a new flag adds it
to the queue; next update cycle, all queued flags are activated, and the queue
is zeroed, ready to accept more flags.
anyone may fire a flag, and anyone may read the existing flags. there will
probably be some predefined flags in here too.
*/

#include <cstdint>

class event_v1 {
public:
  static const uint64_t PHONE_UP = 1;

  ~event_v1();

  static event_v1 &get() {
    static event_v1 instance;
    return instance;
  }

  //accepts a number 1-63 and sets the corresponding bitflag in the queue
  void queue_flag(int flag);
  //acecpts a number 1-63 and checks the corresponding bitflag in the active
  bool check_flag(int flag);
  //moves queued flags to the active flags and zeroes the queue
  void update();

private:
  uint64_t flags_queued;
  uint64_t flags_active;

  event_v1();
  event_v1(const event_v1 &) = delete;
  event_v1 &operator=(const event_v1 &) = delete;
};



#endif
