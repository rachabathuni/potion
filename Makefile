    
lib:
	g++ -c -fPIC -std=c++20 -I/opt/local/include src/utils.cpp src/threading.cpp src/tcpserver_unix.cpp src/request.cpp src/response.cpp src/potion.cpp
	g++ -L/opt/local/lib utils.o threading.o tcpserver_unix.o response.o request.o potion.o -shared -o bin/libpotion_arm64.so
all: lib

