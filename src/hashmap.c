/*
 * hashmap.c
 *
 *  Created on: Oct 7, 2019
 *      Author: scharrernf
 */

#include <stdio.h>
#include <stdlib.h>

struct node
{
    int key;
    int value;
    struct node *next;
};

struct map
{
    int size;
    struct node **node_list;
};

struct map *createEmptyMap(int size)
{
	// Create an empty map of the desired size
    struct map *m = (struct map*)malloc(sizeof(struct map));
    m->size = size;
    m->node_list = (struct node**)malloc(sizeof(struct node*)*size);

    for(int i = 0; i < size; i++)
    {
        m->node_list[i] = NULL;
    }

    return m;
}

int getHashCode(struct map *m, int key)
{
	// Grab the hash code for the given key - gives the location of the key
    if(key < 0)
    {
        return -(key % m->size);
    }
    else
    {
    	return key % m->size;
    }
}

void insert(struct map *m, int key, int value)
{
    int position = getHashCode(m, key);
    struct node *current_node = m->node_list[position];
    struct node *new_node = (struct node*)malloc(sizeof(struct node));
    struct node *temp = current_node;

    // Navigate through the map until either the key is found (set the value) or until it gets to the end (create new)
    while(temp)
    {
        if(temp->key == key)
        {
            temp->value = value;
            return;
        }
        temp = temp->next;
    }

    new_node->key = key;
    new_node->value = value;
    new_node->next = current_node;
    m->node_list[position] = new_node;
}

int lookup(struct map *m,int key)
{
    int pos = getHashCode(m, key);
    struct node *current_node = m->node_list[pos];
    struct node *temp = current_node;

    // Try to find the key in the map by navigating linearly, otherwise return -1
    while(temp)
    {
        if(temp->key == key)
        {
            return temp->value;
        }
        temp = temp->next;
    }

    return -1;
}

struct map *initManchesterMap(void)
{
	// Build a map that contains the Manchester encoding for each hexadecimal character
	struct map *manchester_table = createEmptyMap(16);

	insert(manchester_table, 0x0, 0xAA);
	insert(manchester_table, 0x1, 0xA9);
	insert(manchester_table, 0x2, 0xA6);
	insert(manchester_table, 0x3, 0xA5);
	insert(manchester_table, 0x4, 0x9A);
	insert(manchester_table, 0x5, 0x99);
	insert(manchester_table, 0x6, 0x96);
	insert(manchester_table, 0x7, 0x95);
	insert(manchester_table, 0x8, 0x6A);
	insert(manchester_table, 0x9, 0x69);
	insert(manchester_table, 0xA, 0x66);
	insert(manchester_table, 0xB, 0x65);
	insert(manchester_table, 0xC, 0x5A);
	insert(manchester_table, 0xD, 0x59);
	insert(manchester_table, 0xE, 0x56);
	insert(manchester_table, 0xF, 0x55);

	return manchester_table;
}

struct map *initManchesterDecodeMap(void)
{
	struct map *manchester_table = createEmptyMap(16);

	insert(manchester_table, 0xAA, 0x0);
	insert(manchester_table, 0xA9, 0x1);
	insert(manchester_table, 0xA6, 0x2);
	insert(manchester_table, 0xA5, 0x3);
	insert(manchester_table, 0x9A, 0x4);
	insert(manchester_table, 0x99, 0x5);
	insert(manchester_table, 0x96, 0x6);
	insert(manchester_table, 0x95, 0x7);
	insert(manchester_table, 0x6A, 0x8);
	insert(manchester_table, 0x69, 0x9);
	insert(manchester_table, 0x66, 0xA);
	insert(manchester_table, 0x65, 0xB);
	insert(manchester_table, 0x5A, 0xC);
	insert(manchester_table, 0x59, 0xD);
	insert(manchester_table, 0x56, 0xE);
	insert(manchester_table, 0x55, 0xF);

	return manchester_table;
}
