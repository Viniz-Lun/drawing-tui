build:
	gcc drawing-tui.c tuiWrapper.c tuiWrapper.h list.c list.h custom-utils.h custom-utils.c RGB.h -o drawing-tui -lncurses

build-test:
	gcc drawing-tui.c tuiWrapper.c tuiWrapper.h list.c list.h custom-utils.h custom-utils.c RGB.h -o tuiTest -lncurses

test: build-test
	./tuiTest

test-run:
	./tuiTest

