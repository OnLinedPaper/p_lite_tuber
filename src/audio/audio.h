#ifndef AUDIO_H_
#define AUDIO_H_

#include <vector>
#include <string>
#include <utility>

//the "audio" class will be responsible for the audio handling; specifically,
//it's responsible for the microphone input handling and processing. 
//
//many of the hardware-related values are hard-coded here, such as the sample
//rate, frequency of buffer writes, root mean square interval window, and 
//a number of others.

class audio {
public:
  //various audio processing methods:
  //- RAW sends back raw audio data
  //  - values: 0-32767
  //- SQRT sends back sqrt of raw audio data
  //  - values: 0-182
  //- RMS sends back root mean square of raw audio data over an interval
  //  - values: 0-32767, but in practice rarely goes above... 2000 or so?
  //- LOG sends back log of raw audio data
  //  - values: 0-5
  enum audio_proc { RAW, SQRT, RMS, LOG };

  //TODO: delete this later, probably
  audio();
  //allows user to specify most of the settings right off the bat:
  //- hardware id of input device to use
  //- sample frequency
  //- sample count
  //- channels
  //- samples per callback
  //- audio processing method
  audio(int, int, int, int, int, audio_proc);
  audio(const audio &) = delete;
  audio &operator=(const audio &) = delete;
  ~audio();

  //when given an id, attempts to initialize the device corresponding to that
  //id, and open it for audio streaming.
  bool init_device(int id);

  //returns true if device is opened and ready to stream
  bool is_init() { return is_initialized; }

  //returns current audio level as a float value from 0.0 to 1.0, using 
  //whatever methoid's currently been selected
  float get_level() { return level; }

  //getter and setter for audio processing style
  audio_proc get_proc() const { return proc_method; }
  void set_proc(audio_proc p) { proc_method = p; }

  //---- static stuff ---------------------------------------------------------
  //the following methods are usable from outside a specific member. this
  //is mostly done bcause these don't really need to be bound to an instance
  //of a class; the instance can pass in a processing method and a reference
  //to get the level back out when all's said and done.

  //populates an empty vector of pairs with a set of pairs, which contain
  //the id and associated name of an audio input device.
  static void get_devices(std::vector<std::pair<int, std::string>> &);

  //the callback used for audio streaming.
  static void static_stream_callback(void *, uint8_t *, int);

private:

  bool is_initialized;

  int hardware_id;
  int device_id;
  int sample_freq; //samples per second. default is 44100hz, but it can be cut
  int sample_count; //samples before a callback. smaller = more responsive
  int channels; //audio input channels. defaults to 1

  audio_proc proc_method;
  int rms_interval; //root mean square sample interval, in ms (300 by default)
  int rms_max;
  std::vector<float> rms_arr;

  //uses already-set member data and attempts to init the audio device.
  //check whether the device is initialized after calling this via is_init()
  void init_audio();

  void stream_callback(uint8_t *, int);


  //various audio processing methods:
  //- RAW sends back raw audio data
  //  - values: 0-32767
  //- SQRT sends back sqrt of raw audio data
  //  - values: 0-182
  //- RMS sends back root mean square of raw audio data over an interval
  //  - values: 0-32767, but in practice rarely goes above... 2000 or so?
  //- LOG sends back log of raw audio data
  //  - values: 0-5

  //the actual audio level, on a scale of 0.0 to 1.0, as reported by the 
  //chosen audio processing method
  float level;
};

#endif
