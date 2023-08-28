all:
	gcc -Iinclude -Isrc/headers -Llib -o bin/main src/*.c -lmingw32 -lSDL2main -lSDL2 -lSDL2_image