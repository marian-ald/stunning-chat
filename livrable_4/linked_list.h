#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

typedef struct l_node {
   int fd;
   int channel_id;
   pthread_t thread;
   int key;
   struct l_node *next;
} l_node;

l_node* add_first(l_node* head, int key, int fd);

l_node* find(l_node* head, int key);

l_node* rm_node(l_node* head, int key);
