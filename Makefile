build:
	gcc drawing-tui.c tuiWrapper.c tuiWrapper.h list.c list.h -o drawing-tui -lncurses

build-test:
	gcc drawing-tui.c tuiWrapper.c tuiWrapper.h list.c list.h -o tuiTest -lncurses

test: build-test
	./tuiTest

test-run:
	./tuiTest

