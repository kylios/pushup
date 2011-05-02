// src/lib/kernel/hash.c

//    src/lib/kernel/hash.c is part of Kylux.
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

#include <stddef.h>

#include "type.h"
#include "debug.h"
#include "lib/hash.h"
#include "lib/list.h"

static int find_in_list (struct list_elem* a, struct list_elem* b, 
        void* hash_func);

void hash_init (struct hash* hash, 
        hash_hash_func* hash_func, hash_compare_func* compare_func)
{
    unsigned i;

    ASSERT (hash != NULL);
    ASSERT (hash_func != NULL);
    ASSERT (compare_func != NULL);

    for (i = 0; i < HASH_BUCKETS; i++)
    {
        list_init (&hash->buckets[i]);
    }
    
    hash->func = hash_func;
    hash->cmp_func = compare_func;
};

void 
hash_insert (struct hash* hash, struct hash_elem* e)
{
    uint32 idx;
    struct list_elem* le;

    ASSERT (hash != NULL);
    ASSERT (e != NULL);
    ASSERT (hash->func != NULL);

    idx = hash->func (e) % HASH_BUCKETS;
    le = HASH_ELEM_TO_LIST_ELEM (e);
    list_push_back (&hash->buckets[idx], le);
};

struct hash_find_data
{
    void* aux;
    hash_compare_func* cmp;
};

struct hash_elem* 
hash_find (struct hash* hash, struct hash_elem* elem, void* aux)
{
    uint32 idx;
    struct list_elem* e;
    struct hash_find_data d;

    ASSERT (hash != NULL);
    ASSERT (elem != NULL);
    ASSERT (hash->func != NULL);
    ASSERT (hash->cmp_func != NULL);

    d.aux = aux;
    d.cmp = hash->cmp_func;

    idx = hash->func (elem) % HASH_BUCKETS;
    e = HASH_ELEM_TO_LIST_ELEM (elem);
    e = list_find (&hash->buckets[idx], e, &find_in_list, &d);

    if (e == NULL)
        return NULL;

    return LIST_ENTRY (e, struct hash_elem, elem);
};

int
find_in_list (struct list_elem* a, struct list_elem* b, void* aux)
{
    const struct hash_find_data* d;
    struct hash_elem* ha;
    struct hash_elem* hb;

    ASSERT (a != NULL);
    ASSERT (b != NULL);
    ASSERT (aux != NULL);

    d = (const struct hash_find_data*) aux;
    ha = LIST_ELEM_TO_HASH_ELEM (a);
    hb = LIST_ELEM_TO_HASH_ELEM (b);

    return d->cmp (ha, hb, d->aux);
};

void 
hash_remove (struct hash_elem* e)
{
    struct list_elem* le;

    ASSERT (e != NULL);
    le = HASH_ELEM_TO_LIST_ELEM (e);

    list_remove (le);
};

void
hash_print (struct hash* hash, hash_print_func* func)
{
    unsigned i;
    struct list* list;
    struct list_elem* e;

    ASSERT (hash != NULL);
    ASSERT (func != NULL);

    for (i = 0; i < HASH_BUCKETS; i++)
    {
        list = &hash->buckets[i];
        if (list_empty (list))
            continue;

        for (e = list_begin (list); e != list_end (list); 
                e = list_next (e))
        {
            func (e);
        }
    }
};

/* Some common hash functions */
uint32 
hash_string (struct hash_elem* str)
{
    
    return 0;
};

uint32 
hash_addr (struct hash_elem* addr)
{
    
    return 0;
};

