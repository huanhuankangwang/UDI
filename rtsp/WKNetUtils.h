#ifndef _WK_NET_UTILS_H_
#define _WK_NET_UTILS_H_

#include <utils.h>

bool is_ipaddr(short int sin_family, const char *ip_addr);
bool isspace(char ch);
char* convert_url(const char *to_convert);
size_t strcount(const char *string, const char *vals);
U16 getRandomPort();
#endif//_WK_NET_UTILS_H_