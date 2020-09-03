/*
 * rbtree.h
 *
 *  Created on: Mar 17, 2020
 *      Author: krad2
 */

#ifndef RBTREE_H_
#define RBTREE_H_

#include <stddef.h>
#include <stdint.h>
#include "port_config.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 *	Red-black tree structure definitions
 */

typedef struct __attribute__((aligned(sizeof(long)))) __rbnode {
    uintptr_t  __rb_parent_color;
    struct __rbnode *left;
    struct __rbnode *right;
} rbnode;

typedef struct __rbtree {
    rbnode *node;
} rbtree;

/**
 *	'Cached' red-black tree definitions
 */

typedef struct __rbtree_lcached {
    rbtree tree;
    rbnode *leftmost;	// logical min cached
} rbtree_lcached;

typedef struct __rbtree_rcached {
    rbtree tree;
    rbnode *rightmost;	// logical max cached
} rbtree_rcached;

typedef struct __rbtree_lrcached {
    rbtree tree;

    rbnode *leftmost;	// logical min cached
    rbnode *rightmost; 	// logical max cached
} rbtree_lrcached;

#define rb_first_cached(root)   (root)->leftmost
#define rb_last_cached(root)    (root)->rightmost

/**
 * Helper macros to get properties of a node
 */

#define RB_RED      0
#define RB_BLACK    1

#define __rb_color(pc)     ((pc) & 1)
#define __rb_is_black(pc)  __rb_color(pc)
#define __rb_is_red(pc)    (!__rb_color(pc))
#define rb_color(rb)       (((rb) == NULL) || __rb_color((rb)->__rb_parent_color))
#define rb_is_red(rb)      (((rb) != NULL) && __rb_is_red((rb)->__rb_parent_color))
#define rb_is_black(rb)    (((rb) == NULL) || __rb_is_black((rb)->__rb_parent_color))

#define __rb_parent(pc)    ((struct __rbnode *)(pc & ~1))
#define rb_parent(r)   __rb_parent((r)->__rb_parent_color)

#define rb_left(rb)         ((rb)->left)
#define rb_right(rb)        ((rb)->right)

#define rb_root(tree)        ((tree)->node)

/* 'empty' nodes are nodes that are known not to be inserted in an rbtree */
#define RB_EMPTY_NODE(node)  \
    (rb_parent((node)) == (node))
#define RB_CLEAR_NODE(node)  \
    ((node)->__rb_parent_color = (uintptr_t) (node))

#define RB_NULL_ROOT(root)  (rb_root(root) == NULL)

/**
 *	Helper macros to use the rbtree to link other structures
 */

#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})

#define rb_entry(ptr, type, member) container_of(ptr, type, member)
#define rb_entry_safe(ptr, type, member) \
    ({ typeof(ptr) ____ptr = (ptr); \
       ____ptr ? rb_entry(____ptr, type, member) : NULL; \
    })

/**
 *	API
 */

void rbnode_init(rbnode *node);

void rbtree_init(rbtree *root);
void rbtree_lcached_init(rbtree_lcached *root);
void rbtree_rcached_init(rbtree_rcached *root);
void rbtree_lrcached_init(rbtree_lrcached *root);

void rb_insert(rbtree *root, rbnode *node, int (*cmp)(const void *left, const void *right));
void rb_lcached_insert(rbtree_lcached *root, rbnode *node, int (*cmp)(const void *left, const void *right));
void rb_rcached_insert(rbtree_rcached *root, rbnode *node, int (*cmp)(const void *left, const void *right));
void rb_lrcached_insert(rbtree_lrcached *root, rbnode *node, int (*cmp)(const void *left, const void *right));

void rb_delete(rbtree *tree, rbnode *node,
               int (*cmp)(const void *left, const void *right), void (*copy)(const void *src, void *dst));
void rb_lcached_delete(rbtree_lcached *tree, rbnode *node,
               int (*cmp)(const void *left, const void *right), void (*copy)(const void *src, void *dst));
void rb_rcached_delete(rbtree_rcached *tree, rbnode *node,
               int (*cmp)(const void *left, const void *right), void (*copy)(const void *src, void *dst));
void rb_lrcached_delete(rbtree_lrcached *tree, rbnode *node,
               int (*cmp)(const void *left, const void *right), void (*copy)(const void *src, void *dst));

void rbtree_clean(rbtree *tree);
void rb_lcached_clean(rbtree_lcached *tree);
void rb_rcached_clean(rbtree_rcached *tree);
void rb_lrcached_clean(rbtree_lrcached *tree);

const rbnode *rb_find(const rbtree *root, const void *key, int (*cmp)(const void *left, const void *right));

const rbnode *rb_first(const rbtree *root);
const rbnode *rb_last(const rbtree *root);

const rbnode *rb_next(const rbnode *node);
const rbnode *rb_prev(const rbnode *node);

void rb_inorder_foreach(rbtree *tree, void (*cb)(void *key));
void rb_postorder_foreach(rbtree *tree, void (*cb)(void *key));
void rb_preorder_foreach(rbtree *tree, void (*cb)(void *key));

#ifdef __cplusplus
}
#endif

#endif /* RBTREE_H_ */
