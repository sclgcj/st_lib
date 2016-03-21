#ifndef SERVER_TEST_LISTEN_H
#define SERVER_TEST_LISTEN_H 1

typedef void * STListenHandle;
typedef void (*STEpollinFunc)(void *pEpollData);
typedef void (*STEpollOutFunc)(void *pEpollData);
typedef void (*STEpollErrFunc)(void *pEpollData);
typedef void (*STEpollHupFunc)(void *pEpollData);
typedef void (*STEpollPriFunc)(void *pEpollData);
typedef void (*STEpollRDHupFunc)(void *pEpollData);

typedef struct _STListenerOp
{
	STEpollinFunc	   pEpollInFunc;				//读操作
	STEpollOutFunc   pEpollOutFunc;				//写操作
	STEpollErrFunc   pEpollErrFunc;				//错误操作
	STEpollHupFunc   pEpollHupFunc;				//挂起操作
	STEpollPriFunc   pEpollPriFunc;				//外带数据操作
	STEpollRDHupFunc pEpollRDHupFunc;			//对端关闭套接字操作
}STListenOp, *PSTListenOp;

// 监听事件个数
#define SERVER_TEST_MAX_EVENT 8192

/*
 * 初始化监听器
 */
/*int
server_test_init_listener( int iWaitTime, STDispatchFunc pFunc );
*/

/*
 * 反初始化监听器
 */
/*int 
server_test_uninit_listener();
*/

/*
 *	启动监听器
 */
int
server_test_start_listener( STListenHandle struHandle );

/*
 * 添加监听套接字
 */
int
server_test_add_listen_sockfd( int iEvent, int iSockfd, void *pData, STListenHandle struHandle );

/*
 *	删除监听套接字
 */
int
server_test_del_listen_sockfd( int iSockfd, STListenHandle struHandle );

int
server_test_mod_listen_sockfd(
	int iEvent,
	int iSockfd,
	void *pData,
	STListenHandle struHandle
);

int 
server_test_create_listener(
	int iWaitTime, 
	int iListenNum, 
	STListenOp *pStruListenerOper,
	STListenHandle *pStruHandle
);

#endif
