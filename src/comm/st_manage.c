#include "st_exit.h"
#include "st_manage.h"

void
st_create_manager(
	unsigned int uiDurationTime,
	STHandle     *pStruHandle
)
{
	ServerTest *pStruST = NULL;

	st_init_exit();

	ST_CALLOC(pStruST, ServerTest, 1);
	
	pStruST->uiDurationTime = uiDurationTime;

	(*pStruHandle) = (STHandle)pStruST;
}

int
st_manager_create_listener(
	int iWaitTime,
	int iListenNum,
	STListenOp *pStruListeOp,
	STHandle   struHandle
)
{
	ServerTest *pStruST = (ServerTest *)struHandle;

	if( !pStruST )
	{
		return ST_PARAM_ERR;
	}
	
	return st_create_listener( 
																iWaitTime,
																iListenNum, 
																pStruListeOp, 
																&pStruST->struListenHandle
															);
}

int
st_manager_create_thread(
	int iThreadGroupNum,
	STHandle struHandle
)
{
	ServerTest *pStruST = (ServerTest *)struHandle;

	if( !pStruST )
	{
		return ST_PARAM_ERR;
	}

	return st_create_thread_table(
																		iThreadGroupNum,
																		&pStruST->struThreadHandle
																	);
}

int
st_manager_create_timer(
	int iTimerNum,
	int iThreadNum,
	STHandle struHandle
)
{
	ServerTest *pStruST = (ServerTest *)struHandle;

	if( !pStruST )
	{
		return ST_PARAM_ERR;
	}
	if( !pStruST->struThreadHandle )
	{
		ST_ERROR("sfsdfs\n");
	}

	return st_create_timer(
															iTimerNum,
															iThreadNum,
															pStruST->struThreadHandle,
															&pStruST->struTimerHandle
														);
}

int
st_manage_create_hub(
	int iHubNum,
	int iThreadNum,
	int iStackSize,
	STHubFunc pHubFunc,
	STHubHandleFunc pHubHandleFunc,
	STHandle        struHandle
)
{
	ServerTest *pStruST = (ServerTest*)struHandle;

	if( !struHandle )
	{
		return ST_PARAM_ERR;
	}

	return st_create_hub(
									iHubNum,
									iThreadNum,
									iStackSize,
									pHubFunc,
									pHubHandleFunc,
									pStruST->struTimerHandle,
									pStruST->struThreadHandle,
									&pStruST->struHubHandle
								);
}

int
st_manager_create_opt_config(
	int  iArgc,
	char **ssArgv,
	char *sParseFmt,
	STHandle struHandle
)
{
	ServerTest *pStruST = (ServerTest *)struHandle;

	if( !struHandle )
	{
		return ST_PARAM_ERR;
	}

	st_create_opt_config(iArgc, ssArgv, sParseFmt, &pStruST->struOptHandle);
}

int
st_manager_create_read_config(
	char *sFile,
	STHandle struHandle
)
{
	ServerTest *pStruST = (ServerTest *)struHandle;

	if( !struHandle )
	{
		return ST_PARAM_ERR;
	}

	return st_create_read_config(sFile, &pStruST->struRCHandle);
}

int
st_manager_create_all(
	int iWaitTime,
	int iListenNum,			
	unsigned int uiDurationTime,
	int iThreadNum,
	int	iThreadGroupNum, 
	int iTimerNum,
	STListenOp *pStruListenOp,
	STHandle *pStruHandle
)
{
	int iRet = 0;
	ServerTest *pStruST = NULL;	

	ST_CALLOC(pStruST, ServerTest, 1);
	pStruST->uiDurationTime = uiDurationTime;
	
	iRet = st_create_listener(
												iWaitTime, 
												iListenNum, 
												pStruListenOp, 
												pStruST->struListenHandle
											);
	if( iRet != ST_OK )
	{
		return iRet;
	}

	iRet = st_create_thread_table(iThreadGroupNum, &pStruST->struThreadHandle);
	if( iRet != ST_OK )
	{
		return iRet;
	}

	iRet = st_create_timer(
														iThreadNum, 
														iTimerNum, 
														pStruST->struThreadHandle, 
														&pStruST->struTimerHandle 
													);
	if( iRet != ST_OK )
	{
		return iRet;
	}

	(*pStruHandle) = (STHandle)pStruST;

	return ST_OK;
}

int
st_destroy_manager(
	STHandle struHandle
)
{
	ServerTest *pStruST = (ServerTest *)struHandle;

	if( !struHandle )
	{
		return ST_OK;
	}

	st_destroy_listener(pStruST->struListenHandle);

	st_destroy_thread_table(pStruST->struThreadHandle);

	st_destroy_timer(pStruST->struTimerHandle);

	st_destroy_opt_config(pStruST->struOptHandle);

	st_destroy_read_config(pStruST->struRCHandle);

	st_destroy_hub(pStruST->struHubHandle);

	return ST_OK;
}


