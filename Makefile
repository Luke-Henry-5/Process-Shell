wish: main.c functions.c
	gcc -o wish main.c functions.c

clean: 
	rm wish

redo: main.c functions.c
	rm wish 
	gcc -o wish main.c functions.c

test: main.c functions.c
	rm wish 
	gcc -o wish main.c functions.c
	./test-wish.sh

testall: main.c functions.c
	rm wish 
	gcc -o wish main.c functions.c
	./test-wish.sh -c

bat: main.c functions.c
	rm wish 
	gcc -o wish main.c functions.c
	./wish batch.txt