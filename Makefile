rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))
SRC := $(call rwildcard, ./src/, *.cpp)
CXX = g++
CC = gcc
EXOBJS := $(patsubst %.cpp, %.o, $(call rwildcard, ./src/,*.cpp))
CXXFLAGS:= -std=c++0x -O3 -w -fPIC -MMD -MP -fopenmp -c -fmessage-length=0 -I./include/ -I./include/core/
LDLIBS =-L./ -L/usr/lib/ -L/usr/local/lib/ -L/usr/lib/x86_64-linux-gnu/ -L./ext/glfw/src/
LIBS =-lglfw3 -lstdc++ -lgcc -lgomp -lGL -lXext -lGLU -lGLEW -lXi -lXrandr -lX11 -lXxf86vm -lXinerama -lXcursor -lXdamage -lpthread -lm -ldl
RM=rm -f
all: $(EXOBJS)
	$(CXX) -o liballoy.so -shared $(EXOBJS) $(LDLIBS) $(LIBS)

clean:
	@rm $(EXOBJS)

default: all

