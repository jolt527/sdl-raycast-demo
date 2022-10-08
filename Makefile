all:
	g++ main.cpp -o game -I/Library/Frameworks/SDL2.framework/Headers -F/Library/Frameworks -framework SDL2

clean:
	rm -rf game
