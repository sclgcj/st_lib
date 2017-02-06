#include "json_config_mode_private.h"
#include "json_config_mode_same_as_val_private.h"

#define JC_MODE_SAME_AS_VAL  "same as"

struct jc_mode_same_as_val {
	char *module;		
};

static struct jc_mode_same_as_val global_same_val;

static int
jc_mode_same_as_val_init(
	struct json_config_comm *jcc
)
{
	char *tmp = NULL;
	struct json_mode_private *jmp;

	jmp = (struct json_mode_private *)jcc->module_private;

	if (global_same_val.module)
		free(global_same_val.module);

	tmp = strrchr(jmp->obj->valuestring, ' ');
	tmp++;
	global_same_val.module = strdup(tmp);

	return JC_OK;
}

static int
jc_mode_same_as_val_execute(
	struct json_config_comm *jcc
)
{
	struct json_mode_private *jmp = NULL;

	jmp = (struct json_mode_private*)jcc->module_private;

	if (jmp->other_mode_judge)
		return jmp->other_mode_judge(global_same_val.module, jmp->data, jcc);

	return JC_OK;
}

int
json_config_mode_same_as_val_init()
{
	struct json_mode_oper oper;

	memset(&oper, 0, sizeof(oper));
	oper.json_mode_init = jc_mode_same_as_val_init;
	oper.json_mode_execute = jc_mode_same_as_val_execute;

	return json_mode_module_add(JC_MODE_SAME_AS_VAL, &oper);
}

int
json_config_mode_same_as_val_uninit()
{
	if (global_same_val.module)
		free(global_same_val.module);
		
	return JC_OK;
}