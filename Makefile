build:
	gcc headers/*.h src/*.c -o drawing-tui -lncurses

build-test:
	gcc -I. headers/*.h src/*.c -o tuiTest -lncurses

test: build-test
	./tuiTest

test-run:
	./tuiTest

