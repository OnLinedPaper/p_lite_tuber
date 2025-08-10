#include "src/engine.h"
#include <chrono>
#include <algorithm>
#include <iostream>
#include "src/images/image.h"
#include <iomanip>
#include <bitset>
#include <cmath>
#include "src/audio/audio.h"

//-----------------------------------------------

//main event loop, everything happens in here
void engine::play() { 
  //don't run if not initialized - this will usually happen if an SDL
  //subsystem failed to init for any reason
  if(!is_init) { return; }

  SDL_Event e;
  bool quit = false;

  //   -DEBUGGING SECTION  -   -   -   -   -   -   -   -   -   -   -   -   -   
  
  //create a debugging image
  image i("./resources/control/wisp_yellow.txt", &r);
  image torso("./resources/control/sona_tuber_draw_head.txt", &r);
  image penhand("./resources/control/sona_tuber_draw_penhand.txt", &r);
  image bookhand("./resources/control/sona_tuber_draw_bookhand.txt", &r);

  //ok, try to init an audio now
  std::vector<std::pair<int, std::string>> devices;
  audio::get_devices(devices);

  //TODO: eventually, show users their devices here. for now, just print them
  //and then pick the first one.
  for(auto d : devices) {
    std::cout << d.first << " " << d.second;
    std::cout << std::endl;
  }

  audio a_rms(
      0             //device id
    , 44100/50      //sampling frequency (per second)
    , 4096/512      //samples before a callback
    , 1             //1 input channel
    , 300           //300ms RMS audio interval
    , audio::RMS    //use RMS audio processing
  );

  audio a_rmslog(
      0             //device id
    , 44100/50      //sampling frequency (per second)
    , 4096/512      //samples before a callback
    , 1             //1 input channel
    , 300           //300ms RMS audio interval
    , audio::RMSLOG //use RMSLOG audio processing
  );

  if(! a_rms.is_init() ) { 
    std::cout << "S-H-I-T!" << SDL_GetError() << std::endl; 
    return; 
  }


  if(false) {
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

    //draw a little fire on the screen, and make it bounce
    int p_x = r.get_w();
    int p_y = r.get_h() - (r.get_h() / 10) - i.get_h();
    int p_y_rms = (r.get_h() - 2 * (r.get_h() / 10) - i.get_h()) * a_rms.get_level() * 20;
    int p_y_rms_l = (r.get_h() - 2 * (r.get_h() / 10) - i.get_h()) * a_rms.get_level() * 20;

    int p_y_rmslog = (r.get_h() - 2 * (r.get_h() / 10) - i.get_h()) * a_rmslog.get_level();

    i.draw(p_x * 1 / 10, p_y);
    i.draw(p_x * 2 / 10, p_y - p_y_rms);
    i.draw(p_x * 3 / 10, p_y - std::log(p_y_rms) * 40 + 40);
    i.draw(p_x * 4 / 10, p_y - p_y_rms_l);
    i.draw(p_x * 5 / 10, p_y - p_y_rmslog);

    //std::cout << "            " << a_rms.get_level() << "\r" << std::flush;


    //ok, now let's see how the tuber looks
    //really... really fucking big. okay. hm. need to scale this.
    //better! now to scale the other parts too

    //now let's see about making the parts move in sync, and have the
    //limbs move relative to the torso rather than independent of it
    //TODO: something that lets me position/reload the parts at runtime
    //rather than recompiling every time i want to make a change
    //TODO: convert all these absolute limb positions into relative
    //positions, i.e. "50% down the body" and the like so they scale properly
    float scale = 720.0/2800.0;
    int x_base_torso = 500;
    int y_base_torso = 190;

    //how about some drift? we already have sin/cos/tan from cmath, might as
    //well make some use of them

    static float drift_factor = 0;
    drift_factor++;

    //remember that sin/cos measure in radians
    int x_torso = x_base_torso + 8.0 * std::sin(drift_factor/45);
    int y_torso = y_base_torso + 8.0 * std::sin(drift_factor/66);

    //how about we make the pen and book move a bit? let's say pen moves a lot
    //and book moves a tiny bit relative to it
    //pen vs book: 
    //tlc: -50, -55
    //brc: +50, +55

    int x_deflection_penhand = 50.0 * std::sin(drift_factor/9) / 6;
    int y_deflection_penhand = 55.0 * std::sin(drift_factor/11) / 4;

    torso.draw(x_torso, y_torso, scale);
    //penhand.draw(x_torso + 50, y_torso + 310, scale);
    penhand.draw(x_torso + 50 + x_deflection_penhand, y_torso + 310 + y_deflection_penhand, scale);
    bookhand.draw(x_torso - 50 + (0.1 * x_deflection_penhand), y_torso + 320 + (0.1 * y_deflection_penhand), scale);

    // -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -



    r.show();

    //stick something in the term so we have a visual indicator that we're
    //still ticking
    print_debug_swirly();

    //go to sleep for a bit
    SDL_Delay(tick_wait());
  }

}

engine::engine() : is_init(true) { 
  //as an exercise to myself, i am going to deviate from qdbp's design and
  //deliberately AVOID singletons everywhere i can. 
  //therefore, we'll init the vars here instead.

  //audio subsystems
  if(SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
    //error and die, can't have pngtuber without audio
    std::cout << "S-H-I-T! couldn't init audio! error: " << SDL_GetError();
    is_init = false;
  }

  //video subsystems
  if(SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
    //error and die, can't have pngtuber without video
    std::cout << "S-H-I-T! couldn't init video! error: " << SDL_GetError();
    is_init = false;
  }
  else if(!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
    //error and die, need the PNGs for a pngtuber
    std::cout << "S-H-I-T! couldn't init pngs! error: " << SDL_GetError();
    is_init = false;
  }

  
  //now init other stuff
  r.init();
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
