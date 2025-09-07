#include "src/audio/audio.h"
#include <cmath>
  #include <iostream>

audio::audio(audio_proc p) :
    is_initialized(false)
  , stream(nullptr)
  , proc_method(p)
  , rms_interval(900) //TODO: remove magic numbers
  , rms_max(0)
  , min_buffer_time(1.0/24.0)
  , level(0)
{ 
  //create a standardized audio spec and use it to open the default recording
  //device, which iirc changes dynamically as things are plugged and
  //unplugged.
  //TODO: check what happens when all inputs are unplugged.
  //TODO: let user choose input device, later

  SDL_AudioSpec spec { SDL_AUDIO_F32LE, 1, 44100 };
  stream = SDL_OpenAudioDeviceStream(
      SDL_AUDIO_DEVICE_DEFAULT_RECORDING
    , &spec
    , NULL
    , NULL
  );

  if(!stream) {
    //TODO: deal with this later. could abort, could just warn user there's
    //no input device. 
  }


  //set up RMS stuff
  rms_max = spec.freq * rms_interval / 50000.0;
  rms_arr.reserve(rms_max);
  for(int i=0; i<rms_max; i++) { rms_arr.push_back(0); }

  SDL_ResumeAudioStreamDevice(stream);

  is_initialized = true;
}

audio::~audio() {
  SDL_PauseAudioStreamDevice(stream);
  SDL_DestroyAudioStream(stream);
}

/*
check how much data is available in the buffer already - if there's enough
to perform a processing check, grab it and do so; else, do nothing.
*/
void audio::update() {
  //slightly redundant but let's be on the safe side
  SDL_AudioSpec spec;
  SDL_GetAudioStreamFormat(stream, &spec, NULL);

  //TODO: figure out why the fuck this works. this stream doesn't appear to
  //do anything, but when i remove it, the other audio stream breaks and
  //starts slowing down. i'm also 80% sure this is leaking memory, though it's
  //a constant value, not something that'll crash eventually
  static SDL_AudioStream *pbk = SDL_OpenAudioDeviceStream(
      SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK
    , &spec
    , NULL
    , NULL
  ); if(pbk != nullptr) { ;; } // <-- no-op to squash compiler warning

  const int min_size = (SDL_AUDIO_FRAMESIZE(spec) * spec.freq) * min_buffer_time;

  if(min_size > SDL_GetAudioStreamAvailable(stream)) {
    //not enough data here to update yet
    return;
  }

  //process the data
  Uint8 *buf = new Uint8[min_size];
  const int bytes_read = SDL_GetAudioStreamData(stream, buf, min_size);

  //precompute these values
  static const float PCM_MAX = 32767.0;
  static const float PCM_MAX_SQRT = std::sqrt(PCM_MAX);
  static const float PCM_MAX_LOG = std::log(PCM_MAX);

  //convert the incoming data to pcm
  //https://stackoverflow.com/a/45864513/7431860
  const float *fs = (const float *)buf;
  const float *fe = fs + bytes_read / 4;

  for( ; fs < fe; ++fs) {
    int16_t pcm = std::abs(*fs * PCM_MAX); //pcm now holds raw PCM data!

    //now, calculate level from these values
    switch(proc_method) {
    case RAW:
      level = pcm / PCM_MAX;
      break;
    case SQRT:
      level = std::sqrt(pcm) / PCM_MAX_SQRT;
      break;
    case RMSLOG:
    case RMS:
      //dont run this unless we're using rms
      static int rms_pos = 0;
      rms_arr[rms_pos] = pcm * pcm;
      rms_pos++;
      rms_pos %= rms_max;

      level = 0;
      for(const float &f : rms_arr) {
        level += f;
      }
      level = std::sqrt((1.0/float(rms_max)) * level);

      //last-second check to see what form we want
      if(proc_method == RMS) {
          level /= 32767.0;
      }
      else if(proc_method == RMSLOG) {
        level = std::log(level) / PCM_MAX_LOG;
      }
      break;
    case LOG:
      level = std::log(pcm) / PCM_MAX_LOG;
      break;
    default:
      //make level creep upwards slowly and then reset, as a visual
      //warning that something's wrong with the code (i.e. i added a new
      //processing method and forgot to stick it here)
      level += 0.01;
      if(level >= 1.0) { level = 0.0; }
    }
  }

  delete[] buf;
}
