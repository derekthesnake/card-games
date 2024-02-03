#g++ hello-world.cpp uWebSockets/uSockets/*.o -IuWebSockets/uSockets/src -IuWebSockets/src -std=c++20 -flto -lz

CC = g++
C_INCS = -IuWebSockets/src -IuWebSockets/uSockets/src
C_OBJS = uWebSockets/uSockets/*.o
CFLAGS = -std=c++20 -flto -lz

default:
	$(CC) hello-world.cpp -o hello_world $(C_INCS) $(C_OBJS) $(CFLAGS)

dependencies:
	make -C uWebSockets
