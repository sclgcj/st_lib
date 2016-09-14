#ifndef TC_INTERFACE_H
#define TC_INTERFACE_H 1

#include "tc_comm.h"

/*
 * 2016-9-9 record:
 *	This archeture is not good. I have used it yestoday, but the 
 * effect is bad. I think it's a little hard to use when we decide to 
 * bind struct tc_interface_op to every api. However, if we use our 
 * default json function, we can give a common structure to every api.
 * We need more test to check if this is suitable.
 */

enum {
	TC_INTERFACE_PARAM_NORMAL,
	TC_INTERFACE_PARAM_JSON,
	TC_INTERFACE_PARAM_MAX
};

struct tc_interface_oper {
	/*
	 * interface_param() - set the interface parameters
	 * api_name:	the api's name
	 * user_data:	upstream data
	 * param:	pointer to store the parameters. It's a double pointer of char(char**).
	 *		*param will point to the real address. Just look like this:
	 *			char **tmp = (char**)param;
	 *
	 * Please store the final send string in the param, and allocate new space for param
	 *
	 * Return: 0 if success, -1 if not
	 */
	int  (*interface_param)(char *api_name, unsigned long user_data, unsigned long *param);
	/*
	 * curlopt_set() - set the curl options.
	 * curl:	a curl handle pointer
	 * user_data:   upstream user data
	 *
	 * We will set some default curl option, but this may not satisfy
	 * other situations. Thus, we provide this callback for upstreams 
	 * to set their own curl options. default curl opt below:
	 *			CURLOPT_POST		-----> 1
 	 *			CURLOPT_SSL_VERIFYPEER	-----> 0
 	 *			CURLOPT_SSL_VERIFYHOST  -----> 0
 	 *			CURLOPT_WRITEDATA	-----> write_callback data
 	 *			CURLOPT_WRITEFUNCTION   -----> write_callback function
 	 *			CURLOPT_POSTFIELDSIZE	-----> param_size
 	 *			CURLOPT_POSTFIELDS	-----> param
 	 *			CURLOPT_URL		-----> url
 	 *			CURLOPT_NOSIGNAL	-----> 1
 	 *			CURLOPT_TIMEOUT		-----> recv_timeout
 	 *			CURLOPT_CONNECTTIMEOUT  -----> connect_timeout
 	 *			CURLOPT_SSL_SESSIONID_CACHE -> 0
 	 *			CURLOPT_CAINFO		-----> ""
 	 *			CURLOPT_CAPATH		-----> ""
	 *
	 * Return: 0 if successful, -1 if not
	 */
	int (*curlopt_set)(char *api_name, CURL *curl, unsigned long user_data);
	/*
	 * before_send() - let user do something before sending http packet
	 * user_data:	upstream user data
	 *
	 * Before sending a packet, upstream may have something to do, so 
	 * we provide this callback. 
	 *
	 * Return: 0 if successful, -1 if not
	 */
	int (*before_send)(char *api_name, unsigned long user_data);
	/*
	 * first_recv() - do something when first receiving peer data
	 * user_data:	upstream user data
	 *
	 * Sometimes we need do something when we first receiving the peer
	 * data, this may different from the interface_recv. Of course, we
	 * also can realize it in interface_recv callback. The relationship 
	 * of first_recv and interface_recv in realizing is below:
	 *	first_recv(user_data);
	 *	interface_recv(user_data);
	 * It doesn't do any reception operation. Please realize all reception
	 * operation in interface_recv callback
	 *
	 * Return: 0 if successful, -1 if not
	 */
	int (*first_recv)(char *api_name, unsigned long user_data);
	/*
	 * interface_recv() - recv http data
	 * ptr:		http data received from curl
	 * size:	the size of each char
	 * nmemb:	the num of char
	 * user_data:	upstream data
	 *
	 * This callback need to store ptr for later interface_check. we seperate
	 * interface_check and interface_recv for sometimes we need seperate 
	 * reception and disposition to prevent that the disposition will cost
	 * too much time. 
	 *
	 * Return: 0 if successful, -1 if not
	 */
	int (*interface_recv)(char *api_name, 
			      char *ptr, 
			      size_t size, 
			      size_t nmemb, 
			      unsigned long user_data);
	/*
	 * interface_check() - check http data
	 * api_name:    this api interface name
	 * user_data:	upstream data
	 *
	 * Check the http data
	 *
	 * Return: 0 if successful, -1 if not
	 */
	int (*interface_check)(char *api_name, unsigned long user_data);
};

/*
 * tc_interface_register() - register an api interface
 * @group_name:		the group of the api belonging to. For example, if there is a url 
 *			http://ip:port/hello/api_name, we call 'hello' as the group_name.
 *			It can be any length.
 * @api_name:		the interface's name
 * @oper:		the operation function of the api, to see struct tc_interface_oper	
 *			for details
 *
 * We just split the interface process into few parts. 
 * 1、 interface param set
 * 2、 curl opt set
 * 3、 before send disposition
 * 7、 sending http packet
 * 4、 first reception disposition
 * 5、 data reception
 * 6、 data check
 *
 * we try our best to abstract the interface test.
 *
 * Return: 0 if successful, -1 if not and errno will be set
 */
int
tc_interface_register(
	char *group_name,
	char *api_name,
	struct tc_interface_oper *oper
);

/*
 * tc_interface_url_encode() - use url code to encode the data
 * @data:	raw data that need url encode
 * 
 * Return: the string pointer if success, null if something wrong
 */
char*
tc_interface_url_encode(
	char *data
);

#endif
