#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct l_node {
   int fd;
   int channel_id;
   pthread_t thread;
   int key;
   struct l_node *next;
} l_node;

// typedef struct list_t {
//    l_node* first;
// } list_t;


//display the list
void print_list(l_node* head);

//insert link at the first location
l_node* add_first(l_node* head, int key, int fd);

// void modify_first()

//delete first item
l_node* delete_first(l_node* head);

//is list empty
bool is_empty(l_node* head);

int length(l_node* head);

//find a link with given key
l_node* find(l_node* head, int key);

l_node* delete(l_node* head, int key);
