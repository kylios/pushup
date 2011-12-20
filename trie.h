#ifndef TRIE_H
#define TRIE_H

#include "type.h"

typedef void trie_dump_func (const void*);

/* Predeclare the node type here. */
struct trie_node;

/**
 * The trie datastructure.  It has a node to represent the root node. 
 * */
struct trie
{
    struct trie_node* root;
    bool valid;
};

/* Initializes this trie. */
void trie_init (struct trie*);

/**
 * Inserts an element DATA into the trie indexed by STR.
 * */
bool trie_insert (struct trie*, const char* str, 
        const void* data);

/**
 * Searches the trie for a given element indexed by STR.  Returns NULL if
 * the string could not be found or if there is no data inserted at that
 * index.
 * */
const void* trie_find (struct trie*, const char* str);

/**
 * Removes the element indexed by STR if any exists.
 * */
void trie_remove (struct trie*, const char* str);

/**
 * Deallocates all memory used by this trie.
 * */
void trie_destroy (struct trie*);

/**
 * Prints this trie, and prints each element according to the specified 
 * display function.
 * */
void trie_print (struct trie*, trie_dump_func*);

#endif // TRIE_H
