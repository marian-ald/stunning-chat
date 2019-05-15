#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "linked_list.h"
#include "helpers.h"

/* Insert a new node at the beginning of the list*/
l_node* add_first(l_node* head, int key, int fd)
{
	/* Allocate memory for the new node*/
	l_node *new = (l_node*) malloc(sizeof(l_node));

	new->key = key;
	new->fd = fd;

	new->next = head;	
	head = new;

	return head;
}

/* Find a node with a certain key */
l_node* find(l_node* head, int key)
{
	l_node* crt = head;

	/* If list is empty*/
	if(head == NULL) {
		return NULL;
	}

	/* Navigate through the list */
	while(crt->key != key)
    {
		/*If it is last l_node */
		if(crt->next == NULL)
        {
			return NULL;
		} else {
			crt = crt->next;
		}
	}		
	
	return crt;
}

/* Delete a new with given key */
l_node* rm_node(l_node* head, int key)
{
	l_node* crt = head;
	l_node* previous = NULL;
    l_node* aux = NULL;

	if(head == NULL)
    {
		return NULL;
	}

    /* Navigate through the list */
	while(crt->key != key)
    {
		if(crt->next == NULL)
        {
			return NULL;
		}
        else
        {
			previous = crt;
			crt = crt->next;
		}
	}

	if(crt == head)
    {
        aux = head;
		head = head->next;
	}
    else
    {
        aux = previous->next;
		previous->next = crt->next;
	}
    free(aux);
	return head;
}

