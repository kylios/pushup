#include <stdio.h>
#include <stdlib.h>
#include "trie.h"
#include "bst.h"
#include "debug.h"

struct trie_node
{
    /* The data that is stored at this node. */
    const void* data;

    /* Pointers to child nodes, if any.  This structure stores char-pointer
       pairs, indexed by chars. */
    struct bst_tree ptrs;
};

/* This struct object is inserted into the pointer bst in each of the 
   trie nodes. */
struct char_idx
{
    char c;
    struct trie_node* n;
};

/**
 * This function compares the values of two char variables and returns
 * A - B.  This is used for traversing the pointer bst in each TRIE_NODE.
 * */
int
char_compare (const void* _a, const void* _b, const void* AUX)
{
    const struct char_idx* a = (struct char_idx*) _a;
    const struct char_idx* b = (struct char_idx*) _b;

    return a->c - b->c;
};

bool
node_init (struct trie_node** node)
{
    *node = malloc (sizeof (struct trie_node));

    if (*node == NULL)
        return false;

    (*node)->data = NULL;
    bst_init (&(*node)->ptrs, char_compare);

    return true;
};

/* Helper functions */
bool trie_insert_helper (struct trie_node*, const char* c, const void* data);
const void* trie_find_helper (struct trie_node*, const char* c);
void trie_remove_helper (struct trie_node*, const char*);
void trie_print_helper (struct trie_node*, int depth, char* c, trie_dump_func*);

void
trie_init (struct trie* trie)
{
    if (trie == NULL)
        return;

    trie->valid = node_init (&trie->root);
};

bool
trie_insert (struct trie* trie, const char* str, const void* data)
{
    if (trie == NULL || str == NULL || *str == '\0')
        return false;

    return trie_insert_helper (trie->root, str, data);
};

const void* 
trie_find (struct trie* trie, const char* str)
{
    if (trie == NULL || str == NULL || *str == '\0')
        return NULL;

    return trie_find_helper (trie->root, str);
};

void 
trie_remove (struct trie* trie, const char* str)
{
    if (trie == NULL || str == NULL)
        return;

    trie_remove_helper (trie->root, str);
};

void
trie_print (struct trie* trie, trie_dump_func* func)
{
    if (trie == NULL || func == NULL)
        return;
    char c[128];
    trie_print_helper (trie->root, 0, c, func);    
};

void
trie_print_helper (struct trie_node* node, int depth, 
        char* c, trie_dump_func* func)
{
    ASSERT (node);
    struct bst_iterator* it = bst_get_iterator (&node->ptrs);
    if (!it)    return;
    struct char_idx* id = bst_get (it);
    do
    {
        c[depth] = id->c;
        struct trie_node* n = id->n;
        if (n)
        {
            if (n->data)
            {
                c[depth + 1] = '\0';
                printf ("%10s", c);
                func (n->data);
                printf ("\n");
            }
            trie_print_helper (n, depth + 1, c, func);
        }
    } while ((id = bst_next (it)));
    free (it);
};

bool 
trie_insert_helper (struct trie_node* node, const char* c, const void* data)
{
    ASSERT (node);
    ASSERT (*c != '\0');

    struct char_idx* idx = malloc (sizeof (struct char_idx));
    if (idx == NULL)
        return false;
    
    idx->c = *c;
    
    struct char_idx* found = bst_find (&node->ptrs, idx);
    
    /* if it couldn't find anything then we must insert a new node into
       the BST. */
    if (found == NULL)
    {
        bst_insert (&node->ptrs, idx);

        if (!node_init (&idx->n))
        {
            free (idx);
            return false;
        }
    }
    else
    {
        free (idx);
        idx = found;
    }

    /* IDX now points to the mapping for the character we need to go to 
       next. */
    ASSERT (idx);
    ASSERT (idx->c == *c);
    ASSERT (idx->n);

    struct trie_node* next = idx->n;
    
    /* If we've reached the end of the input string, we need to insert the
       data here. */
    if (*++c == '\0')
    {
        /* If there is already data at that index, return false. */
        if (next->data)
            return false;

        next->data = data;

        return true;
    }
    else
    {
        return trie_insert_helper (next, c, data);
    }
};

const void*
trie_find_helper (struct trie_node* node, const char* c)
{
    ASSERT (node);
    ASSERT (*c != '\0');

    /* This only needs to be temporary since we are just using it for
       lookup. */
    struct char_idx idx = {*c, NULL};

    /* Look up this letter in the node's bst to get the next node. */
    struct char_idx* found = bst_find (&node->ptrs, &idx);

    if (!found)
    {
        return NULL;
    }
    struct trie_node* next = found->n;
    
    /* Check if we are at the end.  If so, then return the data at next. */
    if (*++c == '\0')
    {
        return next->data;
    }
    return trie_find_helper (next, c);
};

void
trie_remove_helper (struct trie_node* node, const char* str)
{
    ASSERT (node);
    ASSERT (*str != '\0');

    if (*++str == '\0')
    {
        node->data = NULL;
        return;
    }
    // TODO finish this function
    return;
};
