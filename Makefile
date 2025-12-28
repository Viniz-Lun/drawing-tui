build:
	gcc headers/*.h src/*.c -o drawing-tui -lncurses

build-test:
	gcc -I. headers/*.h src/*.c -o tuiTest -lncurses

test-run:
	./tuiTest

test: build-test test-run

translator:
	gcc -I. headers/context.h headers/myColor.h headers/curse-files.h headers/collection.h headers/custom-utils.h src/myColor.c src/context.c src/curse-files.c src/collection.c src/custom-utils.c translate-tool/translator.c -o translator -lncurses
