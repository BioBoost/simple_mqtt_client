# The compiler to use
# CCPREFIX=/home/bioboost/rpi-tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf-
CC=$(CCPREFIX)g++

# Compiler flags
CFLAGS=-c -Wall -std=c++11
    # -c: Compile or assemble the source files, but do not link. The linking stage simply is not done. The ultimate output is in the form of an object file for each source file.
    # -Wall: This enables all the warnings about constructions that some users consider questionable, and that are easy to avoid (or modify to prevent the warning), even in conjunction with macros.

# LDFLAGS=

# Libraries
LIBS=-lpaho-mqttpp3 -lpaho-mqtt3a -lpthread

# Name of executable output
TARGET=mqtthello
SRCDIR=src
BUILDDIR=bin

OBJS := $(patsubst %.cpp,%.o,$(shell find $(SRCDIR) -name '*.cpp'))

all: makebuildir $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(LDFLAGS) -o $(BUILDDIR)/$@ $(OBJS) $(LIBS)

%.o : %.cpp
	$(CC) $(CFLAGS) $< -o $@

clean :
	rm -rf $(BUILDDIR)
	rm -f $(OBJS)

makebuildir:
	mkdir -p $(BUILDDIR)
