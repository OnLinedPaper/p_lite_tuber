#include "src/engine.h"
#include <chrono>
#include <algorithm>
#include <iostream>
#include "src/images/image.h"
#include <iomanip>
#include <bitset>
#include <cmath>
#include <cstdlib>
#include "src/audio/audio.h"
#include "src/doll/doll.h"
#include "src/actions/action.h"
#include "src/screenwatch/screenwatch.h"
#include "src/events/event.h"

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

  std::vector<std::string> krita_titles;
  krita_titles.push_back(".kra");
  krita_titles.push_back(".png");
  krita_titles.push_back("Saving as");
  krita_titles.push_back("Not Saved");

  std::vector<std::string> discord_titles;
  discord_titles.push_back("Discord");

  std::vector<std::string> videogame_titles;
  videogame_titles.push_back("Deep Rock Galactic");
  videogame_titles.push_back("Warframe");
  videogame_titles.push_back("PEAK");
  videogame_titles.push_back("ENA-4-DreamBBQ");

  std::vector<std::string> firefox_titles;
  firefox_titles.push_back("Mozilla Firefox");

  //catch-all to make the phone appear
  std::vector<std::string> phone_titles;
  phone_titles.insert(phone_titles.end(), discord_titles.begin(), discord_titles.end());
  phone_titles.insert(phone_titles.end(), firefox_titles.begin(), firefox_titles.end());
 
  float vocal_threshold = 0.004;

  //let's see if we can get dolls working
  dollpart dp_torso("./resources/control/sona_tuber_draw_head_base.txt", &r);
  
  //debugging pins
  //dp_torso.pin_to(-200,-800,NULL);
  //dp_torso.pin_to(-800,-2400,NULL);
  
  //default pin
  dp_torso.pin_to(500, 90, NULL);
  dp_torso.set_scale(720.0/2800.0);

  dollpart dp_torso_bb("./resources/control/sona_tuber_draw_head_base_bb.txt", &r);
  dp_torso_bb.pin_to(0, 0, &dp_torso);
  
  //how about an action?
  
  //wobble constantly back and forth
  dp_torso.add_action(new act_sinefloat(-1, action::UP_CONST, action::AXIS_X, 30.0, 0.026, act_sinefloat::SFTYPE_SF));
  dp_torso.add_action(new act_sinefloat(-1, action::UP_CONST, action::AXIS_Y, 30.0, 0.016, act_sinefloat::SFTYPE_SF));

  //bounce continuously while speaking
  dp_torso.add_action(new act_sinefloat(vocal_threshold, action::UP_CONST, action::AXIS_Y, 20.0, 0.6, act_sinefloat::SFTYPE_SF));


  dollpart dp_mouth_closed("./resources/control/sona_tuber_draw_head_mouth_closed.txt", &r);
  dp_mouth_closed.pin_to(362, 1117, &dp_torso);
  dp_mouth_closed.add_action(new act_hide(vocal_threshold, action::UP_CONST));

  dollpart dp_mouth_open("./resources/control/sona_tuber_draw_head_mouth_open.txt", &r);
  dp_mouth_open.pin_to(353, 1119, &dp_torso);
  dp_mouth_open.add_action(new act_hide(vocal_threshold, action::DN_CONST));


  float blink_threshold = 0.974;

  dollpart dp_eyes_closed("./resources/control/sona_tuber_draw_head_eyes_closed.txt", &r);
  dp_eyes_closed.pin_to(360, 1004, &dp_torso);
  dp_eyes_closed.add_action(new act_hide(blink_threshold, action::DN_CONST));

  dollpart dp_eyes_open("./resources/control/sona_tuber_draw_head_eyes_open.txt", &r);
  dp_eyes_open.pin_to(357, 993, &dp_torso);
  dp_eyes_open.add_action(new act_hide(blink_threshold, action::UP_CONST));


  dollpart dp_outfit_polo("./resources/control/sona_tuber_draw_outfit_polo.txt", &r);
  dp_outfit_polo.pin_to(127, 1635, &dp_torso);

  dollpart dp_outfit_roseglasses("./resources/control/sona_tuber_draw_outfit_roseglasses.txt", &r);
  dp_outfit_roseglasses.pin_to(369, 1160, &dp_torso);


  dollpart dp_penhand("./resources/control/sona_tuber_draw_penhand.txt", &r);
  dp_penhand.pin_to(295, 1795, &dp_torso);

  //scribble randomly
  dp_penhand.add_action(new act_sinefloat(-1, action::UP_CONST, action::AXIS_X, 32.5, 0.168, act_sinefloat::SFTYPE_SF));
  dp_penhand.add_action(new act_sinefloat(-1, action::UP_CONST, action::AXIS_Y, 53.8, 0.112, act_sinefloat::SFTYPE_SF));

  //up and down
  dp_penhand.add_action(new act_move(action::AXIS_Y, 2400, 0, {2100, 200, -300}, 20, act_move::MVTYPE_ST, 1.0, action::UP_CONST));
  dp_penhand.add_action(new act_move(action::AXIS_Y, 0, 2400, {-300, 200, 2100}, 24, act_move::MVTYPE_ST, 0.9, action::DN_CONST));


  dollpart dp_bookhand("./resources/control/sona_tuber_draw_bookhand.txt", &r);
  dp_bookhand.pin_to(-95, 1835, &dp_torso);

  //follow the pen at 1/10 speed
  dp_bookhand.add_action(new act_sinefloat(-1, action::UP_CONST, action::AXIS_X, 3.25, 0.168, act_sinefloat::SFTYPE_SF));
  dp_bookhand.add_action(new act_sinefloat(-1, action::UP_CONST, action::AXIS_Y, 5.38, 0.112, act_sinefloat::SFTYPE_SF));

  //up and down
  dp_bookhand.add_action(new act_move(action::AXIS_Y, 2400, 0, {2100, 200, -300}, 16, act_move::MVTYPE_ST, 1.0, action::UP_CONST));
  dp_bookhand.add_action(new act_move(action::AXIS_Y, 0, 2400, {-300, 200, 2100}, 16, act_move::MVTYPE_ST, 0.9, action::DN_CONST));

  dollpart dp_phonehand("./resources/control/sona_tuber_draw_phonehand.txt", &r);
  dp_phonehand.pin_to(-95, 1635, &dp_torso);
  //dp_phonehand.add_action(new act_hide(1.0, action::DN_CONST));
  
  //bring the phone up 
  dp_phonehand.add_action(new act_move(action::AXIS_Y, 2400, 0, {2100, 200, -300}, 16, act_move::MVTYPE_ST, 1.0, action::UP_CONST, event_v1::PHONE_UP));
  //hide it down
  dp_phonehand.add_action(new act_move(action::AXIS_Y, 0, 2400, {-300, 200, 2100}, 16, act_move::MVTYPE_ST, 0.9, action::DN_CONST));

  //float around randomly
  dp_phonehand.add_action(new act_sinefloat(-1, action::UP_CONST, action::AXIS_X, 30.0, 0.026, act_sinefloat::SFTYPE_SF));
  dp_phonehand.add_action(new act_sinefloat(-1, action::UP_CONST, action::AXIS_Y, 30.0, 0.016, act_sinefloat::SFTYPE_SF));


  dollpart dp_phone_base("./resources/control/sona_tuber_draw_phone_base.txt", &r);
  dp_phone_base.pin_to(-1000, -850, &dp_phonehand);
  dp_phone_base.add_action(new act_hide(1.0, action::DN_CONST));
  dp_phone_base.add_action(new act_sinefloat(-1, action::UP_CONST, action::AXIS_X, -15.0, 0.026, act_sinefloat::SFTYPE_SF));
  dp_phone_base.add_action(new act_sinefloat(-1, action::UP_CONST, action::AXIS_Y, -15.0, 0.016, act_sinefloat::SFTYPE_SF));
  
  //TODO: more of this later
  //dp_phone_base.add_action(new act_move(action::AXIS_X, 0, 0, {-800, 800}, 24, 1, action::DN_CONST));


  dollpart dp_phone_bubble_discord("./resources/control/sona_tuber_draw_phone_bubble_discord.txt", &r);
  dp_phone_bubble_discord.pin_to(116, 84, &dp_phone_base);
  dp_phone_bubble_discord.add_action(new act_hide(1.0, action::DN_CONST));

  dollpart dp_phone_bubble_base("./resources/control/sona_tuber_draw_phone_bubble_base.txt", &r);
  dp_phone_bubble_base.pin_to(116, 84, &dp_phone_base);
  dp_phone_bubble_base.add_action(new act_hide(1.0, action::DN_CONST));

  dollpart dp_phone_bubble_firefox("./resources/control/sona_tuber_draw_phone_bubble_firefox.txt", &r);
  dp_phone_bubble_firefox.pin_to(116, 84, &dp_phone_base);
  dp_phone_bubble_firefox.add_action(new act_hide(1.0, action::DN_CONST));



  dollpart dp_xboxhand("./resources/control/sona_tuber_xbox.txt", &r);
  dp_xboxhand.pin_to(-200, 1800, &dp_torso);

  //float around randomly
  dp_xboxhand.add_action(new act_sinefloat(-1, action::UP_CONST, action::AXIS_X, 30.0, 0.026, act_sinefloat::SFTYPE_SF));
  dp_xboxhand.add_action(new act_sinefloat(-1, action::UP_CONST, action::AXIS_Y, 30.0, 0.016, act_sinefloat::SFTYPE_SF));

  //up and down
  dp_xboxhand.add_action(new act_move(action::AXIS_Y, 2400, 0, {2100, 200, -300}, 16, act_move::MVTYPE_ST, 1.0, action::UP_CONST));
  dp_xboxhand.add_action(new act_move(action::AXIS_Y, 0, 2400, {-300, 200, 2100}, 16, act_move::MVTYPE_ST, 0.9, action::DN_CONST));


  //now let's try to get the screenwatcher running
  screenwatch sw;
 
  if(false) {
    std::cout << "\ndebugging finished - returning" << std::endl;
    return;
  }
  
  //   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   

  bool phone_on = false;

  //main loop - everything that happens, happens in here
  while(!quit) {

    //event handling loop - get user inputs
    while(SDL_PollEvent(&e) != 0) {
      //check to see if the window was closed out
      if(e.type == SDL_EVENT_QUIT) { quit = true; }
    }

    // -   DEBUGGING SECTION   -   -   -   -   -   -   -   -   -   -   -   -

    a_rms.update();
    //a_rmslog.update();

    //this little section controls blinking. it's very simplistic, and can
    //be moved somewhere else later.
    static float blink = 0.0;
    //reset blink if it goes over 1.0
    blink = std::fmod(blink, 1);
    //add some time and blink afterwards
    float blink_speed = 0.2;
    float blink_randomness = 2.0;
    blink += std::pow((blink_speed * ((float)std::rand() / RAND_MAX)), blink_randomness);

    dp_torso.update(a_rms.get_level());
    dp_torso_bb.update();
    dp_mouth_closed.update(a_rms.get_level());
    dp_mouth_open.update(a_rms.get_level());
    dp_eyes_closed.update(blink);
    dp_eyes_open.update(blink);
    dp_outfit_polo.update();
    dp_outfit_roseglasses.update();
    dp_penhand.update(sw.check_titles(krita_titles));
    dp_bookhand.update(sw.check_titles(krita_titles));
    dp_phonehand.update(sw.check_titles(phone_titles));
    dp_xboxhand.update(sw.check_titles(videogame_titles));

    //little experiment here to see if i can get this working
    phone_on = phone_on || event_v1::get().check_flag(event_v1::PHONE_UP);
    phone_on = phone_on && sw.check_titles(phone_titles);
    dp_phone_base.update((phone_on ? 1.0 : 0.0));
    dp_phone_bubble_base.update(sw.check_titles(phone_titles));
    dp_phone_bubble_discord.update(sw.check_titles(discord_titles));
    dp_phone_bubble_firefox.update(sw.check_titles(firefox_titles));

    event_v1::get().update();

    dp_torso.draw();
    //dp_torso_bb.draw();
    dp_eyes_closed.draw();
    dp_eyes_open.draw();
    dp_outfit_polo.draw();
    dp_mouth_closed.draw();
    dp_mouth_open.draw();
    dp_outfit_roseglasses.draw();
    dp_penhand.draw();
    dp_bookhand.draw();
    dp_phonehand.draw();
    dp_phone_bubble_base.draw();
    dp_phone_bubble_firefox.draw();
    dp_phone_bubble_discord.draw();
    dp_phone_base.draw();
    dp_xboxhand.draw();

    //show the focused window (it works!)
    if(false) {
      std::string sw_out = "";
      sw.get_screen_title(&sw_out);
      if(sw.check_titles(krita_titles) && false) {
        std::cout << "krita!" << std::endl;
      }
      std::cout << std::setw(60) << std::left << sw_out << "            \r" << std::right << std::flush;
      static int count = 0;
      std::cout << count++/24 << " " << std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1000) << " " << T_DELAY << std::endl;
     }

    // -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -



    r.show();

    //stick something in the term so we have a visual indicator that we're
    //still ticking
    print_debug_swirly();

    //go to sleep for a bit
    SDL_Delay(tick_wait());
    //now reset the timer
    tick_set();
  }

}

engine::engine() : is_init(true), last_tick(0) { 
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
  double curr_tick = 
    std::chrono::system_clock::now().time_since_epoch() /
    std::chrono::milliseconds(1);

  double elapsed = curr_tick - last_tick;
  //std::cout << T_DELAY << " " << elapsed << " " << T_DELAY - elapsed << " " << std::max(0.0, std::round(T_DELAY - elapsed)) << std::endl;

  return std::max(0.0, std::round(T_DELAY - elapsed));
}

void engine::tick_set() {
  last_tick = 
    std::chrono::system_clock::now().time_since_epoch() /
    std::chrono::milliseconds(1);
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
