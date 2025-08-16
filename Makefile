CC=g++
DIR := ${CURDIR}
CFLAGS= -Wall -Wextra -Wpedantic --std=c++17 -I$(DIR)
OFLAGS= -O3
DFLAGS= -g -ggdb -O0
LFLAGS= -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf
DDIR= debugging
XDIR= bin
BDIR= build
SDIR= src
DBDIR= build/dbuild

DEPS:= main.h engine.h render.h image.h audio.h doll.h action.h

OBJS:= action.o audio.o engine.o doll.o image.o main.o render.o 
DOBJS:= $(addprefix $(DBDIR)/,$(OBJS))
OBJS:= $(addprefix $(BDIR)/,$(OBJS))

SRCS:= main.cpp engine.cpp render.cpp image.cpp audio.cpp doll.cpp action.cpp

PATHS:= . renders images audio doll actions
VPATH:= $(addprefix src/,$(PATHS))

# copied more or less verbatim from qdbp's makefile
# for all .o files: use the .cpp files and the deps
# and compile them with
#   -c      for an object file
#   -o $@   for giving it a name of the object file before the :
#   $<      for the first prerequisite to make the file (i think a .h)
$(BDIR)/%.o: %.cpp %.h
	@mkdir -p $(BDIR)
	@printf "building %s\n" $@
	@$(CC) $(CFLAGS) $(OFLAGS) -c -o $@ $<

$(DBDIR)/%.o: %.cpp %.h
	@mkdir -p $(BDIR)
	@mkdir -p $(DBDIR)
	@printf "building debug obj %s\n" $@
	@$(CC) $(CFLAGS) $(DFLAGS) -c -o $@ $<

run: $(OBJS) 
	@mkdir -p $(XDIR)
	@printf "final compilation..."
	@$(CC) $(CFLAGS) $(OFLAGS) -o $(XDIR)/$@ $^ $(LFLAGS)
	@printf "compiled\ndone\n"

debug: $(DOBJS) 
	@mkdir -p $(BDIR)
	@mkdir -p $(DBDIR)
	@mkdir -p $(DDIR)
	@printf "final compilation... "
	@$(CC) $(CFLAGS) $(DFLAGS) -o $(DDIR)/$@ $^ $(LFLAGS);
	@printf "compiled\ndone\n"
	

clean:
	rm -f *.o run $(DDIR)/debug $(XDIR)/run $(BDIR)/*.o $(DBDIR)/*.o

#TODO: clean out the log pollution from nvidia's garbage drivers
mem:
	@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=$(DDIR)/.v.out $(DDIR)/debug && \
	cat $(DDIR)/.v.out | awk '/HEAP SUMMARY/{p=1}p' > $(DDIR)/.v2.out && \
	sed 's/==.*== //' $(DDIR)/".v2.out" > $(DDIR)/"full-valgrind-out.txt" && \
	cat $(DDIR)/full-valgrind-out.txt > $(DDIR)/valgrind-out.txt && \
	perl -i -ne 'BEGIN{$$/=""} print unless (/SDL_.*Init/ or /X11_ShowCursor/ or  /dlopen\@\@GLIBC_2.2.5/ or /XSetLocaleModifiers/ or /_dl_catch_exception/ or /_XlcCurrentLC/ or /libpulsecommon/ or /SDL_CreateWindow_REAL/ or /lib\/x86_64-linux-gnu\/dri\/i965_dri.so/ or /TTF_Init/ or /_dl_init/ or /_dbus_strdup/ or /vgpreload/ or /wrefresh/ or /waddch/)' $(DDIR)/valgrind-out.txt;
	@rm $(DDIR)/.v.out $(DDIR)/.v2.out; less $(DDIR)/valgrind-out.txt;


