rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

SRC := $(call rwildcard, ./src/core/, *.cpp)
SRC += $(call rwildcard, ./src/physics/, *.cpp)
SRC += $(call rwildcard, ./src/poisson/, *.cpp)
SRC += $(call rwildcard, ./src/segmentation/, *.cpp)
# SRC += $(call rwildcard, ./src/tiger/, *.cpp)
CXX = g++
CC = gcc

EXOBJS :=$(patsubst %.cpp, %.o, $(call rwildcard, ./src/example/, *.cpp))
CXXFLAGS:= -DGL_GLEXT_PROTOTYPES=1 -std=gnu++14 -O3 -w -fPIC -MMD -MP -fopenmp -c -g -fmessage-length=0 -I./include/ -I./include/core/ 
CFLAGS:= -DGL_GLEXT_PROTOTYPES=1 -std=c11 -O3 -w -fPIC -MMD -MP -fopenmp -c -g -fmessage-length=0 -I./include/ -I./include/core/
LDLIBS =-L./ -L/usr/lib/ -L/usr/local/lib/ -L/usr/local/cuda-8.0/lib64/ -L/usr/lib/x86_64-linux-gnu/ -L./ext/glfw/src/
LIBS = -lglfw3 -lstdc++ -lgcc -lgomp -lGL -lXext -lGLU -lGLEW -lXi -lXrandr -lX11 -lXxf86vm -lXinerama -lXcursor -lXdamage -lpthread -lm -ldl

HAS_OPENCL=0
ifneq ($(wildcard /usr/lib/libOpenCL.so /usr/local/cuda-8.0/lib64/libOpenCL.so /usr/lib/x86_64-linux-gnu/libOpenCL.so), "")
	LIBS+=-lOpenCL
	SRC += $(call rwildcard, ./src/ocl/, *.cpp)
	HAS_OPENCL=1
endif

LIBOBJS := $(patsubst %.cpp, %.o, $(SRC))
LIBOBJS += $(patsubst %.c, %.o, $(call rwildcard, ./src/, *.c))
SRC += $(call rwildcard, ./src/, *.c)

default: all

release: $(LIBOBJS)
	mkdir -p ./Release
	rm -f ./Release/libAlloy.so
	$(CXX) -shared -o "Release/libAlloy.so" $(LIBOBJS) $(LDLIBS) $(LIBS) -Wl,-rpath="/usr/local/lib/:/usr/lib/x86_64-linux-gnu/"

examples: $(LIBOBJS) $(EXOBJS)
	$(CXX) -o ./Release/examples $(EXOBJS) $(LIBOBJS) $(LDLIBS) $(LIBS) -Wl,-rpath="/usr/local/lib/:/usr/lib/x86_64-linux-gnu/"

all: examples

clean:
	rm -f $(LIBOBJS) $(EXOBJS)
	rm -f ./Release/libAlloy.so
	rm -f ./Release/examples

.PHONY : all

