ALL: test

rbtree.o: rbtree.c
	gcc -c -g rbtree.c

rbtest_test.o: rbtree_test.c
	gcc -c -g rbtree_test.c

test: rbtree.o rbtree_test.o
	gcc rbtree_test.o rbtree.o -g -o test

clean:
	rm rbtree.o rbtree_test.o test -f
