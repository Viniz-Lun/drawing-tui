build:
	gcc headers/* drawing-tui.c tuiWrapper.c list.c custom-utils.c -o drawing-tui -lncurses

build-test:
	gcc headers/*.h *.c -o tuiTest -lncurses

test: build-test
	./tuiTest

test-run:
	./tuiTest

