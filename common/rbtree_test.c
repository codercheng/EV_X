#include <stdio.h>
#include <stdlib.h>
#include "rbtree.h"


struct fd_node {
    struct rb_node rb;
    int fd;
};

int fd_insert(struct rb_root *root, struct fd_node *data)
{
    struct rb_node **new = &(root->rb_node), *parent = NULL;

    /* Figure out where to put new node */
    while (*new) {
        struct fd_node *this = container_of(*new, struct fd_node, rb);
        int result = data->fd - this->fd;

        parent = *new;
        if (result < 0)
            new = &((*new)->rb_left);
        else if (result > 0)
            new = &((*new)->rb_right);
        else
            return 0;
    }

    /* Add new node and rebalance tree. */
    rb_link_node(&data->rb, parent, new);
    rb_insert_color(&data->rb, root);

    return 1;
}

struct fd_node * fd_search(struct rb_root *root, int fd)
{
    struct rb_node *node = root->rb_node;

    while (node) {
        struct fd_node *data = container_of(node, struct fd_node, rb);
        int result = fd - data->fd;

        if (result < 0)
            node = node->rb_left;
        else if (result > 0)
            node = node->rb_right;
        else
            return data;
    }
    return NULL;
}

void fd_free(struct fd_node *node) {
    if (node) {
        free(node);
        node = NULL;
    }
}

void fd_delete(struct rb_root *root, struct fd_node *node) {
    if (node) {
        rb_erase(&node->rb, root);
    }
}

#define fd_entry_for_each(root, type) \
struct rb_node *node;\
  for (node = rb_first(&root); node; node = rb_next(node))\
    printf("fd=%d\n", ((type *)rb_entry(node, struct fd_node, rb))->fd);


int main()
{
    int i;
    struct rb_root fd_root = RB_ROOT;

    for (i = 0; i < 1024; i++) {
        struct fd_node *fd_ptr = (struct fd_node*)malloc(sizeof(struct fd_node));
        fd_ptr->fd = rand() % 1024;
        fd_insert(&fd_root, fd_ptr);
    }

    {
        struct rb_node *node;
        for (node = rb_first(&fd_root); node; node = rb_next(node)) {
            struct fd_node *fdptr = rb_entry(node, struct fd_node, rb);
            printf("fd=%d\n", fdptr->fd);
        }
    }
    //fd_entry_for_each(fd_root, struct fd_node);
    //fd_delete(&fd_root, fd_ptr);
    return 0;
}
