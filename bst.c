
#include    <stdio.h>
#include    <stdlib.h>
#include    "debug.h"
#include    "bst.h"

/* helper functions for tree operations */
bool insert_helper (struct bst_node** node, struct bst_node* parent, void* data, 
        bst_tree_compare_func* func);
struct bst_node* find_helper (struct bst_node*, const void*, bst_tree_compare_func*);
void dump_helper (struct bst_node* node, unsigned, bst_tree_dump_func*);
void dump_node (struct bst_node* node, unsigned level);

void 
bst_init (struct bst_tree* tree, bst_tree_compare_func* func)
{
    if (!tree || !func)   return;
    
    tree->root = NULL;
    tree->comparator = func;
};

bool 
bst_insert (struct bst_tree* tree, void* data)
{
    if (!data)  return false;
    return insert_helper (&(tree->root), NULL, data, tree->comparator);
};

void* 
bst_delete (struct bst_tree* tree, void* data)
{
    if (!data || !tree)
        return NULL;

    struct bst_node* del = bst_find (tree, data);

    /* Case 1: Node is a leaf */
    if (del->left == NULL && del->right == NULL)
    {
        if (!del->parent)   tree->root = NULL;
        else if (del->parent->right == del) del->parent->right = NULL;
        else                                del->parent->left = NULL;
    }

    /* Case 2: Node has one child */
    else if (del->left)
    {
        if (del->parent == NULL)    tree->root = del->left;
        else if (del->parent->right == del) del->parent->right = del->left;
        else                                del->parent->left = del->left;
    }
    else if (del->right)
    {
        if (del->parent == NULL)    tree->root = del->right;
        else if (del->parent->right == del) del->parent->right = del->right;
        else                                del->parent->left = del->left;
    }

    /* Case 3: Node has both children */
    else
    {
        ASSERT (del->left);
        ASSERT (del->right);

        struct bst_node* right = del->right;
        /* Find the leftmost child */
        struct bst_node* n = right;
        while (n->left)
            n = n->left;
        ASSERT (n->left == NULL);
        if (n->right)
        {
            n->parent->left = n->right;
            n->right->parent = n->parent;
        }
        n->right = del->right;
        n->left = del->left;
        n->parent = del->parent;
        if (del->parent == NULL)    tree->root = n;
        else if (del->parent->right == del) del->parent->right = n;
        else                                del->parent->left = n;
    }

    void* ret = del->data;
    free (del);
    return ret;
};

void*
bst_find (struct bst_tree* tree, const void* elem)
{
    ASSERT (tree != NULL);

    if (elem == NULL)   return NULL;

    struct bst_node* node = find_helper (tree->root, elem, tree->comparator);
    if (node == NULL)
        return NULL;
    return node->data;
};

void 
bst_dump (struct bst_tree* tree, bst_tree_dump_func* func)
{
    dump_helper (tree->root, 0, func);   
};


/*******
  Iterator Functions
  *********/
struct bst_iterator* 
bst_get_iterator (struct bst_tree* tree)
{
    if (tree == NULL || tree->root == NULL) return NULL;
    struct bst_iterator* it = malloc (sizeof (struct bst_iterator));
    if (it == NULL)
        return NULL;
    it->node = tree->root;
    it->tree = tree;
    /* Go to the smallest element in the tree... The left-most. */
    while (it->node->left)
        it->node = it->node->left;
    return it;
};

struct bst_iterator* 
bst_get_reverse_iterator (struct bst_tree* tree)
{
    if (tree == NULL)   return NULL;
    struct bst_iterator* it = malloc (sizeof (struct bst_iterator));
    if (it == NULL) return NULL;
    it->node = tree->root;
    it->tree = tree;
    /* Go to the largest element in the tree... The right-most. */
    while (it->node->right)
        it->node = it->node->right;
    return it;
};

void* 
bst_get (struct bst_iterator* it)
{
    if (it == NULL) return NULL;
    return it->node->data;
};

void 
bst_itr_remove (struct bst_iterator* it)
{
    if (it == NULL) return;
    struct bst_node* node = it->node;
    bst_next (it);
    bst_delete (it->tree, node->data);
};

void* 
bst_next (struct bst_iterator* it)
{
    if (it == NULL) return NULL;
    if (it->node->right)
    {
        it->node = it->node->right;
        while (it->node->left)
            it->node = it->node->left;
    }
    else if (it->node->parent)
    {
        /* We need to go up the tree. */
        if (it->node == it->node->parent->left)
            it->node = it->node->parent;
        else
        {
            while (it->node == it->node->parent->right)
            {
                it->node = it->node->parent;
                if (!it->node->parent)
                    return NULL;
            }
            it->node = it->node->parent;
        }
    }
    else
    {
        return NULL;
    }
    return it->node->data;
};

void* 
bst_prev (struct bst_iterator* it)
{
    if (it == NULL) return NULL;
    if (it->node->left)
    {
        it->node = it->node->left;
        while (it->node->right)
            it->node = it->node->right;
    }
    else if (it->node->parent)
    {
        if (it->node == it->node->parent->right)
            it->node = it->node->parent;
        else
        {
            while (it->node == it->node->parent->left)
            {
                it->node = it->node->parent;
                if (!it->node->parent)
                    return NULL;
            }
            it->node = it->node->parent;
        }
    }
    else
    {
        return NULL;
    }
    return it->node->data;
};



/* ===  HELPER FUNCTIONS === */
bool 
insert_helper (struct bst_node** node, struct bst_node* parent, void* data, 
        bst_tree_compare_func* func)
{
    ASSERT (node != NULL);
    ASSERT (data != NULL);

    struct bst_node* new_node = NULL;
    if (*node == NULL)  
    {
        new_node = malloc (sizeof (struct bst_node));
        new_node->left      = NULL;
        new_node->right     = NULL;
        new_node->parent    = parent;
        new_node->data      = data;
    
        (*node) = new_node;

        return true;
    }   
    else    
    {
        new_node = (*node);
        ASSERT (new_node->data != NULL);

        int compare_result = func (data, (*node)->data, NULL);
            
        if (compare_result > 0) 
        {
            return insert_helper (&(*node)->right, *node, data, func);
        }   
        else if (compare_result < 0) 
        {
            return insert_helper (&(*node)->left, *node, data, func);
        }
        else
        {
            return false;
        }
    }
};

struct bst_node*
find_helper (struct bst_node* node, const void* elem, 
        bst_tree_compare_func* func)
{
    ASSERT (elem != NULL);

    if (node == NULL)   return NULL;

    ASSERT (node->data != NULL);
    int cmp = func (elem, node->data, NULL);

    if (cmp > 0)
    {
        return find_helper (node->right, elem, func);
    }
    else if (cmp < 0) 
    {
        return find_helper (node->left, elem, func);
    }
    else
    {
        return node;
    }
};

void 
dump_helper (struct bst_node* node, unsigned level, 
        bst_tree_dump_func* func)
{
    if (node)   
    {
        dump_helper (node->left, level + 1, func);

        unsigned i = 0;
        for (i = 0; i < level; i++)    
        {
            printf ("   ");
        } 
        if (node->data) 
        {
            func (node->data);
        }   
        else    
        {
            printf ("NULL");
        }
        printf ("\n");
        dump_helper (node->right, level + 1, func);
    }
};

void
dump_node (struct bst_node* node, unsigned level)
{
    
};
