OBJ = demo.o ev_loop.o ev_io.o

demo: $(OBJ)
	gcc -g -Wall $(OBJ) -o demo

demo.o: demo.c ev_loop.h ev_type.h
	gcc -g -Wall -c demo.c

ev_loop.o: ev_loop.c ev_loop.h ev_type.h
	gcc -g -Wall -c ev_loop.c

ev_io.o: ev_io.c ev_loop.h ev_type.h
	gcc -g -Wall -c ev_io.c
