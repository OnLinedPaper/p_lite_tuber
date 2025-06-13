#include "src/audio/audio.h"
#include <SDL2/SDL.h>
#include <vector>
#include <iostream>

audio::audio() : 
    is_initialized(false)
  , hardware_id(1)
  , device_id(-1)
  , sample_freq(44100)
  , sample_count(4096)
  , channels(1)
  , proc_method(RAW)
  , rms_interval(300) //300ms rms interval
  , rms_max(sample_freq * rms_interval / 1000.0)
{ 
  rms_arr.reserve(rms_max);
  for(int i=0; i<rms_max; i++) { rms_arr.push_back(0); }
}

audio::audio(int h, int freq, int count, int chan, int interval, audio_proc p) :
    is_initialized(false)
  , hardware_id(h)
  , device_id(-1)
  , sample_freq(freq)
  , sample_count(count)
  , channels(chan)
  , proc_method(p)
  , rms_interval(interval)
  , rms_max(sample_freq * rms_interval / 1000.0)
{ 
  rms_arr.reserve(rms_max);
  for(int i=0; i<rms_max; i++) { rms_arr.push_back(0); }
  init_audio();
}

audio::~audio() {
  //if we initialized, be sure to close things done when we're done
  if(is_initialized) {
    SDL_PauseAudioDevice(device_id, SDL_TRUE);
    SDL_CloseAudioDevice(device_id);
    is_initialized = false;
  }

  
}

void audio::init_audio() {
  SDL_AudioSpec spec_des, spec_act;
  SDL_zero(spec_des);
  SDL_zero(spec_act);

  spec_des.freq = sample_freq;
  spec_des.format = AUDIO_F32; //defaulting to F32
  spec_des.channels = channels;
  spec_des.samples = sample_count;
  spec_des.callback = static_stream_callback;
  spec_des.userdata = this; // <-- the secret sauce!

  //attempt to open the desired audio device
  device_id = SDL_OpenAudioDevice(
      SDL_GetAudioDeviceName(hardware_id, SDL_TRUE)
    , SDL_TRUE
    , &spec_des
    , &spec_act
    , SDL_AUDIO_ALLOW_FORMAT_CHANGE
  );

  if(device_id == 0) {
    //something went wrong! just return, though - the user can call SDL_Error()
    //on their own time when they see the init failed.
    return;
  }

  //TODO: compare desired and actual specs here, and... do something if they
  //don't match, i guess?? i'm not worried about it right now

  is_initialized = true;

  //roll playback!
  SDL_PauseAudioDevice(device_id, SDL_FALSE);
}

void audio::get_devices(std::vector<std::pair<int, std::string>> &d) {
  int device_count = SDL_GetNumAudioDevices(SDL_TRUE);
  for(int i=0; i<device_count; i++) {
    d.push_back({i, SDL_GetAudioDeviceName(i, SDL_TRUE)});
  }

}

void audio::static_stream_callback(void *userdata, uint8_t *stream, int len) { 
  //userdata holds the class; reinterpret it and then call its stream callback
  //https://stackoverflow.com/a/61842657
  const auto a = reinterpret_cast<audio *>(userdata);
  a->stream_callback(stream, len);
}

void audio::stream_callback(uint8_t *stream, int len) { 
  //first, convert the stream data. it's coming in as raw uint8_t, and needs
  //to be coverted to PCM format.
  //https://stackoverflow.com/a/45864513/7431860
  const float *fs = (const float *)stream;
  const float *fe = fs + len / 4;

  for( ; fs < fe; ++fs) {
    int16_t pcm = std::abs(*fs * 32767); //pcm now holds raw PCM data!

    //second, use the chosen processing method to calculate the audio level
    switch(proc_method) {
      case RAW:
        level = pcm / 32767.0;
        break;
      case SQRT:
        level = std::sqrt(pcm) / std::sqrt(32767.0);
        break;
      case RMS:
        //dont run this unless we're using rms
        static int rms_pos = 0;
        rms_arr[rms_pos] = pcm * pcm;
        rms_pos++;
        rms_pos %= rms_max;

        level = 0;
        //TODO: this loop is exiting immediately. why?
        for(const float &f : rms_arr) {
          level += f;
        }
        level = std::sqrt((1.0/float(rms_max)) * level) / 32767.0;
        break;
      case LOG:
        level = std::log(pcm) / std::log(32767.0);
        break;
      default:
        //make level creep upwards slowly and then reset, as a visual
        //warning that something's wrong with the code (i.e. i added a new
        //processing method and forgot to stick it here)
        level += 0.01;
        if(level >= 1.0) { level = 0.0; }
    }
  }
}
