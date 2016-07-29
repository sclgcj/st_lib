#ifndef TC_HASH_H
#define TC_HASH_H

#include "tc_comm.h"

typedef void * tc_hash_handle_t;
#define TC_HASH_ERR  (void*)-1

/**
 * tc_hash_create() - used to create a hash_table for the caller
 * @tabel_size:		tell the handler the size of caller's hash table size
 * @hash_func:		function used to determine the pos of the hash head in
 *			the hash table, can be null
 * @hash_get:		used to get check if the current node is the one that 
 *			user really needed, this function can't no be null,
 * @hash_destroy:	the fucntion used when destroy the created hash table,
 *			can be null
 *
 * Normally, hash_func and hash_destroy can not be null, but here we will give 
 * them default functions. If hash_func is null, we will always add node to the
 * fisrt hlsit_head structure; if hash_destroy is null, we will do nothing but 
 * delete the node from the head and the left response should be taken by caller.
 *
 * Return: a tc_hash_handle_t structure wiill return if successful, otherwise 
 *	   return TC_HASH_ERR
 */
tc_hash_handle_t
tc_hash_create(
	int table_size,
	int (*hash_func)(struct hlist_node *node, unsigned long user_data),
	int (*hash_get)(struct hlist_node *node, unsigned long user_data),
	int (*hash_destroy)(struct hlist_node *node)
);

/**
 * tc_hash_destrooy() - used to destry the created hash tabel
 * @handle:		a tc_hash_handle_t structure returned from tc_hash_create
 *
 * Return: 0 if successful and -1 will be returned if something wrong
 */
int
tc_hash_destroy(
	tc_hash_handle_t handle
);

/**
 * tc_hash_add() - add a new node to the hash table
 * @handle:		a structure returned from tc_hash_create
 * @node:		the hash node address in the caller's structure
 * @user_data:		used in hash_func to determine which hash_head it should 
 *			be added in
 *
 * Return: 0 if successful and -1 will be returned if something wrong
 */
int
tc_hash_add(
	tc_hash_handle_t handle,
	struct hlist_node *node,
	unsigned long user_data
);

/**
 * tc_hash_del() - add a new node to the hash table
 * @handle:		a structure returned from tc_hash_create
 * @node:		the hash node address in the caller's structure
 * @user_data:		used in hash_func to determine which hash_head it should 
 *			be added in
 *
 * Return: 0 if successful and -1 will be returned if something wrong
 */
int
tc_hash_del(
	tc_hash_handle_t handle,
	struct hlist_node *node,
	unsigned long user_data
);

/**
 * tc_hash_get() - get an exist  node to the hash table
 * @handle:		a structure returned from tc_hash_create
 * @user_data:		used as parameter of hash_func
 *
 * Return: an hlist_node structure if find one and NULL will be returned if something wrong
 */
struct hlist_node * 
tc_hash_get(
	tc_hash_handle_t handle,
	unsigned long hash_data,
	unsigned long search_cmp_data
);
#define TC_HASH_GET(h, type, memb, index_data, search_data) \
	list_entry(tc_hash_get(h, type, memv), index_data, search_data)

/*
 * tc_hash_traversal - traversal the whole hash list
 * @del_flag:	a flag to determine if we need to delete each hash node, 0 need, 1 not
 * @user_data:	user data
 * @handle:	hash handle pointer created by tc_hash_create fucntion
 * @hash_walk_handle: function to dispose every hash node
 *	@@hnode: hash list node
 *
 * We hope to provide a function to simplify the operation of traversal every hash node.
 * We don't want to write this kind of code every time, so we provide this. We just need 
 * to give the node handle function which will be different at different scene.
 *
 * Return: no
 */
int
tc_hash_traversal(
	int del_flag,
	unsigned long    user_data,
	tc_hash_handle_t handle,
	int (*hash_walk_handle)(unsigned long user_data, struct hlist_node *hnode)
);
#define TC_HASH_WALK(handle, ud, walk_func) tc_hash_traversal(0, ud, handle, walk_func)
#define TC_HASH_WALK_DEL(handle, ud, walk_func) tc_hash_traversal(1, ud, handle, walk_func)

#endif