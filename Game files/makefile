all: zombie_jump.exe

zombie_jump.exe: main.c
	gcc *.c -I../ZDK -L../ZDK -std=c99 -lzdk -lm -lncurses -o zombie_jump
	
clean:
	rm main.c zombie_jump.exe