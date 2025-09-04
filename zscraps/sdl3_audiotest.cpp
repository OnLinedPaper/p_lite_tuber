#include <iostream>
#include <iomanip>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>
#include <cmath>
#include <chrono>

double tick_wait() {
  //first, get the current tick
  static double last_tick = 0;
  double curr_tick = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch()
  ).count();

  double elapsed = curr_tick - last_tick;
  last_tick = curr_tick;

  double wait = 1000.0/24.0;
  return std::max(0.0, wait - elapsed);
}

void old_callback(void *userdata, Uint8 *stream, int len) {
      //process data in buf
      static int cycle=0;      

      const float *fs = (const float *)stream;
      const float *fe = fs + len / 4;
      
      static const float PCM_MAX = 32767.0;
      static const float PCM_MAX_SQRT = std::sqrt(PCM_MAX);
      static const float PCM_MAX_LOG = std::log(PCM_MAX);
      float l1, l2, l3 = 0;

      for( ; fs < fe; ++fs) {
        int16_t pcm = std::abs(*fs * PCM_MAX); //pcm now holds raw PCM data!
        
        l1 = pcm / PCM_MAX;
        l2 = std::sqrt(pcm) / PCM_MAX_SQRT;
        static const int rms_max = 200;
        static float rms_arr[rms_max];
        static int rms_pos = 0;
        rms_arr[rms_pos] = pcm*pcm;
        rms_pos++;
        rms_pos %= rms_max;

        float level = 0;
        for(const float &f : rms_arr) {
          level += f;
        }
        l3 = std::sqrt((1.0/float(rms_max)) * level) / 32767.0;

      }
    
    std::cout << "\n\n\n\n" << cycle++ << " (" << cycle/24 << ")\nRAW:  " << l1 << "\nSQRT: " << l2 << "\nRMS:  " << l3 << "\n" << std::flush;
}

void SDLCALL stream_callback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount) {
  if(additional_amount > 0) {
    Uint8 *data = SDL_stack_alloc(Uint8, additional_amount);
    if(data) {
      old_callback(userdata, data, additional_amount);
      SDL_GetAudioStreamData(stream, data, additional_amount);
      SDL_ClearAudioStream(stream);
      SDL_stack_free(data);
    }  
  }
}

//test program - opens an sdl3 audio device and starts recording audio levels.
int main(void) {

  //---- init the devices -----------------------------------------------------
  if(!SDL_Init(SDL_INIT_AUDIO)) {
    std::cerr << "S_H_I_T! couldnt init audio" << std::endl;
    return -1;
  }

  int device_count_i = 0;
  int device_count_o = 0;
  SDL_AudioDeviceID *devices_i = SDL_GetAudioRecordingDevices(&device_count_i);
  SDL_AudioDeviceID *devices_o = SDL_GetAudioPlaybackDevices(&device_count_o);
  if(!devices_i || !devices_o) {
    std::cerr << "S_H_I_T! couldnt get devices" << std::endl;
    return -1;
  }
  else if (device_count_i < 1 || device_count_o < 1) {
    std::cerr << "couldn't find recording/playback devices" << std::endl;
    return 0;
  }

  for(int i=0; i<device_count_i; i++) {
    std::cout << i << ": " << SDL_GetAudioDeviceName(devices_i[i]) << std::endl;
  }

  for(int i=0; i<device_count_o; i++) {
    std::cout << i << ": " << SDL_GetAudioDeviceName(devices_o[i]) << std::endl;
  }

  
  


  //---- link up audio stream -------------------------------------------------
  //just grab default device for now
  //TODO: format -> lower sample rate
  SDL_AudioSpec spec { SDL_AUDIO_F32LE, 1, 44100 };
  //SDL_AudioStream *rec = SDL_OpenAudioDeviceStream(devices[0], &spec, stream_callback, NULL);
  SDL_AudioStream *rec = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_RECORDING, &spec, NULL, NULL);
  //SDL_AudioStream *rec = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_RECORDING, NULL, NULL, NULL);
  if(!rec) {
    std::cerr << "S_H_I_T! couldnt link input stream" << std::endl;
    return -1;
  }

  //what format is this in...?
  SDL_GetAudioStreamFormat(rec, &spec, NULL);
  //std::cout << std::hex << spec.format << std::endl;
  //std::cout << (spec.format == SDL_AUDIO_S16LE) << std::endl;

  //now, open a playback device
  SDL_AudioSpec spec2;
  SDL_GetAudioStreamFormat(rec, &spec2, NULL);
  SDL_AudioStream *pbk = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec2, NULL, NULL);
  if(!pbk) {
    std::cerr << "S_H_I_T! couldnt link output stream" << std::endl;
    return -1;
  }
  
  

  //---- run the loop ---------------------------------------------------------
  bool quit = false;
  SDL_Event e;
  int len = 0;

  int ms_delay = (1000/24);
  //guess at how much data to get...
  const int get_size = SDL_AUDIO_FRAMESIZE(spec) * spec.freq / ms_delay;

  int cycle = 400;
  int cycle_count = 0;
  float l1, l2, l3 = 0;

  const int RECORDING = 0;
  const int PLAYBACK = 1;
  int state = RECORDING;

//  double cycle_time = 5;
  double cycle_time = 1.0/24.0; //24 per second

  SDL_ResumeAudioStreamDevice(rec);

  //callback notes:
  //- when audio comes in, a buffer fills
  //- when getaudiostreamavailable is called, it says how much buffer is filled
  //- when getaudiostreamdata is called, a buffer is filled and i THINK it is also emptied an equal amount
  //- when getaudiostreamdata is called, the callback ALSO fires. total_amount is the amount that the original
  //  call was asking for, and additional_amount is *how short the stream is* - if it's 0, the stream had
  //  enough data to satisfy the call already.
  //
  //i think the play is to ditch the callback. then, call getaudiostreamavailable every update cycle, and
  //if it has enough data, call getaudiostreamdata to grab the data for processing. this will likely fire
  //at the same 24-times-per-second speed as the rest of the code. let's see if it works...

  //take 3 - check audio levels
  while(!quit) { 
    //event handling loop - get user inputs
    while(SDL_PollEvent(&e) != 0) {
      //check to see if the window was closed out
      if(e.type == SDL_EVENT_QUIT) { quit = true; }
    }

    //how much data is available?
    const int recorded_sofar = SDL_GetAudioStreamAvailable(rec);
    //report it
    //std::cout << recorded_sofar << " ";
    //we want 1/24 second of data
    SDL_AudioSpec spec_i;
    SDL_GetAudioStreamFormat(rec, &spec_i, NULL);
    const int want_size = (SDL_AUDIO_FRAMESIZE(spec_i) * spec_i.freq) * cycle_time; //want 1/24 second

    if(recorded_sofar > want_size) {
      //got enough - request it into a buffer

      Uint8 *buf = new Uint8[want_size];
      const int bytes_read = SDL_GetAudioStreamData(rec, buf, want_size);

      //---- audio processing section -----------------------------------------

      //TODO: either force an audio spec or adapt to what comes in. currently
      //receiving SDL_AUDIO_S16LE. 

      const Uint8 *start = buf;
/*
      const Uint8 *end = start + bytes_read;
      std::cout << int(end - start) << "<-- size \n\n\n\n\n\n\n\n" << std::endl;
      for(int i=0; i<bytes_read; i+=4) {
        std::cout << std::setw(4) << int(buf[i+0]) << " " << std::flush;
        std::cout << std::setw(4) << int(buf[i+1]) << " " << std::flush;
        std::cout << std::setw(4) << int(buf[i+2]) << " " << std::flush;
        std::cout << std::setw(4) << int(buf[i+3]) << " " << std::flush;
        std::cout << std::endl;
      }
*/

      static int cycle=0;      

      const float *fs = (const float *)buf;
      const float *fe = fs + bytes_read / 4;
      
      static const float PCM_MAX = 32767.0;
      static const float PCM_MAX_SQRT = std::sqrt(PCM_MAX);
      static const float PCM_MAX_LOG = std::log(PCM_MAX);
      float l1, l2, l3 = 0;

      for( ; fs < fe; ++fs) {
        int16_t pcm = std::abs(*fs * PCM_MAX); //pcm now holds raw PCM data!
        
        l1 = pcm / PCM_MAX;
        l2 = std::sqrt(pcm) / PCM_MAX_SQRT;
        static const int rms_max = 200;
        static float rms_arr[rms_max];
        static int rms_pos = 0;
        rms_arr[rms_pos] = pcm*pcm;
        rms_pos++;
        rms_pos %= rms_max;

        float level = 0;
        for(const float &f : rms_arr) {
          level += f;
        }
        l3 = std::sqrt((1.0/float(rms_max)) * level) / 32767.0;

        //std::cout << l1 << " " << std::flush;
      }
      int o_on = 60 * l3;
      int o_off = 60 - o_on;
      std::cout << "[";
      for( ; o_on > 0; o_on--) { std::cout << '!';}
      for( ; o_off > 0; o_off--) { std::cout << ' '; }
      std::cout << "]" << l3 << " " << o_on << " " << o_off << "\r" << std::flush;
      //std::cout << "[" << std::setfill(' ') << std::setw(o_off) << std::setfill('|') << std::setw(o_on) << "]\r" << std::flush;
  

      //-----------------------------------------------------------------------

      //now delete the buffer, we're done with it
      delete[] buf;
    }



    //std::cout << " [REC ]        \r" << std::flush;
    //now, sleep
    //24 cycles per second
    SDL_Delay(tick_wait());

  }


  //take 2 - a success!
  while(!quit) {

    //event handling loop - get user inputs
    while(SDL_PollEvent(&e) != 0) {
      //check to see if the window was closed out
      if(e.type == SDL_EVENT_QUIT) { quit = true; }
    }

    
    //grab 5 seconds of audio here, then play it back. take it nice and slow.
    if(state == RECORDING) {
      std::cout << SDL_GetAudioStreamAvailable(rec) << "[REC ]\r" << std::flush;
      SDL_AudioSpec spec_i;
      SDL_GetAudioStreamFormat(rec, &spec_i, NULL);
      const int want_size = (SDL_AUDIO_FRAMESIZE(spec_i) * spec_i.freq) * cycle_time; //get 5 seconds
      const int recorded_sofar = SDL_GetAudioStreamAvailable(rec); //how much it's accumulated so far

      if(recorded_sofar > want_size) {
        //got enough
        SDL_PauseAudioStreamDevice(rec);
        
        //pass it to playback stream
        SDL_AudioSpec spec_i;
        SDL_GetAudioStreamFormat(rec, &spec_i, NULL);
        const int want_size = (SDL_AUDIO_FRAMESIZE(spec_i) * spec_i.freq) * cycle_time; //look for 5 seconds
        Uint8 *buf = new Uint8[want_size];
        const int bytes_read = SDL_GetAudioStreamData(rec, buf, want_size); //try to get want_size bytes, but record how many we actually get
        SDL_PutAudioStreamData(pbk, buf, bytes_read); //load it into playback buffer

        SDL_ClearAudioStream(rec); //wipe any EXTRA data we didnt use
        delete[] buf; //clean up

        //start playback
        SDL_FlushAudioStream(pbk); //let it know we're done recording
        SDL_ResumeAudioStreamDevice(pbk);

        state = PLAYBACK;
      }
    }
    else if(state == PLAYBACK) {
      std::cout << SDL_GetAudioStreamAvailable(pbk) << "[PLAY]\r" << std::flush;
      if(SDL_GetAudioStreamAvailable(pbk) == 0) {
        //all done playing
        SDL_PauseAudioStreamDevice(pbk);
        SDL_ResumeAudioStreamDevice(rec);
        state = RECORDING;
       }      
   }    


    //24 cycles per second
    SDL_Delay(tick_wait());

    std::cout << cycle_count++ << "\r" << std::flush;
  }


  //SKIPPING THIS WHOLE LOOP FOR NOW. don't feel like dealing with it, gonna try playback instead.
  while(!quit) {


    //event handling loop - get user inputs
    while(SDL_PollEvent(&e) != 0) {
      //check to see if the window was closed out
      if(e.type == SDL_EVENT_QUIT) { quit = true; }
    }


    //24 cycles per second
    SDL_Delay(tick_wait());

    //skip the rest...
    continue; 

    //extract some data... if we have enough
    std::cout << SDL_GetAudioStreamAvailable(rec) << " " << get_size << std::endl;
    if(SDL_GetAudioStreamAvailable(rec) > get_size) {
      //we have enough to process now, let's read it
      //first, a buffer
      Uint8 *buf = new Uint8[get_size];
      //now, fill it
      int bytes_read = SDL_GetAudioStreamData(rec, buf, sizeof(buf));
      //trash the rest
      SDL_ClearAudioStream(rec);


      //process data in buf
      const float *fs = (const float *)buf;
      const float *fe = fs + bytes_read / 4;
      
      static const float PCM_MAX = 32767.0;
      static const float PCM_MAX_SQRT = std::sqrt(PCM_MAX);
      static const float PCM_MAX_LOG = std::log(PCM_MAX);

      for( ; fs < fe; ++fs) {
        int16_t pcm = std::abs(*fs * PCM_MAX); //pcm now holds raw PCM data!
        
        l1 = pcm / PCM_MAX;
        l2 = std::sqrt(pcm) / PCM_MAX_SQRT;
        static const int rms_max = 200;
        static float rms_arr[rms_max];
        static int rms_pos = 0;
        rms_arr[rms_pos] = pcm*pcm;
        rms_pos++;
        rms_pos %= rms_max;

        float level = 0;
        for(const float &f : rms_arr) {
          level += f;
        }
        l3 = std::sqrt((1.0/float(rms_max)) * level) / 32767.0;

      }

      //clean up
      delete[] buf;


    }
    std::cout << "\n\n\n\n" << cycle_count << "\nRAW:  " << l1 << "\nSQRT: " << l2 << "\nRMS:  " << l3 << "\n" << std::flush;
    if(cycle_count++ > cycle) { quit = true; }
  }

  //---- clean up -------------------------------------------------------------
  SDL_PauseAudioStreamDevice(rec);
  SDL_DestroyAudioStream(rec);
  SDL_free(devices_i);
  SDL_free(devices_o);
  SDL_Quit();
  
  return(0);
}
