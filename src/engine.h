#ifndef ENGINE_H_
#define ENGINE_H_

#include "src/renders/render.h"

class engine {

public:
  engine();
  engine(const engine&) = delete;
  engine &operator=(const engine &) = delete;
  ~engine();

  void play();

private:
  bool init();
  bool is_init;
  void print_debug_swirly() const;

  render r;
};

#endif
