#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "linked_list.h"
#include "helpers.h"



//display the list
void print_list(list_t *list) {
   l_node* head = list->first;

   l_node *ptr = head;
   printf("\n[ ");
	
   //start from the beginning
   while(ptr != NULL) {
      printf("(%d,%d) ",ptr->key,ptr->fd);
      ptr = ptr->next;
   }
	
   printf(" ]");
}

//insert link at the first location
void add_first(list_t* list, int key, int fd) {
   // l_node* head = list->first;

   //create a link
   l_node *link = (l_node*) malloc(sizeof(l_node));
	
   link->key = key;
   link->fd = fd;
	
   //point it to old first l_node
   link->next = list->first;
	
   //point first to new first l_node
   list->first = link;
}

//delete first item
l_node* delete_first(list_t* list) {
   l_node* head = list->first;


   //save reference to first link
   l_node *tempLink = head;
	
   //mark next to first link as first 
   head = head->next;
	
   //return the deleted link
   return tempLink;
}

//is list empty
bool is_empty(list_t* list) {
   l_node* head = list->first;
   return head == NULL;
}

int length(list_t* list) {
   l_node* head = list->first;
   int length = 0;
   l_node *current;
	
   for(current = head; current != NULL; current = current->next) {
      length++;
   }
	
   return length;
}

//find a link with given key
l_node* find(list_t* list, int key) {
   l_node* head = list->first;

   //start from the first link
   l_node* current = head;

   //if list is empty
   if(head == NULL) {
      return NULL;
   }

   //navigate through list
   while(current->key != key) {
	
      //if it is last l_node
      if(current->next == NULL) {
         return NULL;
      } else {
         //go to next link
         current = current->next;
      }
   }      
	
   //if fd found, return the current Link
   return current;
}

//delete a link with given key
l_node* delete(list_t* list, int key) {
   l_node* head = list->first;

   //start from the first link
   l_node* current = head;
   l_node* previous = NULL;
	
   //if list is empty
   if(head == NULL) {
      return NULL;
   }

   //navigate through list
   while(current->key != key) {

      //if it is last l_node
      if(current->next == NULL) {
         return NULL;
      } else {
         //store reference to current link
         previous = current;
         //move to next link
         current = current->next;
      }
   }

   //found a match, update the link
   if(current == head) {
      //change first to point to next link
      head = head->next;
   } else {
      //bypass the current link
      previous->next = current->next;
   }    
	
   return current;
}

