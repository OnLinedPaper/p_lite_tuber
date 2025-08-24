#ifndef ACTION_H_
#define ACTION_H_

#include <cstdint>

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
*/

class action {
public:
  //threshold type bitflags
  static const uint32_t UP_CONST = 0;
  static const uint32_t UP_PULSE = 1;
  static const uint32_t DN_CONST = 2;
  static const uint32_t DN_PULSE = 3;

  //action type bitflags
  static const uint32_t TYPE_SF = 1;

  //axis bitflags
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

//TODO: internally, sinefloat and bounce are identical, differing only in
//their putput value (which gets put through abs() before return)
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


class act_bounce : public action {
public:
  act_bounce(
      float threshold
    , uint32_t trigger_flags
    , int axis
    , float deflect
    , float speed
  );
  ~act_bounce();
  void update(float input);

};

#endif
