/*
 * rbtree.c
 *
 *  Created on: Mar 21, 2020
 *      Author: krad2
 */

#include "rbtree.h"

/**
 *	Helper functions for __rb_parent_color member of rbnode.
 */

static inline void __rb_set_red(rbnode *rb) {
    if (!rb) return;
    rb->__rb_parent_color &= ~RB_BLACK;
	rb->__rb_parent_color |= RB_RED;
}

static inline void __rb_set_black(rbnode *rb) {
    if (!rb) return;
    rb->__rb_parent_color |= RB_BLACK;
}

static inline void __rb_set_color(rbnode *rb, int color) {
    if (!rb) return;

	if (color == RB_BLACK) __rb_set_black(rb);
	else __rb_set_red(rb);
}

static inline void __rb_set_parent(rbnode *rb, rbnode *parent) {
    if (!rb) return;
    rb->__rb_parent_color = rb_color(rb) | ((uintptr_t) parent);
}

static inline void __rb_set_parent_and_color(rbnode *rb,
                       rbnode *parent, int color) {
    if (!rb) return;
    __rb_set_parent(rb, parent);
	__rb_set_color(rb, color);
}

/**
 *	Basic BST insertion algorithm - no balancing yet!
 */

static inline void __rb_insert_basic(rbnode *root, rbnode *node,
                                       int (*cmp)(const void *left, const void *right)) {

    rbnode *cursor = root;
    rbnode *cursor_parent = rb_parent(cursor);

    rbnode *next;
    uint8_t left = 0;
    while (cursor != NULL) {
        if (cmp((void *) node, (void *) cursor) < 0) {
            next = rb_left(cursor);
            left = 1;
        } else {
            next = rb_right(cursor);
            left = 0;
        }

        cursor_parent = cursor;
        cursor = next;
    }

    if (left) {
        rb_left(cursor_parent) = node;
    } else {
        rb_right(cursor_parent) = node;
    }

    rb_left(node) = NULL;
    rb_right(node) = NULL;

    __rb_set_parent_and_color(node, cursor_parent, RB_RED);
}

/**
 *	Fetches the other child of the given node's parent, if it exists.
 */

static inline rbnode *__rb_sibling(const rbnode *node) {

    rbnode *parent, *sibling;
    parent = node ? rb_parent(node) : NULL;

    if (parent == NULL || node == NULL) sibling = NULL;
    else if (rb_left(parent) == node) sibling = rb_right(parent);
    else sibling = rb_left(parent);

    return sibling;
}

/**
 *	Links 'new' in place of 'old' on the side of 'root' that 'old' was on.
 */

static inline void __rb_replace_child(rbnode *root, rbnode *old, rbnode *nw) {
    if (!root) return;
    if (!old) return;

    if (rb_left(root) == old) rb_left(root) = nw;
    else if (rb_right(root) == old) rb_right(root) = nw;

	__rb_set_parent(nw, root);
}

/**
 * Swaps colors between 2 nodes.
 */

static inline void __rb_swap_colors(rbnode *src, rbnode *dst) {
    unsigned int scolor_old = rb_color(src);        // back up src color
    __rb_set_color(src, rb_color(dst));             // change src to dst color
    __rb_set_color(dst, scolor_old);                // change dst to old src color
}

/*
 *	Tree rotation operations centered on 'root'.
 */

static inline void __rb_left_rotate(rbnode *root) {

    rbnode *upper_root, *pivot;

    upper_root = rb_parent(root); 				// 'master tree' containing the subtree being rotated
    pivot = rb_right(root);						// new root of that subtree

    rb_right(root) = rb_left(pivot);			// link the current root's right subtree as the new root's left
    __rb_set_parent(rb_right(root), root);

    rb_left(pivot) = root;						// link the old root as the left subtree of the new root
    __rb_set_parent(rb_left(pivot), pivot);

    __rb_set_parent(pivot, upper_root);			// update the subtree's connection to the master
    __rb_replace_child(upper_root, root, pivot);
}

static inline void __rb_right_rotate(rbnode *root) {

    rbnode *upper_root, *pivot;

    upper_root = rb_parent(root);
    pivot = rb_left(root);

    rb_left(root) = rb_right(pivot);
    __rb_set_parent(rb_left(root), root);

    rb_right(pivot) = root;
    __rb_set_parent(rb_right(pivot), pivot);

    __rb_set_parent(pivot, upper_root);
    __rb_replace_child(upper_root, root, pivot);
}

/**
 *	Red-black tree ancestor transformations centered on 'node' for insertions.
 */

static inline void __rb_ins_ll_transform(rbnode *node, rbnode *parent, rbnode *grandparent) {
	__rb_swap_colors(parent, grandparent);
    __rb_right_rotate(grandparent);
}

static inline void __rb_ins_lr_transform(rbnode *node, rbnode *parent) {

    __rb_left_rotate(parent); // left-rotate to convert it to the LL case

	// reassignments to properly set up for LL case handling
	// the ancestors of the LL-transform 'center' are remapped because the original center 'node' is now the parent of the new one
	rbnode *__new_center, *__new_center_parent, *__new_center_grandparent;

	__new_center = parent;	// parent is the child of node after the rotation
	__new_center_parent = rb_parent(__new_center);
	__new_center_grandparent = rb_parent(__new_center_parent);

	__rb_ins_ll_transform(__new_center, __new_center_parent, __new_center_grandparent);
}

static inline void __rb_ins_rr_transform(rbnode *node, rbnode *parent, rbnode *grandparent) {
	__rb_swap_colors(parent, grandparent);
    __rb_left_rotate(grandparent);
}

static inline void __rb_ins_rl_transform(rbnode *node, rbnode *parent) {

    __rb_right_rotate(parent);

	rbnode *__new_center, *__new_center_parent, *__new_center_grandparent;

	__new_center = parent;	// parent is the child of node after the rotation
	__new_center_parent = rb_parent(__new_center);
	__new_center_grandparent = rb_parent(__new_center_parent);

	__rb_ins_rr_transform(__new_center, __new_center_parent, __new_center_grandparent);
}

/**
 * Red-black tree recoloring transformation centered on 'node'.
 */

static inline void __rb_ins_ancestor_recolor(rbnode *parent, rbnode *uncle, rbnode *grandparent) {
    __rb_set_black(parent);
    __rb_set_black(uncle);
    __rb_set_red(grandparent);
}

/**
 * Red-black tree rebalancing centered on 'node'. Called after insertion.
 */

static inline void __rb_insert_rebalance(rbnode *node) {

    rbnode *parent, *uncle, *grandparent;

    for (;;) {

        parent = node ? rb_parent(node) : NULL;
        uncle = __rb_sibling(parent);
        grandparent = parent ? rb_parent(parent) : NULL;

		// hitting the root means we're done - make sure it's black afterwards.
        if (parent == NULL) {
			__rb_set_black(node);
            return;
        }

		// if the node we've hit is is black then the subtree formed by it is going to be balanced
		if (rb_is_black(node)) {
			return;
		}

		// same thing as above for degenerate sizes of trees (prevents segfault)
		if (rb_is_black(parent)) {
			return;
		}

		// try a recolor first
		if (rb_is_red(uncle)) {
            __rb_ins_ancestor_recolor(parent, uncle, grandparent);
            node = grandparent;
			continue;
        } 

		// ...otherwise do rotations

		// left-left
		if ((parent == rb_left(grandparent)) && (node == rb_left(parent))) {
			__rb_ins_ll_transform(node, parent, grandparent);
		}

		// left-right
		else if ((parent == rb_left(grandparent)) && (node == rb_right(parent))) {
			__rb_ins_lr_transform(node, parent);
		}

		// right-right
		else if ((parent == rb_right(grandparent)) && (node == rb_right(parent))) {
			__rb_ins_rr_transform(node, parent, grandparent);
		}

		// right-left
		else if ((parent == rb_right(grandparent)) && node == (rb_left(parent))) {
			__rb_ins_rl_transform(node, parent);
		}

		// work your way up the tree
		node = parent;
    }
}

/**
 *	Sets tree structures to default state.
 */

void rbtree_init(rbtree *tree) {
    if (!tree) return;

    rb_root(tree) = NULL;
}

void rbtree_lcached_init(rbtree_lcached *tree) {
	rbtree_init(&tree->tree);
	rb_first_cached(tree) = NULL;
}

void rbtree_rcached_init(rbtree_rcached *tree) {
	rbtree_init(&tree->tree);
	rb_last_cached(tree) = NULL;
}

void rbtree_lrcached_init(rbtree_lrcached *tree) {
	rbtree_init(&tree->tree);
	rb_first_cached(tree) = NULL;
	rb_last_cached(tree) = NULL;
}

static inline void __rb_node_clear(rbnode *node) {
	if (!node) return;

    rb_left(node) = NULL;
    rb_right(node) = NULL;
    RB_CLEAR_NODE(node);
}

void rbnode_init(rbnode *node) {
	__rb_node_clear(node);
}

/**
 *	Red-black insertion given a comparator.
 */

void rb_insert(rbtree *tree, rbnode *node,
		int (*cmp)(const void *left, const void *right)) {
    if (!tree) return;
    if (!node) return;
    if (!cmp) return;

    // if the tree is empty, then set the root of the tree to a known black node
    if (RB_NULL_ROOT(tree)) {
        rb_root(tree) = node;
        __rb_set_parent_and_color(rb_root(tree), NULL, RB_BLACK);
        return;
    }

    // insertion with red coloring followed by autobalancing
    __rb_insert_basic(rb_root(tree), node, cmp);
    __rb_insert_rebalance(node);

    // retrace the root (has no parent)
    while (rb_parent(node) != NULL) {
        node = rb_parent(node);
    }

    rb_root(tree) = node;
}

/**
 *	Insertion into 'cached' trees is the same as above, but maintains a running max / min.
 */

void rb_lcached_insert(rbtree_lcached *root, rbnode *node,
                       int (*cmp)(const void *left, const void *right)) {

    // the very first node inserted is the minimum, for now
    if (RB_NULL_ROOT(&root->tree)) {
        rb_first_cached(root) = node;
    }

    // subsequent inserts are cached as the min if they're smaller than the known min
    if (cmp((void *) node, (void *) rb_first_cached(root)) < 0) {
        rb_first_cached(root) = node;
    }

    rb_insert(&root->tree, node, cmp);
}

void rb_rcached_insert(rbtree_rcached *root, rbnode *node,
                       int (*cmp)(const void *left, const void *right)) {

    // the very first node inserted is the minimum, for now
    if (RB_NULL_ROOT(&root->tree)) {
        rb_last_cached(root) = node;
    }

    // subsequent inserts are cached as the max if they're larger than the known max
    if (cmp((void *) node, (void *) rb_last_cached(root)) >= 0) {
        rb_last_cached(root) = node;
    }

    rb_insert(&root->tree, node, cmp);
}

void rb_lrcached_insert(rbtree_lrcached *root, rbnode *node,
                       int (*cmp)(const void *left, const void *right)) {

    // first element inserted is both the largest and smallest
    if (RB_NULL_ROOT(&root->tree)) {
        rb_first_cached(root) = node;
        rb_last_cached(root) = node;
    }

    // subsequent inserts are cached as the min if they're smaller than the known min
    if (cmp((void *) node, (void *) rb_first_cached(root)) < 0) {
        rb_first_cached(root) = node;
    }

    // subsequent inserts are cached as the max if they're larger than the known max
    if (cmp((void *) node, (void *) rb_last_cached(root)) >= 0) {
        rb_last_cached(root) = node;
    }

    rb_insert(&root->tree, node, cmp);
}

/**
 *	Binary search to find 'key'. Returns NULL if not found.
 */

static inline const rbnode *__rb_find(const rbnode *anchor,
                                       const void *key, int (*cmp)(const void *left, const void *right)) {

    rbnode *cursor = (rbnode *) anchor;
	
    int comparison;
    rbnode *next;
    while (cursor != NULL) {	
		comparison = cmp((void *) key, (void *) cursor);
		if (comparison < 0) {
			next = rb_left(cursor);
		} else if (comparison == 0) {
			break;
		} else {
			next = rb_right(cursor);
		}
		
		cursor = next;
	}

    return cursor;
}

const rbnode *rb_find(const rbtree *root,
                       const void *key, int (*cmp)(const void *left, const void *right)) {
    return __rb_find(rb_root(root), key, cmp);
}

/**
 *	Traversal to the logical min / max of the tree.
 */

static inline const rbnode *__rb_first(const rbnode *anchor) {

    rbnode *cursor = (rbnode *) anchor;
    if (cursor == NULL) return NULL;

    while (rb_left(cursor) != NULL) {
        cursor = rb_left(cursor);
    }

    return cursor;
}

static inline const rbnode *__rb_last(const rbnode *anchor) {

    rbnode *cursor = (rbnode *) anchor;
    if (cursor == NULL) return NULL;

    while (rb_right(cursor) != NULL) {
        cursor = rb_right(cursor);
    }

    return cursor;
}

const rbnode *rb_first(const rbtree *root) {
    if (!root) return NULL;
    return __rb_first(root->node);
}

const rbnode *rb_last(const rbtree *root) {
    if (!root) return NULL;
    return __rb_last(root->node);
}

/**
 *	Iterative tree traversal in sorted / backwards order.
 */

const rbnode *rb_next(const rbnode *node) {
    if (!node) return NULL;
    if (RB_EMPTY_NODE(node)) return NULL;

	// if there is a right subtree to traverse, then go as far right as possible down there
    if (rb_right(node)) {
        return __rb_first(rb_right(node));
    }

	// else move up until we find the first instance that we're the left subtree of something
    rbnode *cursor, *cursor_parent;

    cursor = (rbnode *) node;
	cursor_parent = rb_parent(cursor);
   	while (cursor_parent != NULL && cursor == rb_right(cursor_parent)) {
		cursor = cursor_parent;
		cursor_parent = rb_parent(cursor);
    }

    return cursor_parent;
}

const rbnode *rb_prev(const rbnode *node) {
    if (!node) return NULL;
    if (RB_EMPTY_NODE(node)) return NULL;

	// if there is a left subtree to traverse, gotta go as far right as possible on that side
    if (rb_left(node)) {
        return __rb_last(rb_left(node));
    }

	// else move up until we are on the right side of something
    rbnode *cursor, *cursor_parent;
        
	cursor = (rbnode *) node;
	cursor_parent = rb_parent(cursor);
    while (cursor_parent != NULL && cursor == rb_left(cursor_parent)) {
		cursor = cursor_parent;
		cursor_parent = rb_parent(cursor);
    }

    return cursor_parent;
}

/**
 * Basic BST deletion algorithm components - no balancing yet!
 */

static inline const rbnode *__rb_node_successor(const rbnode *target) {
	rbnode *successor;

	if (rb_left(target) && rb_right(target)) {
		successor = (rbnode *) rb_next(target);
	} else if (!rb_right(target)) {
		successor = rb_left(target);
	} else if (!rb_left(target)) {
		successor = rb_right(target);
	} else {
		successor = NULL;
	}

    return successor;
}

/**
 * Deletes a node and relinks subtrees around it.
 */

static inline void __rb_delete_node(rbnode *target) {
	rbnode *child;

	if (rb_left(target)) child = rb_left(target);
	else if (rb_right(target)) child = rb_right(target);
	else child = NULL;

    __rb_replace_child(rb_parent(target), target, child);
    __rb_node_clear(target);
}

/**
 * Basic copy-and-remove algorithm used by BST basic delete.
 */

static inline void __rb_move_and_delete(rbnode *src, rbnode *dst,
                                        void (*copy)(const void *src, void *dst)) {

	if (src) {
		copy((void *) src, (void *) dst);
    	__rb_delete_node(src);
	} else {
		__rb_delete_node(dst);
	}
}

static inline void __rb_delete_basic(rbnode *successor, rbnode *target,
										void (*copy)(const void *src, void *dst)) {
	__rb_move_and_delete(successor, target, copy);
}

/**
 * RB tree rebalancer.
 */

static inline void __rb_delete_rebalance(rbnode *node) {

    rbnode *parent, *sibling;
    rbnode *sibling_lchild, *sibling_rchild;

    for (;;) {

        // if we hit the root we're done
        parent = rb_parent(node);
        if (parent == NULL) {
			__rb_set_black(node);
            break;
        }

		// deleting red node doesn't do anything bad
		if (rb_is_red(node)) {
			__rb_set_black(node);
			break;
		}

        // otherwise black height property is in violation and we'll need the sibling, etc.
        sibling = __rb_sibling(node);

        // if we have a red nearby, let's try to dump the double black into that nearby red... that'll prep us for the next case
        if (rb_is_red(sibling)) {
			__rb_set_black(sibling); // pull the red up
			__rb_set_red(parent);

			if (sibling == rb_right(parent)) __rb_left_rotate(parent);  // reconfigure the tree for recoloring
			else __rb_right_rotate(parent);
			
			sibling = __rb_sibling(node);
        }

        // new sibling is ALWAYS black, but we might be able to dump the black somewhere else
        sibling_lchild = sibling ? rb_left(sibling) : NULL;
        sibling_rchild = sibling ? rb_right(sibling) : NULL;

        // if the nephew / niece can't take the black recolor, try to propagate it up
        if (rb_is_black(sibling_lchild) && rb_is_black(sibling_rchild)) {
	    	__rb_set_red(sibling);
			node = parent;
			continue;
        }

        // otherwise, we can do some rotations to find a red to drop the double-black into.
        if (sibling == rb_right(parent)) {

            // if the only red available is on the left side of the sibling, pull the red up and adjust black height
            if (rb_is_black(sibling_rchild)) {
				__rb_set_black(sibling_lchild);
				__rb_set_red(sibling);

                __rb_right_rotate(sibling);		// rl
     
				sibling = __rb_sibling(node);
				sibling_lchild = sibling ? rb_left(sibling) : NULL;
        		sibling_rchild = sibling ? rb_right(sibling) : NULL;
            }

            // terminal case pulls the red through and out of the system
			__rb_set_color(sibling, rb_color(parent));
            __rb_set_black(parent);
            __rb_set_black(sibling_rchild);
            __rb_left_rotate(parent);

            break;
        } else {

            // if we're in the LR case specifically (the ONLY red child is on the right)
            if (rb_is_black(sibling_lchild)) {
				__rb_set_black(sibling_rchild); // pull the red up
				__rb_set_red(sibling);

                __rb_left_rotate(sibling);

				sibling = __rb_sibling(node);
				sibling_lchild = sibling ? rb_left(sibling) : NULL;
        		sibling_rchild = sibling ? rb_right(sibling) : NULL;
            }

            __rb_set_color(sibling, rb_color(parent));
            __rb_set_black(parent);
            __rb_set_black(sibling_lchild);
            __rb_right_rotate(parent);
			
            break;
        }
	}
}

/**
 * Deletes a node from the tree. Requires a comparator to find the element and a copy function to exchange elements.
 */

void rb_delete(rbtree *tree, rbnode *node,
               int (*cmp)(const void *left, const void *right),
               void (*copy)(const void *src, void *dst)) {

    if (!tree) return;
    if (!node) return;
    if (!cmp) return;
    if (!copy) return;

    rbnode *target, *successor, *cursor;

    target = (rbnode *) rb_find(tree, node, cmp);       // try to find the node to even delete, if it exists
    if (!target) return;

    successor = (rbnode *) __rb_node_successor(target);
	if (successor) __rb_delete_rebalance(successor);	
	else __rb_delete_rebalance(target);
	
	// follow the tree up to retrace root if it changed
    cursor = target;
	while (rb_parent(cursor) != NULL) {
        cursor = rb_parent(cursor);
    }

	// actually delete the node now
	__rb_delete_basic(successor, target, copy);

	// post-deletion check to see if tree has been cleared
    if (RB_EMPTY_NODE(rb_root(tree))) {                             
        rb_root(tree) = NULL;
    } else {
		rb_root(tree) = cursor;
	}
}

void rb_lcached_delete(rbtree_lcached *root, rbnode *node,
                       int (*cmp)(const void *left, const void *right),
                       void (*copy)(const void *src, void *dst)) {

    // deleting the min makes the new min the next element in sorted order
    uint8_t min_changed = 0;
    if (cmp((void *) node, (void *) rb_first_cached(root)) == 0) {
        min_changed = 1;
    }

    rb_delete(&root->tree, node, cmp, copy);

    // if the tree was emptied, we don't have a min
    if (RB_NULL_ROOT(&(root->tree))) {
        rb_first_cached(root) = NULL;
    } else if (min_changed) {
        rb_first_cached(root) = (rbnode *) rb_first(&(root->tree));
    }
}

void rb_rcached_delete(rbtree_rcached *root, rbnode *node,
                       int (*cmp)(const void *left, const void *right),
                       void (*copy)(const void *src, void *dst)) {

    // deleting the max makes the new max the previous element in sorted order
    uint8_t max_changed = 0;
    if (cmp((void *) node, (void *) rb_last_cached(root)) == 0) {
        max_changed = 1;
    }

    rb_delete(&root->tree, node, cmp, copy);

    // if the tree was emptied, we don't have a max
    if (RB_NULL_ROOT(&root->tree)) {
        rb_last_cached(root) = NULL;
    } else if (max_changed) {
        rb_last_cached(root) = (rbnode *) rb_last(&(root->tree));
    }
}

void rb_lrcached_delete(rbtree_lrcached *root, rbnode *node,
                       int (*cmp)(const void *left, const void *right),
                       void (*copy)(const void *src, void *dst)) {

    // deleting the min makes the new min the next element in sorted order
    uint8_t min_changed = 0;
    if (cmp((void *) node, (void *) rb_first_cached(root)) == 0) {
        min_changed = 1;
    }

    // deleting the max makes the new max the previous element in sorted order
    uint8_t max_changed = 0;
    if (cmp((void *) node, (void *) rb_last_cached(root)) == 0) {
        max_changed = 1;
    }

    rb_delete(&root->tree, node, cmp, copy);

    // deleting the whole tree deletes the max and min
    if (RB_NULL_ROOT(&root->tree)) {
        rb_first_cached(root) = NULL;
        rb_last_cached(root) = NULL;
    } else if (min_changed) {
        rb_first_cached(root) = (rbnode *) rb_first(&(root->tree));
    } else if (max_changed) {
        rb_last_cached(root) = (rbnode *) rb_last(&(root->tree));
    }
}

void rbtree_clean(rbtree *tree) {
	rb_inorder_foreach(tree, __rb_delete_node);
}

void rb_lcached_clean(rbtree_lcached *tree) {
	rbtree_clean(&tree->tree);
	rb_first_cached(tree) = NULL;
}

void rb_rcached_clean(rbtree_rcached *tree) {
	rbtree_clean(&tree->tree);
	rb_last_cached(tree) = NULL;
}

void rb_lrcached_clean(rbtree_lrcached *tree) {
	rbtree_clean(&tree->tree);
	rb_first_cached(tree) = NULL;
	rb_last_cached(tree) = NULL;
}

/**
 * Tree traversal in all 3 'styles'. Invokes 'cb' on each node.
 */

static inline void __rb_inorder_foreach(rbnode *anchor, void (*cb)(void *key)) {
    if (!anchor) return;

    __rb_inorder_foreach(anchor->left, cb);
    cb(anchor);
    __rb_inorder_foreach(anchor->right, cb);
}

static inline void __rb_postorder_foreach(rbnode *anchor, void (*cb)(void *key)) {
    if (!anchor) return;

    __rb_postorder_foreach(anchor->left, cb);
    __rb_postorder_foreach(anchor->right, cb);
    cb(anchor);
}

static inline void __rb_preorder_foreach(rbnode *anchor, void (*cb)(void *key)) {
    if (!anchor) return;

    cb(anchor);
    __rb_preorder_foreach(anchor->left, cb);
    __rb_preorder_foreach(anchor->right, cb);
}

void rb_inorder_foreach(rbtree *tree, void (*cb)(void *key)) {
    __rb_inorder_foreach(rb_root(tree), cb);
}

void rb_postorder_foreach(rbtree *tree, void (*cb)(void *key)) {
    __rb_postorder_foreach(rb_root(tree), cb);
}

void rb_preorder_foreach(rbtree *tree, void (*cb)(void *key)) {
    __rb_preorder_foreach(rb_root(tree), cb);
}
