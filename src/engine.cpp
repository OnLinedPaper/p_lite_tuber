#include "src/engine.h"
#include <chrono>
#include <algorithm>
#include <iostream>
#include "src/images/image.h"
#include <iomanip>
#include <bitset>
#include <cmath>


//TODO: move this whole section to its own class

//buffer to hold recording
uint8_t *rec_buf = NULL;
int rec_buf_size = 0;       //size of buffer
int rec_buf_pos = 0;        //buffer position
int rec_buf_pos_max = 0;    //max buffer position
bool rec = false;

uint8_t *stream_buf = NULL;   //buffer for streaming - runs in a loop
int stream_buf_size = 0;      //max stream buffer size
int stream_buf_pos = 0;       //position in stream buffer
int stream_buf_pos_max = 0;   //end of stream buffer

int SMPL_FREQ = 44100/50;  //low sample rate
int SMPL_COUNT = 4096/512;//128; //rapid write-to-buffer

void audio_stream_callback(void *userdata, uint8_t *stream, int len) {
  if (userdata) {}
  //run through the stream, in a loop

  //run this one through float
  //https://stackoverflow.com/a/45864513/7431860
  //converting it lets me see it as needed!
  //TODO: check how to measure volume. raw? square root? log? it looks like
  //square root's going to be the best solution here, but i need to test more.
  const float *fs = (const float *)stream;
  const float *fe = fs + len / 4;
  for( ; fs < fe; ++fs) {
    static int16_t mx = 0;
    int16_t i = *fs * 32767;
    if(i > mx) { mx = i; }
    std::cout << "STREAM " << std::setw(7) << std::abs(i);
    std::cout << std::setw(8) << std::fixed << std::setprecision(2) << std::sqrt(std::abs(i));
    std::cout << std::setw(6) << std::fixed << std::setprecision(2) << std::log(std::abs(i));
    //now do a little "volume meter"
    std::cout << "  [";
    std::string s = "";
    for(int j=0; j<std::min(std::sqrt(std::abs(i)), 48.0); j++) {
      s += "o";
    }
    std::cout << std::left << std::setw(48) << s << std::right;
    std::cout << "]    " << /*mx <<*/ "\r";
  }

  //dump buffer
  for(int i=0; i<len; i+=4) {
    if(i%(4*2) == 0 && false) {
      std::cout << "STREAM ";
      //print raw stream data in its original uint8_t format
      //print binary here
      std::cout << std::setw(9) << std::bitset<8>(stream[i+0]) << " "; 
      std::cout << std::setw(9) << std::bitset<8>(stream[i+1]) << " "; 
      std::cout << std::setw(9) << std::bitset<8>(stream[i+2]) << " "; 
      std::cout << std::setw(9) << std::bitset<8>(stream[i+3]) << " "; 
      //print decimal here
      std::cout << std::setw(4) << unsigned(stream[i+0]) << " ";
      std::cout << std::setw(4) << unsigned(stream[i+1]) << " ";
      std::cout << std::setw(4) << unsigned(stream[i+2]) << " ";
      std::cout << std::setw(4) << unsigned(stream[i+3]) << " ";
  //    std::cout << std::setw(9) << std::bitset<8>(stream[i+4]) << " "; 
  //    std::cout << std::setw(9) << std::bitset<8>(stream[i+5]) << " "; 
  //    std::cout << std::setw(9) << std::bitset<8>(stream[i+6]) << " "; 
  //    std::cout << std::setw(9) << std::bitset<8>(stream[i+7]) << " "; 
      std::cout << "\r";
    }
    if(i%(4*2) == 0 && false) {
      //convert data to float (32bit) and print it here
      std::cout << "STREAM ";
      std::cout << std::setw(11) << float(stream[i]);
      std::cout << "\r";
    }
  }
}
   
void audio_rec_callback(void *userdata, uint8_t *stream, int len) {
  if (userdata) {}
  //copy into buf from stream
  memcpy(&rec_buf[rec_buf_pos], stream, len);
  rec_buf_pos += len;

  //...dump the buffer? let's see how this looks
  //seems to be a repeating set of 4 numbers, let's try that
  //since these are between 0 adn 255, i'll setw to 4
  //that fourth number seems to be what i'm after! it's bounding when i talk. let's
  //see what it looks like in binary.
  //i'm missing something. the fourth number only jumps between 59-60 and
  //180-189. these SEEM to be related to when i'm talking or not talking, but
  //don't seem to relate to volume at all. where else should i be looking...?
  for(int i=0; i<len; i+=4) {
    if(i%(4*2) == 0) {
      std::cout << "REC  ";
      std::cout << std::setw(9) << std::bitset<8>(stream[i+0]) << " "; 
      std::cout << std::setw(9) << std::bitset<8>(stream[i+1]) << " "; 
      std::cout << std::setw(9) << std::bitset<8>(stream[i+2]) << " "; 
      std::cout << std::setw(9) << std::bitset<8>(stream[i+3]) << " "; 
  //    std::cout << std::setw(9) << std::bitset<8>(stream[i+4]) << " "; 
  //    std::cout << std::setw(9) << std::bitset<8>(stream[i+5]) << " "; 
  //    std::cout << std::setw(9) << std::bitset<8>(stream[i+6]) << " "; 
  //    std::cout << std::setw(9) << std::bitset<8>(stream[i+7]) << " "; 
      std::cout << std::endl;
    }
  }
}

void audio_play_callback(void *userdata, uint8_t *stream, int len) {
  if (userdata) {}
  //copy into stream from buf
  memcpy(stream, &rec_buf[rec_buf_pos], len);
  rec_buf_pos += len;

  //dump buffer
  for(int i=0; i<len; i+=4) {
    if(i%(4*2) == 0) {
      std::cout << "PLAY ";
      std::cout << std::setw(9) << std::bitset<8>(stream[i+0]) << " "; 
      std::cout << std::setw(9) << std::bitset<8>(stream[i+1]) << " "; 
      std::cout << std::setw(9) << std::bitset<8>(stream[i+2]) << " "; 
      std::cout << std::setw(9) << std::bitset<8>(stream[i+3]) << " "; 
      std::cout << std::setw(4) << unsigned(stream[i+0]) << " ";
      std::cout << std::setw(4) << unsigned(stream[i+1]) << " ";
      std::cout << std::setw(4) << unsigned(stream[i+2]) << " ";
      std::cout << std::setw(4) << unsigned(stream[i+3]) << " ";
  //    std::cout << std::setw(9) << std::bitset<8>(stream[i+4]) << " "; 
  //    std::cout << std::setw(9) << std::bitset<8>(stream[i+5]) << " "; 
  //    std::cout << std::setw(9) << std::bitset<8>(stream[i+6]) << " "; 
  //    std::cout << std::setw(9) << std::bitset<8>(stream[i+7]) << " "; 
      std::cout << std::endl;
    }
  }
}
//-----------------------------------------------

//main event loop, everything happens in here
void engine::play() { 

  SDL_Event e;
  bool quit = false;

  //   -DEBUGGING SECTION  -   -   -   -   -   -   -   -   -   -   -   -   -   
  
  image i("./resources/control/wisp_yellow.txt", &r);

  //time to take a crack at audio. first things first, let's inspect available devices.
  //first, init audio
  if(SDL_Init(SDL_INIT_AUDIO) < 0) {
    //error and die
    std::cout << "S-H-I-T! couldn't init audio! error: " << SDL_GetError();
    return;
  }

  int dev_audio_in = SDL_GetNumAudioDevices(SDL_TRUE); //FALSE for playback, TRUE for recording
  int dev_audio_out = SDL_GetNumAudioDevices(SDL_FALSE); //FALSE for playback, TRUE for recording
  std::cout << "SDL detected " << dev_audio_in << " audio input device(s) and " << dev_audio_out << " audio output devices" << std::endl;

  if(!(dev_audio_in > 0 || dev_audio_out > 0)) {
    std::cout << "none detected - returning" << std::endl;
    return;
  }

  std::cout << "\navailable recording devices: " << std::endl;
  for(int i=0; i<dev_audio_in; i++) {
    std::cout << i << ": " << SDL_GetAudioDeviceName(i, SDL_TRUE) << std::endl;
  }

  std::cout << "\navailable playback devices: " << std::endl;
  for(int i=0; i<dev_audio_out; i++) {
    std::cout << i << ": " << SDL_GetAudioDeviceName(i, SDL_FALSE) << std::endl;
  }

  //alright, devices are listed. let's skip picking them for now and just
  //assume it's "0" "0" for the default devices - in this case, my webcam and
  //soundcard. we'll record some audio and play it back.

  //init the recording spec. start with 44100 and then later try to pare it down
  SDL_AudioSpec desired_spec_in;
  SDL_zero(desired_spec_in);
  desired_spec_in.freq = SMPL_FREQ;
  desired_spec_in.format = AUDIO_F32;
  desired_spec_in.channels = 1;
  desired_spec_in.samples = SMPL_COUNT;
  desired_spec_in.callback = audio_rec_callback;

  SDL_AudioSpec actual_spec_in;
  SDL_zero(actual_spec_in);

  //try opening the device
  int audio_rec_dev_id = SDL_OpenAudioDevice(SDL_GetAudioDeviceName(0, SDL_TRUE), SDL_TRUE, &desired_spec_in, &actual_spec_in, SDL_AUDIO_ALLOW_FORMAT_CHANGE);

  if(audio_rec_dev_id == 0) {
    std::cout << "S-H-I-T! couldn't open audio input device! error: " << SDL_GetError();
    return;
  }

  //display data
  std::cout << "\ncomparing requested / received spec..." << std::endl;
  std::cout << "frequency: " << desired_spec_in.freq << " / " << actual_spec_in.freq << std::endl;
  std::cout << "format: " << desired_spec_in.format << " / " << actual_spec_in.format << std::endl;
  std::cout << "channels: " <<desired_spec_in.channels << " / " << actual_spec_in.channels << std::endl;
  std::cout << "samples: " << desired_spec_in.samples << " / " << actual_spec_in.samples << std::endl;

  //calculate bytes per sample
  int b_per_sample = actual_spec_in.channels * (SDL_AUDIO_BITSIZE(actual_spec_in.format) / 8);
  int b_per_second = actual_spec_in.freq * b_per_sample;
  rec_buf_size = 5 * b_per_second;      //make space for 5 seconds of audio...
  rec_buf_pos_max = 4 * b_per_second;   //...but record only 4, to be safe

  std::cout << "\nchecking buffer sizes..." << std::endl;
  std::cout << "bits per sample: " << b_per_sample << std::endl;
  std::cout << "bits per second: " << b_per_second << std::endl;
  std::cout << "buf size: " << rec_buf_size << std::endl;
  std::cout << "buf max pos: " << rec_buf_pos_max << std::endl;

  //allocate buffer
  rec_buf = new uint8_t[rec_buf_size];
  memset(rec_buf, 0, rec_buf_size);

  //grab some data. start by unpausing the device.
  rec_buf_pos = 0;
  SDL_PauseAudioDevice(audio_rec_dev_id, SDL_FALSE);

  std::cout << "init capture..." << std::endl;

  //spin while recording
  rec = false;//true;
  while(rec) { 
    //gotta block the audio device to ensure the callback (which runs in
    //another thread) isn't concurrently accessing the variable
    SDL_LockAudioDevice(audio_rec_dev_id);
    if(rec_buf_pos >= rec_buf_pos_max) { rec = false; }
    SDL_UnlockAudioDevice(audio_rec_dev_id);
  }
  SDL_PauseAudioDevice(audio_rec_dev_id, SDL_TRUE);
  SDL_CloseAudioDevice(audio_rec_dev_id);

  //now we've got some data! play it back
  //first, open the output device as well. i don't think i'm going to open
  //an actual output channel in the final program; this is just sort of...
  //curiosity? i want to hear what happens to the audio when i chop the 
  //frequency and sampling rate.


  //init the playback spec. start with 44100 and then later try to pare it down
  SDL_AudioSpec desired_spec_out;
  SDL_zero(desired_spec_out);
  desired_spec_out.freq = SMPL_FREQ;
  desired_spec_out.format = AUDIO_F32;
  desired_spec_out.channels = 1;
  desired_spec_out.samples = SMPL_COUNT;
  desired_spec_out.callback = audio_play_callback;

  SDL_AudioSpec actual_spec_out;
  SDL_zero(actual_spec_out);

  //try opening the device
  int audio_rec_dev_out = SDL_OpenAudioDevice(SDL_GetAudioDeviceName(0, SDL_FALSE), SDL_FALSE, &desired_spec_out, &actual_spec_out, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
 
  if(audio_rec_dev_out == 0) {
    std::cout << "S-H-I-T! couldn't open audio output device! error: " << SDL_GetError();
    return;
  }

  //skipping the stat calculation and dump for now, and just initiating the
  //playback immediately instead
  rec_buf_pos = 0;
  SDL_PauseAudioDevice(audio_rec_dev_out, SDL_FALSE);
  rec = false;//true;

  std::cout << "init playback..." << std::endl;

  while(rec) {
    SDL_LockAudioDevice(audio_rec_dev_out);
    if(rec_buf_pos >= rec_buf_pos_max) { rec = false; }
    SDL_UnlockAudioDevice(audio_rec_dev_out);
  }
  SDL_PauseAudioDevice(audio_rec_dev_out, SDL_TRUE);
  SDL_CloseAudioDevice(audio_rec_dev_out);

  delete[] rec_buf;
  rec_buf = NULL;

  //now, let's attempt realtime playback. rather than attempt to wipe the 
  //buffer after every use, we'll attempt to use it circularly, wrapping
  //back around to the beginning once it hits the end for both recording
  //and playback. 
  //we'll allocate... let's say, 1/4 of any given frame to audio recording,
  //and then the remaining time will be used to process the audio and do other
  //stuff like draw things on the screen. 

  SDL_AudioSpec spec_stream_des;
  SDL_zero(spec_stream_des);
  spec_stream_des.freq = SMPL_FREQ;
  spec_stream_des.format = AUDIO_F32;
  spec_stream_des.channels = 1;
  spec_stream_des.samples = SMPL_COUNT;
  spec_stream_des.callback = audio_stream_callback;

  SDL_AudioSpec spec_stream_act;
  SDL_zero(spec_stream_act);
  
  //open mic for streaming. no playback - just audio bumping
  int audio_dev_stream_id = SDL_OpenAudioDevice(SDL_GetAudioDeviceName(0, SDL_TRUE), SDL_TRUE, &spec_stream_des, &spec_stream_act, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
  if(audio_dev_stream_id == 0) {
    std::cout << "S-H-I-T! couldn't open audio input device! error: " << SDL_GetError();
    return;
  }

  //using the actual spec, calculate the bytes per sample and bytes needed
  //just kidding, actually. we're not storing the sound, so let's just dump it
  //immediately. 
  std::cout << "\nMANIP=     RAW    SQRT   LOG  VISUAL (SQRT)" << std::endl;
  SDL_PauseAudioDevice(audio_dev_stream_id, SDL_FALSE);
  std::cin.get();
  SDL_PauseAudioDevice(audio_dev_stream_id, SDL_TRUE);
  SDL_CloseAudioDevice(audio_dev_stream_id);

  if(true) {
    std::cout << "\ndebugging finished - returning" << std::endl;
    return;
  }
  
  //   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   

  //main loop - everything that happens, happens in here
  while(!quit) {

    //event handling loop - get user inputs
    while(SDL_PollEvent(&e) != 0) {
      //check to see if the window was closed out
      if(e.type == SDL_QUIT) { quit = true; }
    }

    // -   DEBUGGING SECTION   -   -   -   -   -   -   -   -   -   -   -   -

    static int d_i = 0;
    d_i++;
    int d_x, d_y;
    d_x = (((d_i)*3) % (r.get_w() + i.get_w())) - i.get_w();
    d_y = (((d_i)*2) % (r.get_h() + i.get_h())) - i.get_h();
    i.draw(d_x, d_y);

    // -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

    r.show();

    //stick something in the term so we have a visual indicator that we're
    //still ticking
    print_debug_swirly();

    //go to sleep for a bit
    SDL_Delay(tick_wait());
  }

}

engine::engine() { 
  //as an exercise to myself, i am going to deviate from qdbp's design and
  //deliberately AVOID singletons everywhere i can. 
  //therefore, we'll init the vars here instead.
}

double engine::tick_wait() {
  //first, get the current tick
  static double last_tick = 0;
  double curr_tick = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch()
  ).count();

  double elapsed = curr_tick - last_tick;

  return std::max(0.0, T_DELAY - elapsed);
}

void engine::print_debug_swirly() const {
  static int k = 0;
  int width = 8;  
  int delay = 256;

  k++;
  k = k % delay;
  if(k % (delay/width) != 0) { return; }

  int i = k / (delay/width);
  std::string s = "";
  s+= "[";
  for(int j=0; j<i; j++) { s += " "; }
  s += "o";
  for(int j=0; j<width-i-1; j++) { s += " "; }
  s+= "]\r";
  std::cout << s << std::flush;
}

engine::~engine() {
  IMG_Quit();
  SDL_Quit();
}
