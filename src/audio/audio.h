#ifndef AUDIO_H_
#define AUDIO_H_

#include <vector>
#include <SDL3/SDL.h>

/*
second attempt at audio class, post-migration to SDL3. this one will be
much lighter-weight, and will NOT be asynchronous; it will instead update
via an explicit function call.

sdl3 uses input buffers that must be manually called upon when trying to
get data from them. this saves me the hassle of dealing with callbacks but
also means i have to be more careful about the data i manage
*/

class audio {
public:
  //various audio processing methods:
  //- RAW sends back raw audio data
  //- SQRT sends back sqrt of raw audio data
  //- RMS sends back root mean square of raw audio data over an interval
  //- LOG sends back log of raw audio data
  //- RMSLOG combines root mean square for signal smoothing and log to level
  //  off extreme volume levels
  enum audio_proc { RAW, SQRT, RMS, LOG, RMSLOG };

  //TODO: expand later
  //this time around, i'm foregoing the ability of the user to control anything
  //about the initialization. it gets the default input at a fixed 44100hz and
  //converts it to SDL_AUDIO_F32LE format; all the user controls is the audio
  //level method.
  audio(audio_proc);
  ~audio();

  //returns true if device is opened and ready to stream
  bool is_init() { return is_initialized; }

  //returns current audio level as a float value from 0.0 to 1.0, using 
  //whatever methoid's currently been selected
  float get_level() { return level; }

  //getter and setter for audio processing style
  audio_proc get_proc() const { return proc_method; }
  void set_proc(audio_proc p) { proc_method = p; }

  //query the stream to see if there's enough data. if there isn't, perform
  //no action and just wait for the next call.
  void update();
 
private:
  bool is_initialized;

  SDL_AudioStream *stream;  //SDL3 - the logical stream that hooks into the
                            //hardware and abstracts the messiness away. nice.

  audio_proc proc_method;
  int rms_interval;   //root mean square sample interval, in ms (default 300)
  int rms_max;
  std::vector<float> rms_arr;

  //various audio processing methods:
  //- RAW sends back raw audio data
  //  - values: 0-32767
  //- SQRT sends back sqrt of raw audio data
  //  - values: 0-182
  //- RMS sends back root mean square of raw audio data over an interval
  //  - values: 0-32767, but in practice rarely goes above... 2000 or so?
  //- LOG sends back log of raw audio data
  //  - values: 0-5

  float min_buffer_time;  //amount of audio needed, in seconds, to perform an
                          //update. 1/24 of a second by default.

  //the actual audio level, on a scale of 0.0 to 1.0, as reported by the 
  //chosen audio processing method. updated each time the buffer has new data.
  float level;

};

#endif
