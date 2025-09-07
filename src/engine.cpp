#include "src/engine.h"
#include <chrono>
#include <algorithm>
#include <iostream>
#include "src/images/image.h"
#include <iomanip>
#include <bitset>
#include <cmath>
#include "src/audio/audio.h"
#include "src/doll/doll.h"
#include "src/actions/action.h"
#include "src/screenwatch/screenwatch.h"

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
  image torso("./resources/control/sona_tuber_draw_head_base.txt", &r);
  image penhand("./resources/control/sona_tuber_draw_penhand.txt", &r);
  image bookhand("./resources/control/sona_tuber_draw_bookhand.txt", &r);

/*
  //ok, try to init an audio now
  std::vector<std::pair<int, std::string>> devices;
  audio::get_devices(devices);

  //TODO: eventually, show users their devices here. for now, just print them
  //and then pick the first one.
  for(auto d : devices) {
    std::cout << d.first << " " << d.second;
    std::cout << std::endl;
  }
*/

  audio a_rms(
//      0             //device id
//    , 44100/50      //sampling frequency (per second)
//    , 4096/512      //samples before a callback
//    , 1             //1 input channel
//    , 300           //300ms RMS audio interval
    /*,*/ audio::RMS    //use RMS audio processing
  );

//  audio a_rmslog(
//      0             //device id
//    , 44100/50      //sampling frequency (per second)
//    , 4096/513      //samples before a callback
//    , 1             //1 input channel
//    , 300           //300ms RMS audio interval
//    /*,*/ audio::RMSLOG //use RMSLOG audio processing
//  );

  if(! a_rms.is_init() ) { 
    std::cout << "S-H-I-T!" << SDL_GetError() << std::endl; 
    return; 
  }

  //let's see if we can get dolls working
  dollpart dp_torso("./resources/control/sona_tuber_draw_head_base.txt", &r);
  
  //dp_torso.pin_to(-200,-1000,NULL);
  
  dp_torso.pin_to(500, 90, NULL);
  dp_torso.set_scale(720.0/2800.0);
  
  //how about an action?
  
  //wobble constantly back and forth
  dp_torso.add_action(new act_sinefloat(-1, action::UP_CONST, action::AXIS_X, 30.0, 0.013, act_sinefloat::SFTYPE_SF));
  dp_torso.add_action(new act_sinefloat(-1, action::UP_CONST, action::AXIS_Y, 30.0, 0.008, act_sinefloat::SFTYPE_SF));

  //bounce once when starting to speak
  //dp_torso.add_action(new act_sinefloat(0.0048, action::UP_PULSE, action::AXIS_Y, -200.0, 0.1, act_sinefloat::SFTYPE_BN));
  //bounce continuously while speaking
  //TODO: replace this with an opacity change: mouth open/closed
  dp_torso.add_action(new act_sinefloat(0.0048, action::UP_CONST, action::AXIS_Y, 20.0, 0.3, act_sinefloat::SFTYPE_SF));

  dollpart dp_mouth_closed("./resources/control/sona_tuber_draw_head_mouth_closed.txt", &r);
  dp_mouth_closed.pin_to(362, 1117, &dp_torso);
  dp_mouth_closed.add_action(new act_hide(0.0048, action::UP_CONST));

  dollpart dp_mouth_open("./resources/control/sona_tuber_draw_head_mouth_open.txt", &r);
  dp_mouth_open.pin_to(353, 1119, &dp_torso);
  dp_mouth_open.add_action(new act_hide(0.0048, action::DN_CONST));

  dollpart dp_penhand("./resources/control/sona_tuber_draw_penhand.txt", &r);
  dp_penhand.pin_to(295, 1795, &dp_torso);

  //scribble randomly
  dp_penhand.add_action(new act_sinefloat(-1, action::UP_CONST, action::AXIS_X, 32.5, 0.084, act_sinefloat::SFTYPE_SF));
  dp_penhand.add_action(new act_sinefloat(-1, action::UP_CONST, action::AXIS_Y, 53.8, 0.056, act_sinefloat::SFTYPE_SF));


  dollpart dp_bookhand("./resources/control/sona_tuber_draw_bookhand.txt", &r);
  dp_bookhand.pin_to(-95, 1835, &dp_torso);

  //follow the pen at 1/10 speed
  dp_bookhand.add_action(new act_sinefloat(-1, action::UP_CONST, action::AXIS_X, 3.25, 0.084, act_sinefloat::SFTYPE_SF));
  dp_bookhand.add_action(new act_sinefloat(-1, action::UP_CONST, action::AXIS_Y, 5.38, 0.056, act_sinefloat::SFTYPE_SF));


  //now let's try to get the screenwatcher running
  screenwatch sw;
 
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
      if(e.type == SDL_EVENT_QUIT) { quit = true; }
    }

    // -   DEBUGGING SECTION   -   -   -   -   -   -   -   -   -   -   -   -
/*
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
*/
    a_rms.update();
    //a_rmslog.update();

    dp_torso.update(a_rms.get_level());
    dp_mouth_closed.update(a_rms.get_level());
    dp_mouth_open.update(a_rms.get_level());
    dp_penhand.update();
    dp_bookhand.update();

    dp_torso.draw();
    dp_mouth_closed.draw();
    dp_mouth_open.draw();
    dp_penhand.draw();
    dp_bookhand.draw();

    //show the focused window (it works!)
    std::string sw_out = "";
    sw.get_screen_title(&sw_out);
    //std::cout << sw_out << "\r" << std::flush;

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
  if(SDL_InitSubSystem(SDL_INIT_AUDIO) == false) {
    //error and die, can't have pngtuber without audio
    std::cout << "S-H-I-T! couldn't init audio! error: " << SDL_GetError();
    is_init = false;
  }

  //video subsystems
  if(SDL_InitSubSystem(SDL_INIT_VIDEO) == false) {
    //error and die, can't have pngtuber without video
    std::cout << "S-H-I-T! couldn't init video! error: " << SDL_GetError();
    is_init = false;
  }
/*
  //no longer needed w/ SDL3
  else if(!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
    //error and die, need the PNGs for a pngtuber
    std::cout << "S-H-I-T! couldn't init pngs! error: " << SDL_GetError();
    is_init = false;
  }
*/
  
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
  //no longer needed w/ SDL3
  //IMG_Quit();
  SDL_Quit();
}
