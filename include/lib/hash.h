// include/lib/kernel/hash.h

//    include/lib/kernel/hash.h is part of Kylux.
//
//    Kylux is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    Kylux is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with Kylux.  If not, see <http://www.gnu.org/licenses/>.

// Author: Kyle Racette
// Date: 2010-02-05 11:54

#ifndef LIB_KERNEL_HASH_H
#define LIB_KERNEL_HASH_H

#include <stddef.h>

#include "type.h"
#include "lib/list.h"

#define HASH_BUCKETS 254

/* All objects that go into this hash should have a hash_elem member. */
struct hash_elem
{
    struct list_elem elem;
};

#define HASH_ENTRY(ELEM, TYPE, MEMBER)    \
    ((TYPE*) ((ELEM) - offsetof(TYPE, MEMBER)))

#define HASH_ELEM_TO_LIST_ELEM(ELEM)    \
    ((struct list_elem*) (((void*)ELEM) +     \
        offsetof(struct hash_elem, elem)))

#define LIST_ELEM_TO_HASH_ELEM(ELEM)    \
    LIST_ENTRY(ELEM, struct hash_elem, elem)

#define HASH_ELEM_INITIALIZER   \
{           \
    LIST_ELEM_INITIALIZER \
}

/* The definition for a hash function */
typedef uint32 hash_hash_func (struct hash_elem* e);

/* Compare two hash elements */
typedef int hash_compare_func (struct hash_elem* a, struct hash_elem* b,
        void* aux);

/* Prints a hash_elem */
typedef void hash_print_func (struct list_elem* e);

struct hash
{
    /* This is the function that will be used to hash elements before
       they are put into the table */
    hash_hash_func* func;

    /* This is the function that will be used to compare two elements */
    hash_compare_func* cmp_func;

    /* Used when we call hash_find: the AUX value is stored here */
    void* aux;

    /* A list of buckets.  When an element is hashed, a value is returned
       and the element gets placed in the corresponding bucket. */
    struct list buckets[HASH_BUCKETS];
};

void hash_init (struct hash* hash, 
        hash_hash_func* hash_func, hash_compare_func* compare_func);
void hash_insert (struct hash* hash, struct hash_elem* e);
struct hash_elem* hash_find (struct hash* hash, struct hash_elem* e, 
        void* aux);
void hash_remove (struct hash_elem* e);
void hash_print (struct hash* hash, hash_print_func* func);

/* Some common hash functions */
uint32 hash_string (struct hash_elem* str);
uint32 hash_addr (struct hash_elem* addr);

/**
 * Basic forward iterator for the hash.  Example:
 *
 *  struct hash_iter it;
 *  struct hash_iter* i;
 *  for (i = hash_start (hash, &it); i != hash_end (i); 
 *           i = hash_next (i))
 *  {
 *      ... do something with i->elem
 *  }
 *
 * */

struct hash_iter
{
    struct hash_elem* elem;
    int level;
};

struct hash_iter* hash_start (struct hash*, struct hash_iter*);
struct hash_iter* hash_next (struct hash*, struct hash_iter*);
struct hash_iter* hash_end ();

typedef void hash_action_func (struct hash_elem*, void* aux);

void hash_apply (struct hash*, hash_action_func* func);


#endif  // LIB_KERNEL_HASH_H
