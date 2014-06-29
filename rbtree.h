/* Minimal red-black-tree helper functions
 *
 * Copyright (c) 2012-2014, Sven Eckelmann <sven@narfation.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef __RBTREE_H__
#define __RBTREE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#if defined(__GNUC__)
#define RBTREE_TYPEOF_USE 1
#endif

#if defined(_MSC_VER)
#define __inline__ __inline
#endif

/**
 * container_of() - Calculate address of object that contains address ptr
 * @ptr: pointer to member variable
 * @type: type of the structure containing ptr
 * @member: name of the member variable in struct @type
 *
 * Return: @type pointer of object containing ptr
 */
#ifndef container_of
#ifdef RBTREE_TYPEOF_USE
#define container_of(ptr, type, member) __extension__ ({ \
	const __typeof__(((type *)0)->member) *__pmember = (ptr); \
	(type *)((char *)__pmember - offsetof(type, member)); })
#else
#define container_of(ptr, type, member) \
	((type *)((char *)(ptr) - offsetof(type, member)))
#endif
#endif

/**
 * struct rb_node - node of a red-black tree
 * @parent: pointer to the parent node in the tree
 * @left: pointer to the left child in the tree
 * @right: pointer to the right child in the tree
 *
 * The red-black tree consists of a root and nodes attached to this root. The
 * rb_* functions and macros can be used to access and modify this data
 * structure.
 *
 * The @parent pointer of the rb node points to the parent rb node in the
 * tree, the @left to the left "smaller key" child and @right to the right
 * "larger key" node of the tree.
 *
 * The rb nodes are usually embedded in a container structure which holds the
 * actual data. Such an container object is called entry. The helper rb_entry
 * can be used to calculate the object address from the address of the node.
 */
struct rb_node {
	struct rb_node *parent;
	struct rb_node *left;
	struct rb_node *right;
};

/**
 * struct rb_root - root of a red-black-tree
 * @node: pointer to the root node in the tree
 *
 * For an empty tree, node points to NULL.
 */
struct rb_root {
	struct rb_node *node;
};

/**
 * DEFINE_RBROOT - define tree root and initialize it
 * @root: name of the new object
 */
#define DEFINE_RBROOT(root) \
	struct rb_root root = { NULL }

/**
 * INIT_RB_ROOT() - Initialize empty tree
 * @root: pointer to rb root
 */
static __inline__ void INIT_RB_ROOT(struct rb_root *root)
{
	root->node = NULL;
}

/**
 * rb_empty() - Check if tree has no nodes attached
 * @root: pointer to the root of the tree
 *
 * Return: 0 - tree is not empty !0 - tree is empty
 */
static __inline__ int rb_empty(const struct rb_root *root)
{
	return !root->node;
}

/**
 * rb_change_child() - Fix child entry of parent node
 * @old_node: rb node to replace
 * @new_node: rb node replacing @old_node
 * @parent: parent of @old_node
 * @root: pointer to rb root
 *
 * Detects if @old_node is left/right child of @parent or if it gets inserted
 * as as new root. These entries are then updated to point to @new_node.
 *
 * @old_node and @root must not be NULL.
 */
static __inline__ void rb_change_child(struct rb_node *old_node,
				       struct rb_node *new_node,
				       struct rb_node *parent,
				       struct rb_root *root)
{
	if (parent) {
		if (parent->left == old_node)
			parent->left = new_node;
		else
			parent->right = new_node;
	} else {
		root->node = new_node;
	}
}

/**
 * rb_link_node() - Add new node as new leaf
 * @node: pointer to the new node
 * @parent: pointer to the parent node
 * @rb_link: pointer to the left/right pointer of @parent
 *
 * @node will be initialized as leaf node of @parent. It will be linked to the
 * tree via the @rb_link pointer. @parent must be NULL and @rb_link has to point
 * to "node" of rb_root when the tree is empty.
 *
 * WARNING The new node may cause the tree to be become unbalanced or violate
 * any rules of the red black tree. A call to rb_insert_color after rb_link_node
 * is therefore always required to rebalance the tree correctly. rb_insert
 * can be used as helper to run both steps at the same time.
 */
static __inline__ void rb_link_node(struct rb_node *node,
				    struct rb_node *parent,
				    struct rb_node **rb_link)
{
	node->parent = parent;
	node->left = NULL;
	node->right = NULL;

	*rb_link = node;
}

/**
 * rb_erase_node() - Remove rb node from tree
 * @node: pointer to the node
 * @root: pointer to rb root
 *
 * The node is only removed from the tree. Neither the memory of the removed
 * node nor the memory of the entry containing the node is free'd. The node
 * has to be handled like an uninitialized node. Accessing the parent or
 * right/left pointer of the node is not safe.
 *
 * WARNING The removed node may cause the tree to be become unbalanced or
 * violate any rules of the red black tree. A call to rb_erase_color after
 * rb_erase_node is therefore always required to rebalance the tree correctly.
 * rb_erase can be used as helper to run both steps at the same time.
 */
static __inline__ void rb_erase_node(struct rb_node *node, struct rb_root *root)
{
	struct rb_node *smallest;

	/* no child */
	if (!node->left && !node->right) {
		rb_change_child(node, NULL, node->parent, root);
		return;
	}

	/* one child, left */
	if (node->left && !node->right) {
		node->left->parent = node->parent;
		rb_change_child(node, node->left, node->parent, root);
		return;
	}

	/* one child, right */
	if (!node->left && node->right) {
		node->right->parent = node->parent;
		rb_change_child(node, node->right, node->parent, root);
		return;
	}

	/* two children, take smallest of right (grand)children */
	smallest = node->right;
	while (smallest->left)
		smallest = smallest->left;

	/* move right child of smallest one up */
	if (smallest->right)
		smallest->right->parent = smallest->parent;
	rb_change_child(smallest, smallest->right, smallest->parent, root);

	/* exchange node with smallest */
	smallest->parent = node->parent;

	smallest->left = node->left;
	smallest->left->parent = smallest;

	smallest->right = node->right;
	if (smallest->right)
		smallest->right->parent = smallest;

	rb_change_child(node, smallest, node->parent, root);
}

/**
 * rb_first() - Find leftmost rb node in tree
 * @root: pointer to rb root
 *
 * Return: pointer to leftmost node. NULL when @root is empty.
 */
static __inline__ struct rb_node *rb_first(const struct rb_root *root)
{
	struct rb_node *node = root->node;

	if (!node)
		return node;

	/* descend down via smaller/preceding child */
	while (node->left)
		node = node->left;

	return node;
}

/**
 * rb_last() - Find rightmost rb node in tree
 * @root: pointer to rb root
 *
 * Return: pointer to rightmost node. NULL when @root is empty.
 */
static __inline__ struct rb_node *rb_last(const struct rb_root *root)
{
	struct rb_node *node = root->node;

	if (!node)
		return node;

	/* descend down via larger/succeeding child */
	while (node->right)
		node = node->right;

	return node;
}

/**
 * rb_next() - Find successor node in tree
 * @node: starting rb node for search
 *
 * Return: pointer to successor node. NULL when no successor of @node exist.
 */
static __inline__ struct rb_node *rb_next(struct rb_node *node)
{
	struct rb_node *parent;

	/* there is a right child - next node must be the leftmost under it */
	if (node->right) {
		node = node->right;
		while (node->left)
			node = node->left;

		return node;
	}

	/* otherwise check if we have a parent (and thus maybe siblings) */
	parent = node->parent;
	if (!parent)
		return parent;

	/* go up the tree until the path connecting both is the left child
	 * pointer and therefore the parent is the next node
	 */
	while (parent && parent->right == node) {
		node = parent;
		parent = node->parent;
	}

	return parent;
}

/**
 * rb_prev() - Find predecessor node in tree
 * @node: starting rb node for search
 *
 * Return: pointer to predecessor node. NULL when no predecessor of @node exist.
 */
static __inline__ struct rb_node *rb_prev(struct rb_node *node)
{
	struct rb_node *parent;

	/* there is a left child - prev node must be the rightmost under it */
	if (node->left) {
		node = node->left;
		while (node->right)
			node = node->right;

		return node;
	}

	/* otherwise check if we have a parent (and thus maybe siblings) */
	parent = node->parent;
	if (!parent)
		return parent;

	/* go up the tree until the path connecting both is the right child
	 * pointer and therefore the parent is the prev node
	 */
	while (parent && parent->left == node) {
		node = parent;
		parent = node->parent;
	}

	return parent;
}

/**
 * rb_entry() - Calculate address of entry that contains tree node
 * @node: pointer to tree node
 * @type: type of the entry containing the tree node
 * @member: name of the rb_node member variable in struct @type
 *
 * Return: @type pointer of entry containing node
 */
#define rb_entry(node, type, member) container_of(node, type, member)

#ifdef __cplusplus
}
#endif

#endif /* __RBTREE_H__ */