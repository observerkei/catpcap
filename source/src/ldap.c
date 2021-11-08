#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <arpa/inet.h>


#include "ldapexpr/ldapexpr.h"
#include "session.h"

#define ldap_debug(fmt, ...) do {\
		if (g_ldap_debug_enable) printf("%s %s (%d) "fmt,__FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);\
	} while (0)

char g_ldap_debug_enable = 1;

int ldap_cmp_src_ip(ldapexpr_ftv_t *ftv, void *data)
{
	if (!ftv || !ftv->value || !data)
		return -2;

	session_t *sess = (session_t *)data;	
	uint32_t src_ip = ntohl(sess->src_ip.addr_ip);
	addr_ip_t ip = {0}; /* 主机序 */
	if (4 != sscanf(ftv->value, "%hhu.%hhu.%hhu.%hhu", &ip.ipv4_all[3], 
			&ip.ipv4_all[2], &ip.ipv4_all[1], &ip.ipv4_all[0])) {
		return -2;
	}
	//ldap_debug("ip:%u, %u.%u.%u.%u, sip：%u, %u.%u.%u.%u\n", ip.addr_ip, NIPQUAD(ip.addr_ip), src_ip, NIPQUAD(src_ip));
		
	switch (ftv->type) {			
		case FT_EQ:
			return !(src_ip == ip.addr_ip);
		case FT_NE:
			return !(src_ip != ip.addr_ip);
		case FT_LT:
			return !(src_ip < ip.addr_ip);
		case FT_GT:
			return !(src_ip > ip.addr_ip);
		case FT_LTE:
			return !(src_ip <= ip.addr_ip);
		case FT_GTE:
			return !(src_ip >= ip.addr_ip);
		default:
			return -1;
	}

	return 1;
}

int ldap_cmp_dst_ip(ldapexpr_ftv_t *ftv, void *data)
{
	if (!ftv || !ftv->value || !data)
		return -2;

	session_t *sess = (session_t *)data;	
	uint32_t dst_ip = ntohl(sess->dst_ip.addr_ip);
	addr_ip_t ip = {0}; /* 主机序 */
	if (4 != sscanf(ftv->value, "%hhu.%hhu.%hhu.%hhu", &ip.ipv4_all[3], 
			&ip.ipv4_all[2], &ip.ipv4_all[1], &ip.ipv4_all[0])) {
		return -2;
	}
	//ldap_debug("ip:%u, %u.%u.%u.%u, dip：%u, %u.%u.%u.%u\n", ip.addr_ip, NIPQUAD(ip.addr_ip), dst_ip, NIPQUAD(dst_ip));
		
	switch (ftv->type) {			
		case FT_EQ:
			return !(dst_ip == ip.addr_ip);
		case FT_NE:
			return !(dst_ip != ip.addr_ip);
		case FT_LT:
			return !(dst_ip < ip.addr_ip);
		case FT_GT:
			return !(dst_ip > ip.addr_ip);
		case FT_LTE:
			return !(dst_ip <= ip.addr_ip);
		case FT_GTE:
			return !(dst_ip >= ip.addr_ip);
		default:
			return -1;
	}

	return 1;

}

int ldap_cmp_ip(ldapexpr_ftv_t *ftv, void *data)
{
	if (!ftv || !ftv->value || !data)
		return -2;

	int ret_src_ip = ldap_cmp_src_ip(ftv, data);
	int ret_dst_ip = ldap_cmp_dst_ip(ftv, data);

	if (!ret_src_ip || !ret_dst_ip)
		return 0;
	if (ret_src_ip < 0 || ret_dst_ip < 0)
		return -1;
	return 1;
}

int ldap_cmp_src_port(ldapexpr_ftv_t *ftv, void *data)
{
	if (!ftv || !ftv->value || !data)
		return -1;

	session_t *sess = (session_t *)data;
	uint16_t src_port = ntohs(sess->src_port);
	uint16_t l = strtol(ftv->value, NULL, 0);
	
	switch (ftv->type) {			
		case FT_EQ:
			return !(src_port == l);
		case FT_NE:
			return !(src_port != l);
		case FT_LT:
			return !(src_port < l);
		case FT_GT:
			return !(src_port > l);
		case FT_LTE:
			return !(src_port <= l);
		case FT_GTE:
			return !(src_port >= l);
		default:
			return -1;
	}

	return 1;
}

int ldap_cmp_dst_port(ldapexpr_ftv_t *ftv, void *data)
{
	if (!ftv || !ftv->value || !data)
		return -1;

	session_t *sess = (session_t *)data;
	uint16_t l = strtol(ftv->value, NULL, 0);
	uint16_t dst_port = ntohs(sess->dst_port);

	switch (ftv->type) {			
		case FT_EQ:
			return !(dst_port == l);
		case FT_NE:
			return !(dst_port != l);
		case FT_LT:
			return !(dst_port < l);
		case FT_GT:
			return !(dst_port > l);
		case FT_LTE:
			return !(dst_port <= l);
		case FT_GTE:
			return !(dst_port >= l);
		default:
			return -1;
	}

	return 1;
}

int ldap_cmp_port(ldapexpr_ftv_t *ftv, void *data)
{
	if (!ftv || !ftv->value || !data)
		return -2;

	int ret_src_port = ldap_cmp_src_port(ftv, data);
	int ret_dst_port = ldap_cmp_dst_port(ftv, data);

	if (!ret_src_port || !ret_dst_port)
		return 0;
	if (ret_src_port < 0 || ret_dst_port < 0)
		return -1;
	return 1;
}


int ldap_cmp_transport(ldapexpr_ftv_t *ftv, void *data)
{
	if (!ftv || !ftv->value || !data)
		return -1;

	session_t *sess = (session_t *)data;
	
	switch (ftv->type) {			
		case FT_EQ:
			switch (sess->transport) {
				case SESSION_TRANSPORT_TCP:
					if (!strcmp("TCP", ftv->value)) {
						ldap_debug("catch transport:%s\n", ftv->value);
						return 0;
					}
					break;
				case SESSION_TRANSPORT_UDP:
					if (!strcmp("UDP", ftv->value)) {
						ldap_debug("catch transport:%s\n", ftv->value);
						return 0;
					}				
					break;
				case SESSION_TRANSPORT_ICMP: 
					if (!strcmp("ICMP", ftv->value)) {
						ldap_debug("catch transport:%s\n", ftv->value);
						return 0;
					}
					break;
				default:
					ldap_debug(" transport: UNKNOW\n");
					return -1;
			}			
			break;
		default:
			return -1;
	}

	return 1;
}

ldapexpr_hook_kv_t s_catpcap_cmp_hook[] = {
	{"ip", ldap_cmp_ip},
	{"src_ip", ldap_cmp_src_ip},
	{"dst_ip", ldap_cmp_dst_ip},
	{"port", ldap_cmp_port},
	{"src_port", ldap_cmp_src_port},
	{"dst_port", ldap_cmp_dst_port},
	{"transport", ldap_cmp_transport},
};

int ldap_init(void) {
	int ret = 0;
	int i = 0;
	for (i = 0; i < sizeof(s_catpcap_cmp_hook)/sizeof(s_catpcap_cmp_hook[0]); ++i) {
		ret = add_ldapexpr_cmp(&s_catpcap_cmp_hook[i]);
		if (ret)
			return -1;
	}

	return 0;
}
