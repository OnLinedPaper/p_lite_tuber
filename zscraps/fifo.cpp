#include <filesystem> //is_fifo
#include <iostream>
//#include <fstream>
#include <unistd.h> //read, close
#include <sys/stat.h> //mkfifo
#include <fcntl.h> //open, O_RDONLY, O_NONBLOCK

//test program: nonblocking read from a FIFO created in a place of my choosing. note that for the sake of this program, i'm going to decide that the strings must end with a null terminator.

int main(void) {

  const char* PIPE_ADDR="./p_lite_pipe_0"; //named pipe (TODO: stick it in resources)
  int pipe_in;  //file descriptor for named pipe
  int buf_size = 255; //buffer size
  char pipe_data[buf_size + 1] = { 0 }; //buffer to grab data from pipe (init full of null terms)

  //make the pipe
  if(!std::filesystem::is_fifo(PIPE_ADDR)) {  //check to see if it already exists
    if(mkfifo(PIPE_ADDR, 0600) == -1) { //make it if it doesn't
      std::cerr << "couldn't make fifo pipe \"" << PIPE_ADDR << "\"" << std::endl;
      return -1;
    }
  }
  
  pipe_in = open(PIPE_ADDR, O_RDONLY | O_NONBLOCK, 0600); //open it in readonly, nonblocking mode
  for(int i=0; i<40000; i++) {
    int rd = read(pipe_in, pipe_data, buf_size);  //check to see if there's any data in there, and record how much if there is
    if(rd != 0) {fprintf(stdout, "%d \"%s\"\n", i, pipe_data); }
    pipe_data[0] = '\0'; //null-terminator to "clear" the buffer
    usleep(10000);
  }  
  close(pipe_in); //remember to close the pipe



  return 0;
}
