OBJ = demo.o ev_loop.o ev_io.o ev_timer.o
CC = gcc
DEBUG = -g -Wall

demo: $(OBJ)
	$(CC) $(DEBUG) $(OBJ) -o demo -lrt

demo.o: demo.c ev_loop.h ev_type.h
	$(CC) $(DEBUG) -c demo.c

ev_loop.o: ev_loop.c ev_loop.h ev_type.h
	$(CC) $(DEBUG) -c ev_loop.c

ev_io.o: ev_io.c ev_loop.h ev_type.h
	$(CC) $(DEBUG) -c ev_io.c
ev_timer.o: ev_timer.c ev_loop.h ev_type.h
	$(CC) $(DEBUG) -c ev_timer.c

clean:
	rm *.o demo