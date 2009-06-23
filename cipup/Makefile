CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

OBJS = obj/rabbit.o obj/cipup.o

#SRCS = prng/rabbit.c src/cipup.cpp

LIBS =

TARGET = bin/cipup.exe

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

#$(OBJS) : $(SRCS)
#	$(CXX) -c $< -o $@

obj/rabbit.o: prng/rabbit.c prng/ecrypt-sync.h prng/ecrypt-portable.h prng/ecrypt-machine.h prng/ecrypt-config.h
	$(CXX) -c $< -o $@
	
obj/cipup.o: src/cipup.cpp prng/ecrypt-sync.h prng/ecrypt-portable.h prng/ecrypt-machine.h prng/ecrypt-config.h
	$(CXX) -c $< -o $@

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
