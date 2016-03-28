#needs chaiscript, https://github.com/ChaiScript/ChaiScript/releases

default: all

all: chailo

chailo: chailo.cc
	@echo "compiling chailo.cc. this may take a while!"
	g++ -std=c++0x -o chailo chailo.cc `pkg-config --libs --cflags chaiscript liblo` -ldl
	@echo example run: ./chailo scripts/send.chai

clean:
	rm -f chailo
