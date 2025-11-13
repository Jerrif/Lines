/*
 * linked_lists.h
 *
 *  Created on: Apr 27, 2018
 *      Author: J
 */

#ifndef LINKED_LISTS_H_
#define LINKED_LISTS_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "rgb_struct.h"

typedef struct node
{
	RGBVALUES data;
	struct node *next;
} node;

void print_list(node* head);
int count_nodes(node* head);
node* create(RGBVALUES data, node* next);
node* prepend(RGBVALUES data, node* head);
node* pop(node* head); // not sure I'll need these
node* push(node* head);
bool find_number(RGBVALUES num, node* head); // Don't think this one needs a key; it searches for the value itself
node* remove_node(int node_to_remove, node* head); // Do I need this?
void free_list(node* head);

void free_list(node* head)
{
    node* cursor = head;
    node* temp = NULL;

    while(cursor != NULL)
    {
        temp = cursor->next;
        free(cursor);
        cursor = temp;
    }
}

node* remove_node(int node_to_remove, node* head)
{
	int count = count_nodes(head);

	if(head == NULL)
	{
		printf("List is empty\n");
		return NULL;
	}

	if(node_to_remove > count || node_to_remove <= 0)
	{
		printf("Node to remove is outside of list\n");
		return head;
	}

	if(node_to_remove == count) // if it's the last node in the list
	{
		printf("Pushing\n");
		return push(head);
	}

	if(node_to_remove == 1)
	{
		printf("Popping\n");
		return pop(head);
	}

	node* cursor = head;
	node* previous = cursor;
	int i = 1;

	while(i != node_to_remove && cursor != NULL)
	{
		previous = cursor;
		cursor = cursor->next;
		i++;
	}

	if(cursor == NULL)
	{
		printf("Node to remove not found\n");
		return head;
	}

	previous->next = cursor->next;
	free(cursor);

	return head;
}

bool find_number(RGBVALUES num, node* head)
{
	node* cursor = head;
	while(cursor != NULL)
	{
		if(cursor->data.red == num.red && cursor->data.green == num.green && cursor->data.blue == num.blue)
		{
			return true;
		}

		cursor = cursor->next;
	}

	return false;
}

node* push(node* head) // Deletes the last item
{
	if(head == NULL) // If there's no other item
	{
		printf("Cannot push empty list\n");
		return NULL;
	}
	node* cursor = head;
	node* previous = NULL;
	while(cursor->next != NULL)
	{
		previous = cursor;
		cursor = cursor->next;
	}

	if(previous != NULL)
	{
		previous->next = NULL;
	}

	if(cursor == head)
	{
		head = NULL;
	}

	free(cursor);

	return head;
}

node* pop(node* head)
{
	if(head == NULL)
	{
		printf("Cannot pop empty list\n");
		return NULL;
	}
	node* front = head;
	head = head->next;
	free(front);
	return head;
}

node* create(RGBVALUES data, node* next)
{
	node* new_node = malloc(sizeof(node));
	if(new_node == NULL)
	{
		printf("Error creating new node\n");
		exit(0);
	}
	new_node->data = data;
	new_node->next = next;

	return new_node;
}

node* prepend(RGBVALUES data, node* head)
{
	node* new_node = create(data, head);
	head = new_node;
	return head;
}

void traverse(node* head)
{
	node* cursor = head;
	while(cursor != NULL)
	{
		cursor = cursor->next;
	}
}

void print_list(node* head)
{
	node* cursor = head;
	int i = 1;
	while(cursor != NULL)
	{
		printf("Position[%d]: (%u, %u, %u)\n", i, cursor->data.red, cursor->data.green, cursor->data.blue);
		cursor = cursor->next;
		i++;
	}
	printf("Done\n");
}

int count_nodes(node* head)
{
	node* cursor = head;
	int count = 0;
	while(cursor != NULL)
	{
		cursor = cursor->next;
		count++;
	}
	return count;
}


#endif /* LINKED_LISTS_H_ */
