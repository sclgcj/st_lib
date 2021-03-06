#include "tc_hash.h"
#include "tc_err.h"
#include "tc_print.h"
#include "tc_global_log_private.h"

/**
 * Luckly, the design of this hash list module is thread-safety. we 
 * add a mutex lock on every hash list. What? the table? it is inited
 * at start and uninited at the end when we are sure that there is nothing
 * to use it and we're sure at the begining.
 *
 * This is a static hash table and there is some limitations on it. We hope 
 * to use the dynamical hash table to replace it...
 *
 */

struct tc_hash_head{
	pthread_mutex_t hlist_mutex;
	struct hlist_head head;
};

struct tc_hash_table{
	int table_size;
	struct tc_hash_head *tc_head;
	int (*hash_func)(struct hlist_node *node, unsigned long user_data);
	int (*hash_get)(struct hlist_node *node, unsigned long user_data);
	int (*hash_destroy)(struct hlist_node *node);
};

tc_hash_handle_t
tc_hash_create(
	int table_size,
	int (*hash_func)(struct hlist_node *node, unsigned long user_data),
	int (*hash_get)(struct hlist_node *node, unsigned long user_data),
	int (*hash_destroy)(struct hlist_node *node)
)
{
	int i = 0;
	struct tc_hash_table *hash_table = NULL;;

	if (!table_size || !hash_get) {
		TC_ERRNO_SET(TC_PARAM_ERROR);
		return TC_HASH_ERR;
	}

	hash_table = (struct tc_hash_table *)calloc(sizeof(*hash_table), 1);
	if (!hash_table) {
		TC_ERRNO_SET(TC_NOT_ENOUGH_MEMORY);
		return TC_HASH_ERR;	
	}
	hash_table->hash_func = hash_func;
	hash_table->hash_get  = hash_get;
	hash_table->hash_destroy = hash_destroy;
	hash_table->table_size = table_size;
	hash_table->tc_head = (struct tc_hash_head*)
				calloc(sizeof(struct tc_hash_head), table_size);
	if (!hash_table->tc_head) {
		TC_ERRNO_SET(TC_NOT_ENOUGH_MEMORY);
		free(hash_table);
		return TC_HASH_ERR;
	}
	for (; i < table_size; i++) {
		INIT_HLIST_HEAD(&hash_table->tc_head[i].head);
		pthread_mutex_init(&hash_table->tc_head[i].hlist_mutex, NULL);
	}

	return (tc_hash_handle_t)hash_table;
}

int
tc_hash_destroy(
	tc_hash_handle_t handle	
)
{
	int i = 0;
	int ret = 0;
	struct hlist_node *hnode = NULL, *hsave = NULL;
	struct tc_hash_table *hash_table = NULL;

	hash_table = (struct tc_hash_table *)handle;
	if (!hash_table || (void*)handle == TC_HASH_ERR) {
		TC_ERRNO_SET(TC_PARAM_ERROR);
		return -1;
	}

	for (; i < hash_table->table_size; i++) {
		pthread_mutex_lock(&hash_table->tc_head[i].hlist_mutex);
		if (hlist_empty(&hash_table->tc_head[i].head)) {
			pthread_mutex_unlock(&hash_table->tc_head[i].hlist_mutex);
			continue;
		}
		hnode = hash_table->tc_head[i].head.first;
		while(hnode) {
			hsave = hnode->next;
			hlist_del_init(hnode);
			if (hash_table->hash_destroy) {
				ret = hash_table->hash_destroy(hnode);
				if (ret != TC_OK) {
					return ret;
				}
			}
			hnode = hsave;
		}
		pthread_mutex_unlock(&hash_table->tc_head[i].hlist_mutex);
	}

	return TC_OK;
}

static int
tc_hlist_safe_check(
	int pos,
	struct tc_hash_table *hash_table
)
{
		return TC_ERR;		
}

int
tc_hash_add(
	tc_hash_handle_t handle,
	struct hlist_node *node,
	unsigned long user_data
)
{
	int pos = 0, ret = 0;
	struct hlist_node *hnode = NULL;
	struct tc_hash_table *hash_table = NULL;

	hash_table = (struct tc_hash_table*)handle;
	if (!hash_table || (void*)handle == TC_HASH_ERR) {
		TC_ERRNO_SET(TC_PARAM_ERROR);
		return TC_ERR;
	}

	if (hash_table->hash_func)
		pos = hash_table->hash_func(node, user_data);
	else
		pos = 0;
	if (pos < 0 || pos >= hash_table->table_size) {
		TC_ERRNO_SET(TC_PARAM_ERROR);
		return TC_ERR;
	}

	pthread_mutex_lock(&hash_table->tc_head[pos].hlist_mutex);
	hlist_add_head(node, &hash_table->tc_head[pos].head);
	pthread_mutex_unlock(&hash_table->tc_head[pos].hlist_mutex);

	return TC_OK;
}

int
tc_hash_traversal(
	unsigned long user_data,
	tc_hash_handle_t handle,
	int (*hash_walk_handle)(unsigned long user_data, struct hlist_node *hnode, int *flag)
)
{
	int ret = 0;
	int pos = 0;
	int del_flag = 0;
	struct hlist_node *hnode = NULL, *next = NULL, **hsave = NULL;
	struct tc_hash_table *hash_table = NULL; 

	hash_table = (struct tc_hash_table*)handle;

	for (; pos < hash_table->table_size; pos++) {
			
		pthread_mutex_lock(&hash_table->tc_head[pos].hlist_mutex);
		if (hlist_empty(&hash_table->tc_head[pos].head)) {
			pthread_mutex_unlock(&hash_table->tc_head[pos].hlist_mutex);
			continue;
		}
		hlist_for_each_safe(hnode, next, &hash_table->tc_head[pos].head) {	
			if (hash_walk_handle) {
				ret = hash_walk_handle(user_data, hnode, &del_flag);
				if (ret != TC_OK) {
					pthread_mutex_unlock(
						&hash_table->tc_head[pos].hlist_mutex);
					return ret;
				}
			}
			if (del_flag) {
				hsave = hnode->pprev;
				hlist_del_init(hnode);
				hash_table->hash_destroy(hnode);
				hnode = *hsave;
			}
		}
		pthread_mutex_unlock(&hash_table->tc_head[pos].hlist_mutex);
		pthread_mutex_destroy(&hash_table->tc_head[pos].hlist_mutex);
	}
	TC_FREE(hash_table->tc_head);

	return TC_OK;
}

int
tc_hash_del(
	tc_hash_handle_t handle,
	struct hlist_node *node,
	unsigned long user_data
)
{
	int pos = 0;	
	struct hlist_node *hnode = NULL;
	struct tc_hash_table *hash_table = NULL;

	hash_table = (struct tc_hash_table *)handle;
	if (!hash_table || (void*)handle == TC_HASH_ERR) {
		TC_ERRNO_SET(TC_PARAM_ERROR);
		return TC_ERR;
	}

	if (hash_table->hash_func) 
		pos = hash_table->hash_func(node, user_data);
	else
		pos = 0;
	if (pos < 0 || pos >= hash_table->table_size) {
		TC_ERRNO_SET(TC_PARAM_ERROR);
		return TC_ERR;
	}
	pthread_mutex_lock(&hash_table->tc_head[pos].hlist_mutex);
	hlist_del_init(node);
	pthread_mutex_unlock(&hash_table->tc_head[pos].hlist_mutex);

	return TC_OK;
}

int
tc_hash_del_and_destroy(
	tc_hash_handle_t handle,
	struct hlist_node *node,
	unsigned long user_data
)
{
	int pos = 0;	
	struct hlist_node *hnode = NULL;
	struct tc_hash_table *hash_table = NULL;

	hash_table = (struct tc_hash_table *)handle;
	if (!hash_table || (void*)handle == TC_HASH_ERR) {
		TC_ERRNO_SET(TC_PARAM_ERROR);
		return TC_ERR;
	}

	if (hash_table->hash_func) 
		pos = hash_table->hash_func(node, user_data);
	else
		pos = 0;
	if (pos < 0 || pos >= hash_table->table_size) {
		TC_ERRNO_SET(TC_PARAM_ERROR);
		return TC_ERR;
	}
	pthread_mutex_lock(&hash_table->tc_head[pos].hlist_mutex);
	hlist_del_init(node);
	hash_table->hash_destroy(node);
	pthread_mutex_unlock(&hash_table->tc_head[pos].hlist_mutex);

	return TC_OK;
}

struct hlist_node *
tc_hash_get(
	tc_hash_handle_t handle,
	unsigned long hash_data,
	unsigned long search_cmp_data
)
{
	int pos = 0, ret = 0;	
	struct hlist_node *hnode = NULL, *safe = NULL;
	struct tc_hash_table *hash_table = NULL;

	hash_table = (struct tc_hash_table *)handle;
	if (!hash_table || (void*)handle == TC_HASH_ERR) {
		TC_ERRNO_SET(TC_PARAM_ERROR);
		return NULL;
	}

	if (hash_table->hash_func) 
		pos = hash_table->hash_func(hnode, hash_data);
	else
		pos = 0;
	if (pos < 0 || pos >= hash_table->table_size) {
		TC_ERRNO_SET(TC_PARAM_ERROR);
		return NULL;
	}

	pthread_mutex_lock(&hash_table->tc_head[pos].hlist_mutex);
	if (hlist_empty(&hash_table->tc_head[pos].head)) {
		pthread_mutex_unlock(&hash_table->tc_head[pos].hlist_mutex);
		return NULL;
	}
	hlist_for_each_safe(hnode, safe, &hash_table->tc_head[pos].head) {
		ret = hash_table->hash_get(hnode, search_cmp_data);
		if (ret == TC_OK) {
			break;
		}
	}
	pthread_mutex_unlock(&hash_table->tc_head[pos].hlist_mutex);

	return hnode;
}

int
tc_hash_head_traversal(
	tc_hash_handle_t  handle,		
	unsigned long     hash_data,
	unsigned long	  user_data,
	void (*traversal)(struct hlist_node *hnode, unsigned long user_data)
)
{
	int pos = 0, ret = 0;	
	struct hlist_node *hnode = NULL, *safe = NULL;
	struct tc_hash_table *hash_table = NULL;

	hash_table = (struct tc_hash_table *)handle;
	if (!hash_table || (void*)handle == TC_HASH_ERR) {
		TC_ERRNO_SET(TC_PARAM_ERROR);
		return TC_ERR;
	}

	if (hash_table->hash_func) {
		pos = hash_table->hash_func(hnode, hash_data);
	}
	else
		pos = 0;
	if (pos < 0 || pos >= hash_table->table_size) {
		TC_ERRNO_SET(TC_PARAM_ERROR);
		return TC_ERR;
	}

	pthread_mutex_lock(&hash_table->tc_head[pos].hlist_mutex);
	if (hlist_empty(&hash_table->tc_head[pos].head)) {
		//TC_GINFO("emtpy--- at pos = %d\n", pos);
		pthread_mutex_unlock(&hash_table->tc_head[pos].hlist_mutex);
		return TC_ERR;
	}
	hlist_for_each_safe(hnode, safe, &hash_table->tc_head[pos].head) {
		if (traversal)
			traversal(hnode, user_data);
	}
	pthread_mutex_unlock(&hash_table->tc_head[pos].hlist_mutex);

	return TC_OK;
}

