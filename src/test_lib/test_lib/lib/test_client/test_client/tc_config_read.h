#ifndef TC_CONFIG_READ_H
#define TC_CONFIG_READ_H

#include "cJSON.h"
/*
 * tc_config_read_oper is a set of operations provided for upstream to to some 
 * extra actions defined by upstream. These function will not affect the default
 * actions.
 */
struct tc_config_read_oper {
	/*
	 * config_read_root() - dispose toml root
	 *
	 * We will call it when we face with the root of the toml configure file.
	 */
	void (*config_read_root)();
	/*
	 * config_read_list() - dispose toml list
	 */
	void (*config_read_list)(char *name);
	/*
	 * config_read_table - dispose toml table
	 */
	void (*config_read_table)(char *name);
	/*
	 * config_read_table_array - dispose toml table array
	 */
	void (*config_read_table_array)(char *name);
};

void
tc_config_read_oper_set(
	struct tc_config_read_oper *oper
);

cJSON *
tc_config_read_get(
	char *table_name
);

void
tc_config_read_del(
	char *table_name
);

int
tc_config_read_handle();

#define CR_INT(json, name, data) do{ \
	cJSON *tmp = cJSON_GetObjectItem(json, name); \
	if (tmp && tmp->valuestring)  \
		data = atoi(tmp->valuestring); \
}while(0)

#define CR_STR(json, name, data) do{ \
	cJSON *tmp = cJSON_GetObjectItem(json, name); \
	if (tmp && tmp->valuestring) \
		memcpy(data, tmp->valuestring, strlen(tmp->valuestring)); \
}while(0)

#define CR_SHORT(json, name, data) do{ \
	cJSON *tmp = cJSON_GetObjectItem(json, name); \
	if (tmp && tmp->valuestring) \
		data = (short)atoi(tmp->valuestring); \
}while(0)

#define CR_USHORT(json, name, data) do{ \
	cJSON *tmp = cJSON_GetObjectItem(json, name); \
	if (tmp && tmp->valuestring) \
		data = (unsigned long)atoi(tmp->valuestring); \
}while(0)

#define CR_IP(json, name, data) do{ \
	cJSON *tmp = cJSON_GetObjectItem(json, name); \
	if (tmp && tmp->valuestring) \
		data = inet_addr(tmp->valuestring); \
}while(0)

#define CR_DURATION(json, name, data) do{ \
	cJSON *tmp = cJSON_GetObjectItem(json, name); \
	int day = 0, hour = 0, min = 0, sec = 0; \
	if (tmp && tmp->valuestring) { \
		sscanf(tmp->valuestring, "%d:%d:%d:%d", &day, &hour, &min, &sec);\
		if (hour >= 24 || min >= 60 || sec >= 60) \
			TC_PANIC("Wrong duration :%s\n", tmp); \
		data = day * 3600 * 24 + hour * 3600 + min * 60 + sec; \
	} \
}while(0);

#define CR_DEV(json, name, data) do{ \
	cJSON *tmp = cJSON_GetObjectItem(json, name); \
	if (tmp && tmp->valuestring) { \
		if (!strcmp(tmp->valuestring, "server")) \
			data = TC_DEV_SERVER; \
		else if (!strcmp(tmp->valuestring, "client")) \
			data = TC_DEV_CLIENT; \
	} \
}while(0)

#define CR_SIZE(json, name, data) do{ \
	int size = 0; \
	char *ch = NULL; \
	cJSON *tmp = cJSON_GetObjectItem(json, name); \
	if (tmp && tmp->valuestring) { \
		size = atoi(tmp->valuestring); \
		ch = strchr(tmp->valuestring, 'K'); \
		if (ch)  \
			size *= 1024; \
		else if ((ch = strchr(tmp->valuestring,'M'))) \
			size *= (1024 * 1024); \
		data = size; \
	} \
}while(0)

#endif
