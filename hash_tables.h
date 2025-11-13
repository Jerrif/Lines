/*
 * hash_tables.h
 *
 *  Created on: Apr 28, 2018
 *      Author: J
 */

#ifndef HASH_TABLES_H_
#define HASH_TABLES_H_

#include <stdio.h>
#include <string.h>
#include "linked_lists_hash_tables.h"

//node hashtable[size];
//
//char* create_table(uint32_t size)
//{
//	node* hash_table[size];
//	return hash_table;
//}

uint32_t hash(RGBVALUES val) // MAYBE I shouldn't use a function for this? It might be more efficient as just an inline call.
{
	return (val.red * val.green + val.blue) % (255 * 255); // The best one?
}

#endif /* HASH_TABLES_H_ */
