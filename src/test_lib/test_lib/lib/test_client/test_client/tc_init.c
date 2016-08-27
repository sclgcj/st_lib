#include "tc_comm.h"
#include "tc_init.h"
#include "tc_print.h"
#include "tc_thread.h"
#include "tc_err.h"

/*
 * Normally, upstream don't  need to care about 
 * how downstream init its modules, what they 
 * need to understand is how to init their own
 * code. All apps can use TC_MOD_INIT to declare
 * its data before main start, but please note,
 * if there is some init operation needing the ones
 * which depend on the the downsteam, it'd better
 * use tc_init_register function in its init function
 * to register it. Because the archecture will call 
 * these function after its own modules initialization.
 */

struct tc_init_conf {
	int (*func)();
	struct list_head node;
};

struct tc_init_data {
	int init_flag;
	int uninit_flag;
	pthread_mutex_t mutex;
};

static struct tc_init_data global_init_data;
static struct list_head global_init_list = LIST_HEAD_INIT(global_init_list);
static struct list_head global_uninit_list = LIST_HEAD_INIT(global_uninit_list);

/*
 * we hope ervery module use this to register its init function to initialize 
 * their own data
 */
static int 
tc_register_handle(
	int (*func)(),
	struct list_head *head
)
{
	struct tc_init_conf *init_conf = NULL;

	init_conf = (struct tc_init_conf*)calloc(1, sizeof(*init_conf));
	if (!init_conf) {
		TC_ERRNO_SET(TC_NOT_ENOUGH_MEMORY);
		return TC_ERR;
	}

	init_conf->func = func;
	list_add_tail(&init_conf->node, head);

	return TC_OK;
}

int
tc_init_register(
	int (*init)()
)
{
	return tc_register_handle(init, &global_init_list);
}


int
tc_uninit_register(
	int (*uninit)()
)
{
	return tc_register_handle(uninit, &global_uninit_list);
}

static int
tc_init_handle(
	struct list_head *head
)
{
	int ret = TC_OK;
	int count = 0;
	struct tc_init_conf *init_conf = NULL;

	list_for_each_entry(init_conf, (head), node) {
		PRINT("count = %d\n", count++);
		if (init_conf->func) {
			ret = init_conf->func();
			if (ret != TC_OK) 
				break;
		}
	}

	return ret;
}

int 
tc_init_test()
{
	int ret = TC_ERR;

	pthread_mutex_lock(&global_init_data.mutex);
	if (global_init_data.init_flag)
		ret = TC_OK;
	pthread_mutex_unlock(&global_init_data.mutex);
	
	return ret;	
}

int
tc_uninit()
{
	pthread_mutex_lock(&global_init_data.mutex);
	if (global_init_data.uninit_flag == 1) {
		goto out;
	}
	global_init_data.uninit_flag = 1;
	tc_thread_exit_wait();
	tc_init_handle(&global_uninit_list);
	pthread_mutex_unlock(&global_init_data.mutex);
out:
	return TC_OK;
}

int
tc_init()
{
	pthread_mutex_init(&global_init_data.mutex, NULL);
	tc_init_handle(&global_init_list);
	pthread_mutex_lock(&global_init_data.mutex);
	global_init_data.init_flag = 1;	
	pthread_mutex_unlock(&global_init_data.mutex);
}

