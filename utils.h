/*filename: utils.h*/

#ifndef _UTILS_H_
#define _UTILS_H_

#include <zlib.h>


void daemon_init(void);

int get_executable_path (char* buffer, size_t len);

int check_only_one_process(const char* pidfilename);

int create_dma_dev(const char* dev_name, int major, int minor);

u_int32_t calc_hashnr(const char *key, u_int32_t length);



/****************************************
* about string
****************************************/
char* ipaddr2str(u_int32_t ipaddr, char* straddr);

int str2bytesH(const char* str, unsigned char* data, unsigned int data_len);

int str2bytesH1(const char* str, unsigned char* data, unsigned int data_len);

int bytes2strH(const unsigned char* data, int data_len, char* str);

int isxdigitstr(const char* str);

int isdigitstr(const char* str);

int is_str_ip(const char *sip);

char *str2lower(char *str);

char *str2upper(char *str);

int strcmp_nocase(const char *s1, const char *s2);

char *trim_domain_string(char *domain);


/****************************************
* about hash
****************************************/
int url_to_hash(const char* url_info_hash, unsigned char* info_hash);

int str_to_hash(const char* url_info_hash, unsigned char* info_hash);

unsigned long get_str_hash(const char *c);

void hash2string(const char *hash, char *string);

void string2hash(char *hash, const char *string);

void string2hash_ex(char *hash, const char *string, int hashLen);

void hash2string_ex(const char *hash, char *string, int hashLen);


/****************************************
* about dir path
****************************************/
bool PathExist(const char *name);

int  IsDir (const char * name) ;

int CreatePath(const char *path);



/****************************************
* about compress
****************************************/
int deflate_compress(Bytef *data, uint ndata, Bytef *zdata, uint *nzdata);
int gzip_compress(char *dst, uint dstlen, const char *src, uint srclen);



/****************************************
* about socket
****************************************/
int ipv6_cmp(const unsigned char *ip1, const unsigned char *ip2);

double ipv6_precise_cmp(const unsigned char *ip1, const unsigned char *ip2);

unsigned long long ntohll(unsigned long long val);

unsigned long long htonll(unsigned long long val);

int create_udp_socket(u_int32_t listenip, u_int16_t listenport, int nonblock);

int init_raw_socket(const char *ifname);


/****************************************
* about debug
****************************************/
void dump_debug_data(const char *data, int data_len, const char *file_name);


#endif


