rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

SRC := $(call rwildcard, ./src/core/, *.cpp)
SRC += $(call rwildcard, ./src/physics/, *.cpp)
SRC += $(call rwildcard, ./src/poisson/, *.cpp)
CXX = g++
CC = gcc

EXOBJS :=$(patsubst %.cpp, %.o, $(call rwildcard, ./src/example/, *.cpp))
CXXFLAGS:= -DGL_GLEXT_PROTOTYPES=1 -std=c++0x -O3 -w -fPIC -MMD -MP -fopenmp -c -g -fmessage-length=0 -I./include/ -I./include/core/ 
CFLAGS:= -DGL_GLEXT_PROTOTYPES=1 -std=c11 -O3 -w -fPIC -MMD -MP -fopenmp -c -g -fmessage-length=0 -I./include/ -I./include/core/
LDLIBS =-L./ -L/usr/lib/ -L/usr/local/lib/ -L/usr/lib/x86_64-linux-gnu/ -L./ext/glfw/src/
LIBS = -lglfw3 -lstdc++ -lgcc -lgomp -lGL -lXext -lGLU -lGLEW -lXi -lXrandr -lX11 -lXxf86vm -lXinerama -lXcursor -lXdamage -lpthread -lm -ldl
RM=rm -f
HAS_OPENCL=0
ifneq ($(wildcard /usr/lib/libOpenCL.so /usr/local/lib/libOpenCL.so /usr/lib/x86_64-linux-gnu/libOpenCL.so), "")
	LIBS+=-lOpenCL
	SRC += $(call rwildcard, ./src/ocl/, *.cpp)
	HAS_OPENCL=1
endif

LIBOBJS := $(patsubst %.cpp, %.o, $(SRC))
LIBOBJS += $(patsubst %.c, %.o, $(call rwildcard, ./src/, *.c))
default: all

library: $(LIBOBJS)
	$(CXX) -shared -o libAlloy.so $(LIBOBJS) $(LDLIBS) $(LIBS)

examples: $(EXOBJS)
	$(CXX) -o examples $(EXOBJS) $(LDLIBS) -lAlloy $(LIBS) -Wl,-rpath="./"

all: library examples

clean:
	rm -f $(LIBOBJS) $(EXOBJS)
	rm -f libAlloy.so
	rm -f examples

.PHONY : all

