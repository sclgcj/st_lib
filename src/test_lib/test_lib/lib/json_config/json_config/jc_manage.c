#include "json_config_private.h"
#include "json_config_manage.h"
#include <sys/stat.h>

struct json_config_manage {
	struct list_head jc_list;
};

struct json_config_manage_node {
	char *name;
	cJSON *root;
	struct list_head node;
};

static struct json_config_manage global_jc_manage = {
	.jc_list = LIST_HEAD_INIT(global_jc_manage.jc_list)
};

static cJSON *
json_config_manage_data_get(
	char *file_path
)
{
	int ret = 0;
	char *data = NULL;
	FILE *fp = NULL;
	cJSON *root = NULL;
	struct stat sbuf;

	memset(&sbuf, 0, sizeof(sbuf));
	if (stat(file_path, &sbuf)) {
		fprintf(stderr, "get file %s stat error: %s\n", 
				file_path, strerror(errno));
		return NULL;
	}

	if (sbuf.st_size == 0) {
		fprintf(stderr, "file %s is empty\n", file_path);
	}

	data = (char *)calloc(sizeof(char), sbuf.st_size);
	if (!data) {
		fprintf(stderr, "can't calloc %d bytes : %s", 
				sbuf.st_size, strerror(errno));
		exit(0);
	}

	fp = fopen(file_path, "r");
	if (!fp) {
		fprintf(stderr, "open file %s error: %s\n",
				file_path, strerror(errno));
		return NULL;
	}

	fread(data, sizeof(char), sbuf.st_size, fp);
	fclose(fp);

	root = cJSON_Parse(data);
	if (!root) 
		fprintf(stderr, "data in file %s, is not correct json string\n", file_path);

	return root;
}

int
json_config_manage_create(
	char *name,
	char *file_path
)
{
	cJSON *root = NULL;
	struct json_config_manage_node *jcmn =NULL;

	jcmn = (struct json_config_manage_node*)calloc(1, sizeof(*jcmn));
	if (!jcmn) {
		fprintf(stderr, "can't calloc %d bytes : %s\n", 
				sizeof(*jcmn), strerror(errno));
		exit(0);
	}

	root = json_config_manage_data_get(file_path);
	if (!root) 
		return JC_ERR;

	jcmn->root = json_config_param_init(0, 0, root);
	if(!jcmn->root) {
		free(jcmn);
		return JC_ERR;
	}

	list_add_tail(&jcmn->node, &global_jc_manage.jc_list);

	return JC_OK;
		

}

static void
json_config_manage_node_destroy(
	struct json_config_manage_node *jcmn
)
{
	if (jcmn->name)
		free(jcmn->name);
	if (jcmn->root)
		cJSON_Delete(jcmn->root);
}

int
json_config_manage_destroy()
{
	struct list_head *sl = NULL;
	struct json_config_manage_node *jcmn = NULL;

	sl = global_jc_manage.jc_list.next;
	while (sl != &global_jc_manage.jc_list) {
		jcmn = list_entry(sl, struct json_config_manage_node, node);
		sl = sl->next;
		list_del_init(&jcmn->node);
		json_config_manage_node_destroy(jcmn);
		free(jcmn);
	}

	return JC_OK;
}

static struct json_config_manage_node *
json_config_manage_node_get(
	char *name
)
{
	struct json_config_manage_node *jcmn = NULL;

	list_for_each_entry(jcmn, &global_jc_manage.jc_list, node) {
		if (!strcmp(name, jcmn->name))
			return jcmn;
	}

	return NULL;
}

int
json_config_manage_special_destroy(
	char *name
)
{
	struct json_config_manage_node *jcmn = NULL;

	jcmn = json_config_manage_node_get(name);
	if (!jcmn)
		return JC_ERR;

	list_del_init(&jcmn->node);
	json_config_manage_node_destroy(jcmn);
	free(jcmn);

	return JC_OK;
}

char *
json_config_manage_param_get(
	int  id,
	unsigned long user_data,
	char *name
)
{
	struct json_config_manage_node *jcmn = NULL;	

	jcmn = json_config_manage_node_get(name);
	if (!jcmn) {
		fprintf(stderr, "no such name %s\n", name);
		return NULL;
	}	
	
	return json_config_to_param(id, user_data, jcmn->root);
}
