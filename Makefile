build: bin/ lexer.o parser.o main.o
	@gcc bin/*.o -o c3

clean:
	@rm -rf bin/
	@rm c3

lexer.o: src/lexer.c
	@gcc -ggdb -c src/lexer.c -o bin/lexer.o

parser.o: src/parser.c
	@gcc -ggdb -c src/parser.c -o bin/parser.o

generator.o: src/generator.c
	@gcc -ggdb -c src/generator.c -o bin/generator.o

main.o: src/main.c
	@gcc -ggdb -c src/main.c -o bin/main.o

bin/:
	@mkdir bin
