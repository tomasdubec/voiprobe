LIBS= -lpcap -lsqlite3 -lpthread
CFLAGS= -c -g
OBJECTS=voiprobe.o packet_catcher.o packet.o db.o

all: voiprobe

voiprobe.o: voiprobe.cpp
	g++ $(CFLAGS) -o voiprobe.o voiprobe.cpp

packet_catcher.o: packet_catcher.cpp packet_catcher.h rtp.h
	g++ $(CFLAGS) -o packet_catcher.o packet_catcher.cpp

packet.o: packet.cpp packet.h
	g++ $(CFLAGS) -o packet.o packet.cpp

db.o: db.cpp db.h
	g++ $(CFLAGS) -o db.o db.cpp

voiprobe: $(OBJECTS)
	g++ $(LIBS) -o voiprobe $(OBJECTS)

clean:
	rm -f voiprobe *.o