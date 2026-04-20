main:
	clang++ -Wno-deprecated-declarations ./src/firstParty/*.cpp ./src/thirdParty/box2d/collision/*.cpp \
		./src/thirdParty/box2d/common/*.cpp ./src/thirdParty/box2d/dynamics/*.cpp \
		./src/thirdParty/box2d/rope/*.cpp -std=c++17 -O3 -I./src/firstParty \
	    -I./src/thirdParty -I./src/thirdParty/SDL2/ -I./src/thirdParty/SDL2_mixer/ -I./src/thirdParty/SDL2_ttf/ \
		-I./src/thirdParty/lua -I./src/thirdParty/SDL2_image/ -I./src/thirdParty/LuaBridge \
		-I./src/thirdParty/box2d -lSDL2 -lSDL2_mixer \
		-lSDL2_ttf -lSDL2_image -llua5.4 -o game_engine_linux

debug:
	clang++ -Wno-deprecated-declarations ./src/firstParty/*.cpp -g -std=c++17 -O3 -I./src/firstParty -I./src/thirdParty \
	    -I./src/thirdParty/SDL2/ -I./src/thirdParty/SDL2_mixer/ -I./src/thirdParty/SDL2_ttf/ \
		-I./src/thirdParty/SDL2_image/ -lSDL2 -lSDL2_mixer \
		-lsrc/thirdParty/SDL2_ttf -lsrc/thirdParty/SDL2_image -o game_engine_linux_debug

clean:
	rm -rf game_engine_linux game_engine_linux_debug
