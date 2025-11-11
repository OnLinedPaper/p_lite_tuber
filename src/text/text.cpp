#include "./src/text/text.h"
#include <filesystem>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

text::text(const std::string fi_name, const std::string fo_name) :
    fifo_path_base("./resources/fifo/")
  , fifo_path(fifo_path_base + fi_name)
  , font_path_base("./resources/control/font/")
  , font_path(font_path_base + fo_name)
  , message("")
  , pipe(-1)
  , pipe_open(false)
{ 
  //check if fifo exists; link if it does, create otherwise
  if(!std::filesystem::is_fifo(fifo_path)) {
    if(mkfifo(fifo_path.c_str(), 0600) == -1) {
      //TODO: throw error here
      return;
    }
  }

  //open fifo in non-blocking mode
  pipe = open(fifo_path.c_str(), O_RDONLY | O_NONBLOCK, 0600);
  if(pipe == -1) { return; }

  //if we got this far, all should be good to go
  pipe_open = true;
}

text::~text() {
  //close fifo
  close(pipe);
}

void text::update() {
  //don't read from empty pipe
  if(!pipe_open) { return; }

  //read from the buffer
  int bytes_read = 1;
  bool reading = false;

  //repeat until entire buffer is empty (for very long strings)
  while(bytes_read > 0) {
    bytes_read = read(pipe, buf, buf_size);
    
    if(bytes_read > 0) {
      //data was in the pipe! wipe it if this is our first read; else, append
      if(!reading) { message.clear(); reading = true; }
      //null-terminate the string
      buf[bytes_read] = '\0';
      message += buf;
    }
    //"clear" the buffer
    buf[0] = '\0';
  }
}

void text::draw() const {

}
