#include <iostream>
#include "src/engine.h"

int main(int argc, char *argv[]) {

  //squash compiler warnings
  if (argc && argv) {}

  engine e;
  e.play();
  return (0);

}
