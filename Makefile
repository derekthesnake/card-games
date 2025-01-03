#g++ hello-world.cpp uWebSockets/uSockets/*.o -IuWebSockets/uSockets/src -IuWebSockets/src -std=c++20 -flto -lz

CC = g++
C_INCS = -Ibackend/src -Ibackend/uWebSockets/src -Ibackend/uWebSockets/uSockets/src
WS_OBJS = backend/uWebSockets/uSockets/*.o
CFLAGS = -std=c++20 -flto -lz
PBFLAGS = `pkg-config --cflags --libs protobuf`

default: backend gui

backend: backend/server

backend/server: backend/src/500.cpp backend/src/server.hpp backend/src/protos/message.pb.cc backend/src/protos/message.pb.h backend/src/game.h backend/src/log.h backend/src/player-data.h
#	$(CC) hello-world.cpp -o hello_world $(C_INCS) $(WS_OBJS) $(CFLAGS)
	$(CC) backend/src/500.cpp -o backend/server backend/src/protos/message.pb.cc $(WS_OBJS) $(C_INCS) $(CFLAGS) $(PBFLAGS)

gui: gui/protos/play.pb.go gui/app.go

dependencies:
	make -C uWebSockets

#objs/%.pb.o: src/protos/%.pb.cc src/protos/%.pb.h
#	$(CC) -c $< -o $@ -Isrc `pkg-config --cflags --libs protobuf`

backend/src/protos/%.pb.h backend/src/protos/%.pb.cc gui/protos/%.pb.go &: protos/%.proto
	protoc $< --cpp_out=backend/src --go_out=gui --go_opt=paths=source_relative --go_opt=default_api_level=API_OPAQUE

clean:
	rm -r backend/src/protos
	rm backend/src/*.o
	rm -r gui/protos

message-test: backend/src/message-test.cpp backend/src/protos/message.pb.cc backend/src/protos/message.pb.h
	g++ -o make-test backend/src/message-test.cpp backend/src/protos/message.pb.cc -Ibackend/src $(PBFLAGS)

.PHONY: message-test clean dependencies default backend gui

#src/game.o: protobufs
#	g++ src/game.h -o src/game.o -Isrc
