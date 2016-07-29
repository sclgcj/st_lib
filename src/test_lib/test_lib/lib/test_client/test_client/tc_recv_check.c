#include "tc_comm.h"
#include "tc_err.h"
#include "tc_init.h"
#include "tc_hash.h"
#include "tc_timer.h"
#include "tc_print.h"
#include "tc_create_private.h"
#include "tc_recv_check_private.h"
#include "tc_timer_list_private.h"

struct tc_recv_check_traversal_data {
	int count;
	int timeout;
	struct timespec ts;
	struct tc_create_link_data *epoll_data;
};

static int
tc_recv_check_traversal(
	unsigned long	  user_data,
	struct hlist_node *hnode
)
{
	struct timespec ts1;
	struct tc_link_timeout_node *lt_node = NULL;
	struct tc_create_link_data *epoll_data = NULL;
	struct tc_recv_check_traversal_data *traversal_data = NULL;

	traversal_data = (struct tc_recv_check_traversal_data *)user_data;
	traversal_data->count++;
	epoll_data = traversal_data->epoll_data;
	lt_node = tc_list_entry(hnode, struct tc_link_timeout_node, node);
	ts1 = lt_node->send_time;

	PRINT("timeout2 = %ld, %d\n", 
			traversal_data->ts.tv_sec - ts1.tv_sec, 
			traversal_data->timeout);
	if (traversal_data->ts.tv_sec - ts1.tv_sec >= traversal_data->timeout && 
			(epoll_data->epoll_oper && epoll_data->epoll_oper->err_handle)) {
		pthread_mutex_lock(&epoll_data->data_mutex);
		epoll_data->epoll_oper->err_handle(
				TC_TIMEOUT, 
				epoll_data->user_data,
				&epoll_data->link_data);
		pthread_mutex_unlock(&epoll_data->data_mutex);
	}

	return TC_OK;
}

static int
tc_recv_check_handle(
	unsigned long data
) 
{
	int ret = TC_OK;
	struct tc_recv_check_traversal_data traversal_data;

	if (!data)
		return TC_OK;

	PRINT("\n");
	memset(&traversal_data, 0, sizeof(traversal_data));
	traversal_data.epoll_data = (struct tc_create_link_data *)data;
//	PRINT("epoll_data->timeout_data.check_flag = %d, %p\n", epoll_data->timeout_data.check_flag, epoll_data);
	/*pthread_mutex_lock(&traversal_data.epoll_data->data_mutex);
	if (traversal_data.epoll_data->timeout_data.check_flag == 0) {
		pthread_mutex_unlock(&traversal_data.epoll_data->data_mutex);
		return TC_OK;
	}
	pthread_mutex_unlock(&traversal_data.epoll_data->data_mutex);*/
	
	clock_gettime(CLOCK_MONOTONIC, &traversal_data.ts);
	pthread_mutex_lock(&traversal_data.epoll_data->data_mutex);
	if (traversal_data.epoll_data->link_data.status != TC_STATUS_CONNECT) 
		traversal_data.timeout = traversal_data.epoll_data->timeout_data.recv_timeout;
	else
		traversal_data.timeout = traversal_data.epoll_data->timeout_data.conn_timeout;
//	PRINT("timerout = %d\n", timeout);
	//ts1 = epoll_data->timeout_data.send_time;
	pthread_mutex_unlock(&traversal_data.epoll_data->data_mutex);

	if (!traversal_data.epoll_data->epoll_oper || 
			!traversal_data.epoll_data->epoll_oper->err_handle)
		PRINT("null oepr\n");

//	PRINT("888888888888--------------%p\n", traversal_data.epoll_data);
	TC_HASH_WALK_DEL(
			traversal_data.epoll_data->timeout_data.timeout_hash,
			(unsigned long)&traversal_data, 
			tc_recv_check_traversal);

	/*
	 * When a packet in this link is timeout, we will call the user defined function to 
	 * do some error handle. User should decide if this timeout is really wrong and if 
	 * they want to delete this link when this happens(set link_data.err_flag to non-zero to
	 * tell the downstream to delete this link). Server and Client's operation may different,
	 * setting err_flag to 1 means just remove the socket from epoll, setting to 2 means 
	 * free the whole data
	 */
	//PRINT("err_flag = %d\n", traversal_data.epoll_data->link_data.err_flag);
	if (traversal_data.epoll_data->link_data.err_flag == 1) {
		tc_epoll_data_del(traversal_data.epoll_data->link_data.sock);
		ret = TC_ERR;
	} else if (traversal_data.epoll_data->link_data.err_flag == 2){
		PRINT("\n");
		tc_create_link_data_del(traversal_data.epoll_data);
		ret = TC_ERR;
	} else if (traversal_data.count == 0)
		ret = TC_ERR;

	return ret;
}

struct tc_recv_check_handle *
tc_recv_check_create(
	int recv_timeout
)
{
	int ret = 0;
	struct tc_recv_check_handle *handle = NULL;

	handle = (struct tc_recv_check_handle*)calloc(1, sizeof(*handle));
	if (!handle) {
		TC_ERRNO_SET(TC_NOT_ENOUGH_MEMORY);
		return NULL;
	}
	
	handle->list_handle = tc_timer_list_start(
						recv_timeout,
						TC_TIMER_FLAG_CONSTANT,
						tc_recv_check_handle);
	if (!handle->list_handle) {
		TC_FREE(handle);
		return NULL;
	}

	return handle;
}

void
tc_recv_check_destroy(
	struct tc_recv_check_handle *handle	
)
{
	return;		
}

int
tc_recv_check_start(
	char *name,
	struct tc_link_data *link_data
)
{
	int len = 0, ret = 0;
	struct tc_link_timeout_node *rc_node = NULL;
	struct tc_create_link_data *epoll_data = NULL;

	epoll_data = (struct tc_create_link_data *)link_data->parent;

	rc_node = (struct tc_link_timeout_node*)calloc(1, sizeof(*rc_node));
	if (!rc_node) {
		TC_ERRNO_SET(TC_NOT_ENOUGH_MEMORY);
		return TC_ERR;
	}
	if (name) {
		len = strlen(name);
		rc_node->name = (char*)calloc(1, len + 1);
		memcpy(rc_node->name, name, len);
	}
	clock_gettime(CLOCK_MONOTONIC, &rc_node->send_time);
	PRINT("1-----------%p\n", epoll_data);
	return tc_hash_add(
			epoll_data->timeout_data.timeout_hash, 
			&rc_node->node, 
			(unsigned long)name);

	/*pthread_mutex_lock(&epoll_data->data_mutex);
	epoll_data->timeout_data.check_flag = 1;
	//clock_gettime(CLOCK_MONOTONIC, &epoll_data->timeout_data.send_time);
	pthread_mutex_unlock(&epoll_data->data_mutex);*/
}

int
tc_recv_check_stop(
	char *name,
	struct tc_link_data *link_data
)
{
	struct hlist_node *hnode = NULL;
	struct tc_create_link_data *epoll_data = NULL;

	epoll_data = (struct tc_create_link_data *)link_data->parent;
	
	hnode = tc_hash_get(
			epoll_data->timeout_data.timeout_hash, 
			(unsigned long)name, 
			(unsigned long)name);
	if (!hnode)
		return TC_OK;

	return tc_hash_del(
			epoll_data->timeout_data.timeout_hash, 
			hnode, 
			(unsigned long)name);

	/*pthread_mutex_lock(&epoll_data->data_mutex);
	epoll_data->timeout_data.check_flag = 0;
	pthread_mutex_unlock(&epoll_data->data_mutex);*/
}

int
tc_recv_check_add(
	struct tc_recv_check_handle	*handle,
	struct tc_create_link_data	*epoll_data
)
{
	if (!handle) {
		TC_ERRNO_SET(TC_PARAM_ERROR);
		return TC_ERR;
	}
	return tc_timer_list_add(
				handle->list_handle,
				(unsigned long)epoll_data,
				&epoll_data->timer_data
				);
}