rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

SRC := $(call rwildcard, ./src/common/, *.cpp)
SRC += $(call rwildcard, ./src/physics/, *.cpp)
SRC += $(call rwildcard, ./src/graphics/, *.cpp)
SRC += $(call rwildcard, ./src/image/, *.cpp)
SRC += $(call rwildcard, ./src/math/, *.cpp)
SRC += $(call rwildcard, ./src/ui/, *.cpp)
SRC += $(call rwildcard, ./src/system/, *.cpp)
SRC += $(call rwildcard, ./src/vision/, *.cpp)
CXX = g++
CC = gcc
EXOBJS := ./src/main.o
EXOBJS +=$(patsubst %.cpp, %.o, $(call rwildcard, ./src/example/, *.cpp))

CXXFLAGS:= -DGL_GLEXT_PROTOTYPES=1 -std=c++11 -O3 -w -fPIC -MMD -MP -fopenmp -c -g -fmessage-length=0 -I./src/ -I./src/common/  -I./src/common/zstd/common/ -I./src/common/lz4/
CFLAGS:= -DGL_GLEXT_PROTOTYPES=1 -std=c11 -O3 -w -fPIC -MMD -MP -fopenmp -c -g -fmessage-length=0 -I./src/ -I./src/common/ -I./src/common/zstd/common/ -I./src/common/lz4/
LDLIBS =-L./ -L/usr/lib64 -L/usr/lib/ -L/usr/local/lib/ -L/usr/local/cuda-9.1/lib64/ -L/usr/lib/x86_64-linux-gnu/ -L./ext/glfw/src/
LIBS = -lglfw -lstdc++ -lgcc -lgomp -lGL -lXext -lGLU -lGLEW -lXi -lXrandr -lX11 -lXxf86vm -lXinerama -lXcursor -lXdamage -lpthread -lm -ldl -ltiff
CXXFLAGS+= -DZSTD_MULTITHREAD -DXXH_NAMESPACE=ZSTD_ -DZSTD_STATIC_LINKING_ONLY -DXXH_STATIC_LINKING_ONLY
CFLAGS+= -Wall -Wextra -Wcast-qual -Wcast-align -Wshadow \
            -Wstrict-aliasing=1 -Wswitch-enum -Wdeclaration-after-statement \
            -Wstrict-prototypes -Wundef -Wpointer-arith -Wformat-security \
            -Wvla -Wformat=2 -Winit-self -Wfloat-equal -Wwrite-strings \
            -Wredundant-decls -Wmissing-prototypes -Wc++-compat

HAS_OPENCL=0
ifneq ($(wildcard /usr/lib/libOpenCL.so /usr/local/cuda-9.1/lib64/libOpenCL.so /usr/lib/x86_64-linux-gnu/libOpenCL.so), "")
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
	$(CXX) -shared -o "Release/libAlloy.so" $(LIBOBJS) $(LDLIBS) $(LIBS) -Wl,-rpath="/usr/lib64/:/usr/local/lib/:/usr/lib/x86_64-linux-gnu/"

examples: $(LIBOBJS) $(EXOBJS)
	mkdir -p ./Release
	$(CXX) -o ./Release/examples $(EXOBJS) $(LIBOBJS) $(LDLIBS) $(LIBS) -Wl,-rpath="/usr/lib64/:/usr/local/lib/:/usr/lib/x86_64-linux-gnu/"

all: examples

clean:
	rm -f $(LIBOBJS) $(EXOBJS)
	rm -f ./Release/libAlloy.so
	rm -f ./Release/examples

.PHONY : all

