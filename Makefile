#g++ hello-world.cpp uWebSockets/uSockets/*.o -IuWebSockets/uSockets/src -IuWebSockets/src -std=c++20 -flto -lz

CC = g++
C_INCS = -Isrc -IuWebSockets/src -IuWebSockets/uSockets/src
C_OBJS = uWebSockets/uSockets/*.o
CFLAGS = -std=c++20 -flto -lz

default: src/server.cpp src/protos/message.pb.cc src/protos/message.pb.h src/game.h
#	$(CC) hello-world.cpp -o hello_world $(C_INCS) $(C_OBJS) $(CFLAGS)
	$(CC) src/server.cpp -o server src/protos/message.pb.cc $(C_OBJS) $(C_INCS) $(CFLAGS) `pkg-config --cflags --libs protobuf`

dependencies:
	make -C uWebSockets

#objs/%.pb.o: src/protos/%.pb.cc src/protos/%.pb.h
#	$(CC) -c $< -o $@ -Isrc `pkg-config --cflags --libs protobuf`

src/protos/%.pb.h src/protos/%.pb.cc: protos/%.proto
	protoc $< --cpp_out=src

clean:
	rm -r src/protos
	rm src/*.o

message-test: src/message-test.cpp src/protos/message.pb.cc src/protos/message.pb.h
	g++ -o make-test src/message-test.cpp src/protos/message.pb.cc -Isrc `pkg-config --cflags --libs protobuf`

#src/game.o: protobufs
#	g++ src/game.h -o src/game.o -Isrc
