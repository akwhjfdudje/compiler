CFLAGS = -ggdb
ifeq ($(DEBUG),1)
	CFLAGS += -DDEBUG
endif

build: bin/ lexer.o parser.o main.o generator.o
	@gcc bin/*.o -o c3
	
debug: CFLAGS += -DDEBUG
debug: build

clean:
	@rm -rf bin/
	@rm c3

lexer.o: src/lexer.c
	@gcc $(CFLAGS) -c src/lexer.c -o bin/lexer.o

parser.o: src/parser.c
	@gcc $(CFLAGS) -c src/parser.c -o bin/parser.o

generator.o: src/generator.c
	@gcc $(CFLAGS) -c src/generator.c -o bin/generator.o

main.o: src/main.c
	@gcc $(CFLAGS) -c src/main.c -o bin/main.o

bin/:
	@mkdir bin

