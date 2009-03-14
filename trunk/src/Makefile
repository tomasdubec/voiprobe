LIBS= -lpcap -lsqlite3 -lpthread `net-snmp-config --libs` `net-snmp-config --agent-libs`
CFLAGS= -c -g
OBJECTS=voiprobe.o packet_catcher.o packet.o db.o reply_server.o latency_computer.o snmp.o

all: voiprobe

voiprobe.o: voiprobe.cpp
	g++ $(CFLAGS) -o voiprobe.o voiprobe.cpp

packet_catcher.o: packet_catcher.cpp packet_catcher.h
	g++ $(CFLAGS) -o packet_catcher.o packet_catcher.cpp

packet.o: packet.cpp packet.h
	g++ $(CFLAGS) -o packet.o packet.cpp

db.o: db.cpp db.h
	g++ $(CFLAGS) -o db.o db.cpp

reply_server.o: reply_server.cpp reply_server.h
	g++ $(CFLAGS) -o reply_server.o reply_server.cpp

latency_computer.o: latency_computer.cpp latency_computer.h
	g++ $(CFLAGS) -o latency_computer.o latency_computer.cpp

snmp.o: snmp.cpp snmp.h
	g++ $(CFLAGS) `net-snmp-config --cflags` -o snmp.o snmp.cpp

voiprobe: $(OBJECTS)
	g++ $(LIBS) -o voiprobe $(OBJECTS)

clean:
	rm -f voiprobe *.o
