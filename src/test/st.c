#include "st_comm.h"
#include "st_manage.h"
#include "st_listen.h"

static char *gsArgFmt = "f:d";

int main(
	int  iArgc,
	char **ppArgv
)
{
	int iRet = 0;
	STHandle struHandle;	
	STListenOp struListenOper;

	iRet = st_set_listen_op( &struListenOper );
	if( iRet != ST_OK )
	{
		return iRet;
	}

	st_create_manager(10, &struHandle);

	iRet = st_manager_create_listener(
																				1000,
																				0,
																				&struListenOper,
																				struHandle
																			);
	if( iRet != ST_OK )
	{
		return iRet;
	}

	iRet = st_manager_create_thread(0, struHandle);
	if( iRet != ST_OK )
	{
		return iRet;
	}

	iRet = st_manager_create_timer(0, 0, struHandle);
	if( iRet != ST_OK )
	{
		return iRet;
	}

	ST_ERROR("\n");
	return st_manage_start_listener(struHandle);
}

