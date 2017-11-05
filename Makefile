CC=$(CCPREFIX)g++
CFLAGS=-c -Wall -std=c++11
LIBS=-lpaho-mqttpp3 -lpaho-mqtt3a -lpthread

TARGET=mqtthello

all: $(TARGET)

$(TARGET) : main.cpp
	$(CC) main.cpp -o $(TARGET) $(LIBS)

clean :
	rm -rf $(TARGET)
