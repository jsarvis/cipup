CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

OBJS = obj/rabbit.o obj/cipup.o obj/test.o

#SRCS = prng/rabbit.c src/cipup.cpp

LIBS = lib/cipup.dll

DESTLIBS = bin/cipup.dll 

DLLOBJS = obj/rabbit.o obj/cipup.o

DLLTARGET = lib/cipup.dll

SYSLIBS =

TESTOBJS = obj/test.o

TESTTARGET = bin/test.exe

$(DLLTARGET): $(DLLOBJS)
	$(CXX) $(DLLOBJS) -shared -o $(DLLTARGET)
	
$(TESTTARGET):	$(TESTOBJS) $(LIBS)
	$(CXX) -o $(TESTTARGET) $(TESTOBJS) $(LIBS) $(SYSLIBS)
	cp $(LIBS) $(DESTLIBS)

#$(OBJS) : $(SRCS)
#	$(CXX) -c $< -o $@

obj/rabbit.o: prng/rabbit.c prng/ecrypt-sync.h prng/ecrypt-portable.h prng/ecrypt-machine.h prng/ecrypt-config.h
	$(CXX) -c $< -o $@
	
obj/cipup.o: src/cipup.cpp src/cipup.hpp prng/ecrypt-sync.h prng/ecrypt-portable.h prng/ecrypt-machine.h prng/ecrypt-config.h
	$(CXX) -c $< -o $@

obj/test.o: src/test.cpp src/cipup.hpp
	$(CXX) -c $< -o $@


all: dll test

dll: $(DLLTARGET)

test: $(DLLTARGET) $(TESTTARGET)

clean:
	rm -f $(OBJS) $(LIBS) $(TESTTARGET) $(DESTLIBS)
