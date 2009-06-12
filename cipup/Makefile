CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

OBJS = obj/cipup.o

SRCS = src/cipup.cpp

LIBS =

TARGET = bin/cipup.exe

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

$(OBJS) : $(SRCS)
	$(CXX) -c $< -o $@

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
