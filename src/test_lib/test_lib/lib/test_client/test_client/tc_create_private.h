#ifndef TC_CREATE_PRIVATE_H
#define TC_CREATE_PRIVATE_H 1

#include "tc_hash.h"
#include "tc_epoll.h"
#include "tc_create.h"
#include "tc_transfer_proto_private.h"

struct tc_timeout_data {
	int check_flag;				//是否开启了检测，1 为开启， 0为未开启
	int conn_timeout;			//连接超时时间
	int recv_timeout;			//接收超时时间
//	struct timespec send_time;		//发送包的时间
	tc_hash_handle_t timeout_hash;		//超时hash表
	pthread_mutex_t mutex;
};

enum {
	TC_STATUS_CONNECT,
	TC_STATUS_SEND_DATA,
	TC_STATUS_LISTEN,
	TC_STATUS_DISCONNECT,
	TC_STATUS_MAX
};

struct tc_link_timeout_node {
	char *name;
	struct timespec send_time;
	struct hlist_node node;
};

struct tc_io_data {
	char *data;
	int  data_len;
	int  addr_type;
	struct sockaddr *addr;
	struct list_head node;
};

#define TC_DEFAULT_RECV_BUF  1024
struct tc_link_private_data {
	int		sock;			//套接字
	int		status;			//内部维护的状态，主要用于区分是否是创建连接
	int		link_id;		//用于标识每一个连接,其实是做hash的时候使用

	int		link_type;		//连接类型, client or server
					
	int		hub_interval;		//心跳间隔，由于有些服务器的心跳间隔会在运行过程中
						//发生变化，因此，提供此参数，当心跳变化时，需要对
						//其重新赋值
	int		port_num;		//用于标识在port_map中，该连接使用的是第几个端口
	int		err_flag;		//错误标记，由用户设置该标志，如果为1, 则只会将
						//该连接移除epoll，如果为2，则会将和该连接有关的
						//所有数据均移除。这么做的目的是因为服务器和客户端
						//对连接失败的处理是不同的。为0，则不作任何操作
						
	int		recv_cnt;
	char		*recv_data;
	
	struct list_head send_list;
	pthread_mutex_t mutex;
	pthread_mutex_t send_mutex;
};

struct tc_link_data {
	char		*unix_path;		//unix socket path
	struct in_addr	peer_addr;		//对端地址
	struct in_addr 	local_addr;		//本地地址
	unsigned short 	peer_port;		//对端端口
	unsigned short 	local_port;		//本地端口
};

struct tc_create_link_data {
	unsigned long		user_data;	//用户数据
	unsigned long		timer_data;     //超时节点链表对应的结构指针
	unsigned long		hub_data;	//心跳包数据, 当连接断开时，删除该数据
	struct tc_link_data	link_data;	//连接数据
	struct tc_link_private_data private_link_data; //连接私有数据
	struct tc_timeout_data	timeout_data;	//超时数据
	struct tc_create_link_oper *epoll_oper;	//epoll的操作
	struct tc_create_config  *config;	//连接配置
	struct tc_transfer_proto_oper *proto_oper;

	int			first_recv;

	pthread_cond_t		interface_cond;
	pthread_mutex_t		interface_mutex;
	pthread_mutex_t		recv_mutex;	//接收数据锁
	pthread_mutex_t		data_mutex;	//数据锁
	pthread_mutex_t		*hlist_mutex;	//hash链表对应的锁指针
	struct hlist_node	node;
	struct list_head	rc_node;	//超时检测节点
};

struct tc_create_data {
	int		transfer_flag;		//转发标志，目未实现
	char		*proto_name;		//协议名称:tcp_client, tcp_server, udp_client...
	int		port_num;		//port map number
	int		link_id;		//连接id
	struct in_addr	addr;			//本地ip地址
	struct in_addr  server_ip;		//服务器ip
	unsigned short	port;			//本地端口
	unsigned short  server_port;		//服务器端口
	unsigned long	user_data;		//用户数据
	struct tc_create_link_oper *oper;	//针对套接字的操作
	struct list_head node;
};

struct tc_transfer_link {
	int enable;
	int proto;
	int addr;
	int port;
	char unix_path[108];

};


/*
 * 关于每个连接的套接字选项的问题，由于涉及到的选项比较多，正在考虑如何实现，
 * 由于本意是想把整个套接字封装起来，让套接字对上层不可见，因此不想把套接字
 * 暴露给上层，目前考虑两种方案：一种是使用类似ioctl的方式，把套接字的各个
 * 选项自己封装一下，让上层通过程序进行设置，工作量比较大，但是使用比较麻烦；
 * 第二种是使用文件，把每个选项标注一下，通过对文件中选项赋值来达到这个效果，
 * 优点是修改选项不用修改程序，使用更加灵活，缺点却是无法对特殊的单个套接字
 * 进行设定，但是由于本身程序时处理高并发的，那么可以猜测套接字选项基本上是
 * 一致。但是不排除每个连接都会创建新套接字的情况（ftp）。
 */

struct tc_create_socket_option {
	int linger;
	int send_buf;
	int recv_buf;
};

struct tc_create_config {
	char		netcard[IFNAMSIZ];	//网卡名
	char		netmask[16];	//子网掩码
	int		duration;	//程序持续运行时间
	int		port_map;	//针对一个连接管理其他多个连接情况，
					//可以为其他链接预留足够的端口
	int		enable_transfer; //开启数据转发, 1 - 开启， 0 - 不开启
	int		total_link;	//总连接数
	int		ip_count;	//ip个数
	int		connect_timeout; //连接超时
	int		recv_timeout;	//接收超时
	int		add_check;	//添加超时检测
	int		link_type;	//设备类型,server or client
	int		proto;		//协议
	
	int		open_push;	//是否开启消息推送, 1 开启， 0 不开启(未实现)
	int		stack_size;	//线程栈大小
	int		hub_interval;	//心跳间隔
	int		hub_enable;	//是否开启心跳, 1 开启， 0 不开启
	int		rendevous_enable; //是否开启集合点
	unsigned int	server_ip;	//服务器ip
	unsigned int	start_ip;	//起始ip
	unsigned short  hub_num;	//心跳模块线程数
	unsigned short  link_num;	//连接创建模块线程数
	unsigned short  recv_num;	//接收数据模块线程数
	unsigned short  send_num;	//发送数据模块线程数
	unsigned short  timer_num;	//定时器线程数
	unsigned short	handle_num;	//处理数据模块线程数据
	unsigned short  end_port;	//结束端口
	unsigned short  start_port;	//起始端口
	unsigned short  server_port;	//服务器端口
	char		res[2];
	struct tc_create_socket_option option;
	struct tc_transfer_link	transfer_server;	//数据转发服务器配置
	struct tc_transfer_link	transfer_client;	//数据转发客户端配置
};

struct tc_create_link_data *
tc_create_link_data_alloc(
	int sock, 
	char *path,
	struct in_addr peer_addr,
	unsigned short peer_port,
	struct tc_create_data *create_data
);

int
tc_sock_event_add(
	int sock,
	int event,
	struct tc_create_link_data *epoll_data
);

/*
 * tc_link_create_start() - start the link create thread
 *
 * Return: 0 if successful, -1 if not
 */
int tc_link_create_start();

int
tc_create_link_data_traversal(
	unsigned long user_data,
	int (*traversal_func)(unsigned long user_data, struct hlist_node *hnode, int *flag)
);

int
tc_create_link_data_traversal_del(
	unsigned long user_data,
	int (*traversal_func)(unsigned long user_data, struct hlist_node *node, int *flag)
);

int
tc_create_link_data_del(
	struct tc_create_link_data  *data
);

int
tc_create_check_duration();

int
tc_create_link_err_handle(
	struct tc_create_link_data *cl_data 
);

int
tc_create_link_data_destroy(
	struct tc_create_link_data  *data
);

void
tc_create_user_data_get(
	int id,	
	unsigned long *cl_user_data
);

struct tc_create_data *
tc_create_data_calloc(
	char		*proto,
	int		transfer_flag,
	int		port_map_cnt,
	struct in_addr  ip,
	struct in_addr  server_ip,
	unsigned short  server_port,
	unsigned short	port,
	unsigned long   user_data
);

struct tc_create_link_data *
tc_create_link_data_get(
	unsigned long link_data
);

void
tc_create_link_create_data_destroy(
	struct list_head *list_node
);

#endif
