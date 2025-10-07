#ifndef ACTION_H_
#define ACTION_H_

#include <cstdint>
#include <vector>

/*
"actions" are ways a dollpart can be modified, manipulated, or otherwise
made to react to some sort of input, typically audio. this class is a virtual
base class; all derived classes are the actual interactions. 
for the time being, i am going to try to make actions "relative" rather than 
"absolute", i.e. favor "move 10 units up" to "move to this y coordinate" and
"rotate 5 degrees clockwise" to "rotate to this degree" etc. 
additionally, i am going to try to restrict actions to the X and Y axis, since
additional directions can be effected by combining the two in some proportion.

parts of an action are:
- "input" - handles input. typically meant to be fed mic audio, but can 
  receive anything (i.e. position of another dollpart etc)
- "threshold" - level of input at which to evaluate logic
- "trigger_type" - bitflag determining how the action behaves upon crossing
  a threshold. one or more of:
  - "UP_CONST": triggers the action as long as the input is greater than or
                equal to the threshold
  - "UP_PULSE": triggers the action once when the input crosses above or meets
                the threshold; will not retrigger until action is "complete"
  - "DN_CONST": triggers the action as long as the input is lesser than the
                threshold
  - "DN_PULSE": triggers the action once when the input crosses below the
                threshold; will not retrigger until action is "complete"
- note that the "pulse" action type is a rising/falling-edge trigger. if the
  input crosses the threshold for even a single frame, it'll run to
  completion; likewise, if the input is still over the threshold after the
  pulse is finished, it must return from and then re-cross the threshold 
  again to reactivate.
- "active" - whether this action has potential to influence the dollpart in
  the future. for effects that "destroy" a part (i.e. shrink-till-vanish),
  this is how the dollpart knows the action is "done" and can be removed from
  the list of actions.

derived actions are what actually manipulate a dollpart. these actions are:

- "TYPE_SF" - "sinefloat" - returns a periodic sinusoidal output. often used 
  to make a dollpart float back and forth along an axis. has its own subtype
  to either finish a full sinusoidal pattern or stop halfway through, which
  replicates a "bounce" behavior
  - const behavior: floats back and forth
  - pulse behavior: completes half a cycle, stopping when it reaches "0"
- "TYPE_HD" - "hide" - returns true or false, if a dollpart should be hidden 
  or visible, respectively. 
  - const behavior: returns 1 (hide) when the condition is met, and 
    0 (don't hide) otherwise
  - pulse behavior: returns 1 (hide) for a single update cycle when the
    condition is met, and 0 (don't hide) in all other circumstances.
- "TYPE_MV" - "move" - repositions a dollpart. movement scales with dollpart
  scale, but this isn't handled by the action, it's handled by the dollpart.
  movement occurs only on one axis (X or Y) but, like sinefloats, two
  movements can be combined on the axes to move in any direction.
  movement has a source and a destination; both are provided when the action
  is created. the output represents an offset from the source towards the
  destination. note that if the dollpart is moving, the destination will also
  be moving relative to the part's movement.
  in addition to a source and destination, a "time" is also provided,
  which represents how many ticks the movement takes from start to
  finish. the program runs at 24 tps, so a "time" of 24 will finish its
  movement in 1 second.
  movements use bezier curves to dictate movement speed. the source and
  destination represent the first and last control points; a vector of
  arbitrarily many additional control points may be included. if no additional
  contol points are provided, the movement uses constant velocity to reach its
  destination.
  TODO: decide if the below is sane.
  when movement reaches its destination, it can do one of three things: reset
  to the start, reverse direction, or nothing at all (this option makes the 
  action "single use"). note that "going backwards" performs the exact same
  motion in reverse, i.e. accelerating at the "start" will cause it to 
  decelerate as it approaches the "start" again on the return trip.
  - const behavior: advances towards the destination when the threshold is 
    met; stops advancing when the threshold is not met.
  - pulse behavior: advances toward the destination without interruption when
    the threshold pulses, and stops when it arrives.
*/

class action {
public:
  //threshold type flags
  static const uint32_t UP_CONST = 0;
  static const uint32_t UP_PULSE = 1;
  static const uint32_t DN_CONST = 2;
  static const uint32_t DN_PULSE = 3;

  //action type flags
  static const uint32_t TYPE_SF = 1;
  static const uint32_t TYPE_HD = 2;
  static const uint32_t TYPE_MV = 3;

  //axis flags
  static const uint32_t AXIS_X = 0;
  static const uint32_t AXIS_Y = 1;

  action(float threshold, uint32_t trigger_flags, uint32_t type);
  action() = delete;
  action(const action &) = delete;
  action &operator=(const action &) = delete;
  virtual ~action() = default;

  bool is_active() { return active; }

  uint32_t get_type() const { return type; }

  //squash "unused variable" and return - base class doesn't update anything
  virtual void update(float input) { if(input) { }; return; };
  //the actual function used for the output. don't call before update()!
  virtual float get_output() { return 0; };

protected:
  float threshold;
  uint32_t trigger_flags;
  uint32_t type;
  bool active;

  float last_input;
  bool is_pulse;
};

class act_sinefloat : public action {
public:
  static const uint32_t FUNC_SIN = 0;
  static const uint32_t FUNC_COS = 1;
  static const uint32_t SFTYPE_SF = 0;
  static const uint32_t SFTYPE_BN = 1;

  act_sinefloat() = delete;
  act_sinefloat(
      float threshold
    , uint32_t trigger_flags
    , uint32_t axis
    , float deflect
    , float speed
    , uint32_t sf_type
    , uint32_t func = FUNC_SIN
  );
  ~act_sinefloat() = default;
  void update(float input);
  float get_output();
  uint32_t get_axis() { return axis; }

private:
  uint32_t axis;
  float deflect;
  float speed;
  float clock;
  uint32_t sf_type; //either TYPE_SF or TYPE_BN for sinefloat/bounce
  uint32_t which_func;
  bool pulse_sign; //internal variable to track pulses - records whether the
                   //pure waveform is positive or negative
};


class act_hide : public action {
public:
  act_hide() = delete;
  act_hide(
      float threshold
    , uint32_t trigger_flags
  );
  ~act_hide() = default;
  void update(float input);
  float get_output();
private:
  bool is_hidden;
};


class act_move : public action {
public: 
  static const uint32_t MVTYPE_ST = 0; //stop
  static const uint32_t MVTYPE_RP = 1; //repeat
  static const uint32_t MVTYPE_RV = 2; //reverse

  act_move() = delete;
  act_move(
      uint32_t axis
    , int src
    , int dst
    , std::vector<int>
    , int time
    , uint32_t mv_type
    , float threshold
    , uint32_t trigger_flags
  );
  ~act_move() = default;
  void update(float input);
  float get_output();
  uint32_t get_axis() { return axis; }
private:
  //input values
  const uint32_t axis;
  const int src;
  const int dst;
  const std::vector<int> c_points;
  const int travel_time;
  uint32_t mv_type;

  bool reverse;
  int pos;
  int elapsed_ticks;
};

#endif
