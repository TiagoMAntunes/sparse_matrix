all:
	gcc -Wall -ansi main.c -o proj1
	cp main.c testes-publicos/main.c
	zip proj.zip *.c