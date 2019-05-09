#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct l_node {
   int fd;
   pthread_t thread;
   int key;
   struct l_node *next;
} l_node;

typedef struct list_t {
   l_node* first;
} list_t;


//display the list
void print_list(list_t* list);

//insert link at the first location
void add_first(list_t* list, int key, int fd);

// void modify_first()

//delete first item
l_node* delete_first(list_t* list);

//is list empty
bool is_empty(list_t* list);

int length(list_t* list);

//find a link with given key
l_node* find(list_t* list, int key);

l_node* delete(list_t* list, int key);
