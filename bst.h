#ifndef BST_H
#define BST_H  

#include    "type.h"

struct bst_node;

struct bst_node
{
    struct bst_node* left;
    struct bst_node* right;
    struct bst_node* parent;    /* Parent is required for iterators. */

    /* Pointer to the actual data object */
    void* data;
};

/* The comparator function for the red-black tree.
 * When we perform insertions or lookups on the index, the function that
 * defines the operation for comparisons must comply to this function typedef.
 * */
typedef int bst_tree_compare_func (const void*, const void*, 
        const void* AUX);
typedef void bst_tree_dump_func (const void*);


struct bst_tree
{
    /* the root node */
    struct bst_node* root;
    
    /* the comparator function */
    bst_tree_compare_func* comparator;
};





/* Initializes a new red black tree.
 * TREE must point to a valid block of memory
 * FUNC must be a comparator function used for comparing elements in the index
 * */
void bst_init (struct bst_tree* tree, bst_tree_compare_func* func);

/* Inserts the data pointed to by DATA into TREE
 * TREE must be a valid initialized tree
 * DATA must be a valid pointer
 * */
bool bst_insert (struct bst_tree* tree, void* data);

/* Deletes the data pointed to by DATA from TREE
 * TREE must be a valid initialized tree
 * DATA must be a valid pointer
 * */
void* bst_delete (struct bst_tree* tree, void* data);

/* Looks up a value from the tree.  If it finds a match, returns it, 
 * otherwise, returns NULL.
 * */
void* bst_find (struct bst_tree* tree, const void* elem);

/* Prints out the contents of TREE using FUNC to print the data 
 * (if supplied)
 * TREE must be a valid initialized tree
 * FUNC (optional) must be of type rb_tree_dump_func
 * */
void bst_dump (struct bst_tree* tree, bst_tree_dump_func* func);


/* BST iterator functions.  */
struct bst_iterator
{
    struct bst_node* node;
    struct bst_tree* tree;
};

struct bst_iterator* bst_get_iterator (struct bst_tree*);
struct bst_iterator* bst_get_reverse_iterator (struct bst_tree*);
void* bst_get (struct bst_iterator*);
void bst_itr_remove (struct bst_iterator*);
void* bst_next (struct bst_iterator*);
void* bst_prev (struct bst_iterator*);



#endif

