build: bin/ lexer.o parser.o
	@gcc bin/lexer.o bin/parser.o -o c3

clean:
	@rm -rf bin/
	@rm c3

lexer.o:
	@gcc -c src/lexer.c -o bin/lexer.o

parser.o:
	@gcc -c src/parser.c -o bin/parser.o

bin/:
	@mkdir bin
