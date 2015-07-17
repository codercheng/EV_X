OBJ := demo.o ev_loop.o ev_io.o ev_timer.o
CC := gcc
DEBUG := -g -Wall

demo: $(OBJ)
	$(CC) $(DEBUG) $(OBJ) -o $@ -lrt

demo.o: demo.c ev_loop.h ev_type.h
	$(CC) $(DEBUG) -c $<

ev_loop.o: ev_loop.c ev_loop.h ev_type.h
	$(CC) $(DEBUG) -c $<

ev_io.o: ev_io.c ev_loop.h ev_type.h
	$(CC) $(DEBUG) -c $<
ev_timer.o: ev_timer.c ev_loop.h ev_type.h
	$(CC) $(DEBUG) -c $<

clean:
	rm *.o demo -f
