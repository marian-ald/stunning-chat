#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "linked_list.h"
#include "helpers.h"



//display the list
void print_list(l_node* head) {
   // l_node* head = list->first;

   l_node *ptr = head;
   printf("\n[ ");
	
   //start from the beginning
   while(ptr != NULL) {
      printf("(%d,%d) ",ptr->key,ptr->fd);
      ptr = ptr->next;
   }
	
   printf(" ]");
}

//insert new at the first location
l_node* add_first(l_node* head, int key, int fd) {
   // l_node* head = list->first;

   //create a new
   l_node *new = (l_node*) malloc(sizeof(l_node));
	
   new->key = key;
   new->fd = fd;
	
   //point it to old first l_node
   new->next = head;
	
   //point first to new first l_node
   head = new;

   return head;
}

//delete first item
l_node* delete_first(l_node* head) {
   // l_node* head = list->first;

   //save reference to first new
   l_node *tempLink = head;
	
   //mark next to first new as first 
   head = head->next;
	
   //return the deleted new
   return tempLink;
}

//is list empty
bool is_empty(l_node* head) {
   return head == NULL;
}

int length(l_node* head) {
   // l_node* head = list->first;
   int length = 0;
   l_node *current;
	
   for(current = head; current != NULL; current = current->next) {
      length++;
   }
	
   return length;
}

//find a new with given key
l_node* find(l_node* head, int key) {
   // l_node* head = list->first;

   //start from the first new
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
         //go to next new
         current = current->next;
      }
   }      
	
   //if fd found, return the current Link
   return current;
}

//delete a new with given key
l_node* delete(l_node* head, int key) {
   // l_node* head = list->first;

   //start from the first new
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
         //store reference to current new
         previous = current;
         //move to next new
         current = current->next;
      }
   }

   //found a match, update the new
   if(current == head) {
      //change first to point to next new
      head = head->next;
   } else {
      //bypass the current new
      previous->next = current->next;
   }    
	
   return current;
}

