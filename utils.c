#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <unistd.h> 
#include <signal.h> 
#include <sys/param.h> 
#include <sys/types.h> 
#include <sys/stat.h>

#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include<linux/if_packet.h>

#include "utils.h"

#define MKDEV(ma,mi)	((ma)<<8 | (mi))


void daemon_init()
{
	int i;
	for (i = 0; i<NOFILE; ++i)
		close(i);

	switch (fork())
	{
		case -1:
		{
			perror("setup_daemon(), 1st fork()");
			exit(2);
		}
		default:
			exit(0);
		case 0:
			if (setsid()==-1)
			{ 
				perror("setup_daemon(), setsid()");
				exit(3);
			}
		switch (fork())
		{
			case -1:
			{
				perror("setup_daemon(), 2nd fork()");
				exit(3);
			}

		default:
			exit(0);
		case 0:
			umask(0);
			/* and return with daemon set up */
			return;
		}
	}

}


int get_executable_path (char* buffer, size_t len)
{
	char* path_end;
	
	/* Read the target of /proc/self/exe. */
	if (readlink ("/proc/self/exe", buffer, len) <= 0)
		return -1;
	
	/* Find the last occurrence of a forward slash, the path separator. */
	path_end = strrchr (buffer, '/');
	if (path_end == NULL)
		return -1;
	
	/* Advance to the character past the last slash. */
	++path_end;
	/* Obtain the directory containing the program by truncating the path after the last slash. */
	*path_end = '\0';
	/* The length of the path is the number of characters up through the last slash. */
	return (path_end - buffer);
}

static int lock_reg(int fd, int cmd, int type, off_t offset, int whence, off_t len)
{
	struct flock lock;
	bzero(&lock, sizeof(struct flock));

	lock.l_type = type;
	lock.l_start = offset;
	lock.l_whence = whence;
	lock.l_len = len;

	return (fcntl(fd, cmd, &lock));
}

#define write_lock(fd, offset, whence, len) lock_reg(fd, F_SETLK, F_WRLCK, offset, whence, len)

int check_only_one_process(const char* pidfilename)
{
	assert(NULL != pidfilename);
  
	int fd = open(pidfilename, O_WRONLY|O_CREAT, 0640);
	if (fd < 0)
	{
		syslog(LOG_ERR | LOG_USER, "fail to open pid file. (%s)\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

 	int ret = write_lock(fd, 0, SEEK_SET, 0);
	if (ret < 0)
	{
		if (EACCES == errno || EAGAIN == errno)
		{
			return -1;
		}
		else
		{
			perror("write_lock");
			close(fd);
			exit(EXIT_FAILURE);
		}
	  }

	ret = ftruncate(fd, 0);
	if (ret < 0)
	{
		perror("ftruncate");
		close(fd);
		exit(EXIT_FAILURE);
	}   

	char buf[64];
	snprintf(buf, sizeof(buf), "%ld\n", (long)getpid());

	if (write(fd, buf, strlen(buf)) != (ssize_t)strlen(buf))
	{
		perror("write");
		close(fd);
		exit(EXIT_FAILURE);
	}

	return 0;
}


int create_dma_dev(const char* dev_name, int major, int minor)
{
	assert(NULL != dev_name);
	struct stat file_info;

	//dev_name exists.
	if (access(dev_name, F_OK) == 0)
	{
		if (stat(dev_name, &file_info) != 0)
		{
			return -1;
		}
		if (!S_ISDIR(file_info.st_mode))
			return 0;
	}

	//dev_name doesn't exist.
	int rval = 0;
	dev_t dev;

	dev = MKDEV(major, minor);
	rval = mknod(dev_name, S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH, dev);

	return rval;
}


u_int32_t calc_hashnr(const char *key, u_int32_t length)
{
	u_int32_t nr=1, nr2=4;
	while (length--)
	{
		nr ^= (((nr & 63)+nr2)*((u_int32_t) (unsigned char) *key++))+ (nr << 8);
		nr2 += 3;
	}
	return((u_int32_t) nr);
}



/****************************************
* about string
****************************************/
char* ipaddr2str(u_int32_t ipaddr, char* straddr)
{
	assert(NULL != straddr);

	struct in_addr inaddr;
	memcpy(&inaddr, &ipaddr, sizeof(struct in_addr));
	strcpy(straddr, inet_ntoa(inaddr));

	return straddr;
}

int str2bytesH(const char* str, unsigned char* data, unsigned int data_len)
{
/*
  unsigned int n = 0;
  unsigned int i = 0;
  const char* pstr = str;
  char buf[4];

  assert(NULL != str);
  assert(NULL != data);
  
  if (strlen(str)%2 != 0)
	return 0;

  if ( data_len < strlen(str)/2)
	return 0;

  
  for(i=0; i<strlen(str)/2; i++){
	strncpy(buf, pstr, 2);
	buf[2]=0;
	sscanf(buf,"%x", &(data[i]));
	pstr += 2;
	n++;
  }
  
  return n;
*/
  return -1;
}


//========================================================================
int str2bytesH1(const char* str, unsigned char* data, unsigned int data_len)
{
  unsigned int n = 0;
  unsigned int i = 0;
  const char* pstr = str;

  assert(NULL != str);
  assert(NULL != data);
  
  if (strlen(str)%2 != 0)
	return 0;

  if ( data_len < strlen(str)/2)
	return 0;

  
  for(i=0; i<strlen(str)/2; i++){
	if(*pstr>='0'&& *pstr<='9'){
		data[i]=*pstr-'0';
	}else
	{
		data[i]=tolower(*pstr)-'a'+10;
	}
	data[i]<<=4;
	pstr++;
	if(*pstr>='0'&&*pstr<='9'){
		data[i]+=*pstr-'0';
	}else
	{
		data[i]+=tolower(*pstr)-'a'+10;
	}
 
	pstr ++;
	n++;
  }
  
  return n;

}

//========================================================================
int bytes2strH(const unsigned char* data, int data_len, char* str)
{
  int i = 0;
  char* pstr = str;
  int n = 0;

  assert(NULL != data);
  assert(NULL != str);

  for( i=0; i<data_len; i++){
	sprintf(pstr, "%02X", data[i]);
	pstr += 2;
	n++;
  }
  return n;
}

//========================================================================
int isxdigitstr(const char* str)
{
  assert(NULL != str);
  unsigned int i = 0;
  int ret = 1;

  for(i=0; i<strlen(str); i++){
	if (isxdigit(str[i]) == 0){
		ret = 0;
		break;
	}//if
  }//for

  return ret;
}

int isdigitstr(const char* str)
{
  assert(NULL != str);
  
  unsigned int i = 0;
  int ret = 1;

  for(i=0; i<strlen(str); i++){
	if (isdigit(str[i]) == 0 && str[i] != '-' && str[i] != '+'){
		ret = 0;
		break;
	}//if
  }//for

  return ret;

}


//字符串IP验证
int is_str_ip(const char *sip)
{
	if(strlen(sip) > 15)
		return 0;

	const char *cp = sip;
	while(*cp != '\0')
	{
		if((*cp<'0' || *cp>'9') && (*cp!='.'))
			return 0;
		++cp;
	}

	char str[16];
	int v;
	char *p = str, *p0 = NULL;
	memcpy(str, sip, 16);

	int i;
	for(i=0; i<3; i++)
	{
		p0 = index(p, '.');
		if(NULL == p0)
			return 0;
		if(p == p0)
			return 0;
		*p0 = '\0';
		v = atoi(p);
		if(v<0 || v>255)
			return 0;
		p = ++p0;
	}

	p0 = index(p, '.');
	if(NULL != p0)
		return 0;
	if(*p == '\0')
		return 0;
	v = atoi(p);
	if(v<0 || v>255)
		return 0;

	return 1;
}


//字符串大小写转换
char *str2lower(char *str)
{
	char *p = str;
	while(*p != '\0')
	{
		*p = tolower(*p);
		++p;
	}
	return str;
}

char *str2upper(char *str)
{
	char *p = str;
	while(*p != '\0')
	{
		*p = toupper(*p);
		++p;
	}
	return str;
}


//字符串比较 不区分大小写
int strcmp_nocase(const char *s1, const char *s2)
{
	int n = 0;
	unsigned int i = 0;
	while('\0' != *(s1+i) || '\0' != *(s2+i) )
	{
		n = *(s1+i) - *(s2+i);
		if(0==n || 32==n || -32==n)
			i++;
		else
			return n;
	}
	return 0;
}


//domain字符串会被改成小写字符串
char *trim_domain_string(char *domain)
{
	char *p0, *p1;
	
	str2lower(domain);

	p0 = strstr(domain, "http://");
	if(!p0)
		p0 = domain;
	else
		p0 += 7;

	p1 = strstr(p0, "www.");
	if(!p1)
		p1 = p0;
	else
		p1 += 4;
	
	return p1;
}



/****************************************
* about hash
****************************************/
int url_to_hash(const char* url_info_hash, unsigned char* info_hash) 
{
	assert(NULL != url_info_hash);
	assert(NULL != info_hash);

	int i = 0;
	int j = 0;
	char ch = 0;

	for(i=j=0; url_info_hash[i] && j<20; i++)
	{
		if(url_info_hash[i]=='%' && url_info_hash[i+1]!=0)
		{
			if(url_info_hash[i+1]=='%')
			{
				ch = '%';
				i++;
			}
			else
			{
				if(url_info_hash[i+2]==0) 
					break;
				if(url_info_hash[i+1]>='0' && url_info_hash[i+1]<='9') 
					ch=url_info_hash[i+1]-'0';
				else if(url_info_hash[i+1]>='A' && url_info_hash[i+1]<='F') 
					ch=url_info_hash[i+1]-'A'+10;
				else if(url_info_hash[i+1]>='a' && url_info_hash[i+1]<='f') 
					ch=url_info_hash[i+1]-'a'+10;

				ch = ch<<4;

				if(url_info_hash[i+2]>='0' && url_info_hash[i+2]<='9') 
					ch|=url_info_hash[i+2]-'0';
				else if(url_info_hash[i+2]>='A' && url_info_hash[i+2]<='F') 
					ch|=url_info_hash[i+2]-'A'+10;
				else if(url_info_hash[i+2]>='a' && url_info_hash[i+2]<='f') 
					ch|=url_info_hash[i+2]-'a'+10;

				i+=2;
			}
			info_hash[j++] = ch;
		}
		else if(url_info_hash[i]=='+')
		{
			info_hash[j++] = ' ';
		}
		else
			info_hash[j++] = url_info_hash[i];
	}
	return j;
}

int str_to_hash(const char* url_info_hash, unsigned char* info_hash)
{
	int i = 0;
	int j = 0;
	char ch = 0;

	for(i=j=0; url_info_hash[i] && j<20; i++)
	{
		if(url_info_hash[i]>='0' && url_info_hash[i]<='9')
			ch=url_info_hash[i]-'0';
		else if(url_info_hash[i]>='A' && url_info_hash[i]<='F')
			ch=url_info_hash[i]-'A'+10;
		else if(url_info_hash[i]>='a' && url_info_hash[i]<='f')
			ch=url_info_hash[i]-'a'+10;
		
			ch = ch<<4;
		
		if(url_info_hash[i+1]>='0' && url_info_hash[i+1]<='9')
			ch|=url_info_hash[i+1]-'0';
		else if(url_info_hash[i+1]>='A' && url_info_hash[i+1]<='F')
			ch|=url_info_hash[i+1]-'A'+10;
		else if(url_info_hash[i+1]>='a' && url_info_hash[i+1]<='f')
			ch|=url_info_hash[i+1]-'a'+10;

		i++;
		info_hash[j++] = ch;
	}
	return j;
}


//字符串hash
unsigned long get_str_hash(const char *c)
{
	unsigned long ret=0;
	long n;
	unsigned long v;
	int r;

	if ((c == NULL) || (*c == '\0'))
		return(ret);

	n=0x100;
	while (*c)
	{
		v=n|(*c);
		n+=0x100;
		r= (int)((v>>2)^v)&0x0f;
		ret=(ret<<r)|(ret>>(32-r));
		ret&=0xFFFFFFFFL;
		ret^=v*v;
		c++;
	}
	return (ret>>16)^ret;
}

void hash2string(const char *hash, char *string)
{
	int i;
	char HEX_TABLE[] = "0123456789ABCDEF";
	for(i=0; i<20; i++){
		string[i*2] = HEX_TABLE[(hash[i]>>4) & 0x0F];
		string[i*2+1] = HEX_TABLE[hash[i] & 0x0F];
	}
	string[i*2] = '\0';
}

void string2hash(char *hash, const char *string)
{
	unsigned char c1, c2;
	for( int i=0; i<20; i++){
		if(isdigit(string[i*2]))
			c1 = string[i*2]-'0';
		else if(islower(string[i*2]))
			c1 = string[i*2]-'a'+10;
		else if(isupper(string[i*2]))
			c1 = string[i*2]-'A'+10;

		if(isdigit(string[i*2+1]))
			c2 = string[i*2+1]-'0';
		else if(islower(string[i*2+1]))
			c2 = string[i*2+1]-'a'+10;
		else if(isupper(string[i*2+1]))
			c2 = string[i*2+1]-'A'+10;		

		hash[i] = ((c1 << 4) & 0xF0) | (c2 & 0x0F);
	}
}

void string2hash_ex(char *hash, const char *string, int hashLen)
{
	unsigned char c1, c2;
	for( int i=0; i<hashLen; i++)
	{
		if(isdigit(string[i*2]))
			c1 = string[i*2]-'0';
		else if(islower(string[i*2]))
			c1 = string[i*2]-'a'+10;
		else if(isupper(string[i*2]))
			c1 = string[i*2]-'A'+10;

		if(isdigit(string[i*2+1]))
			c2 = string[i*2+1]-'0';
		else if(islower(string[i*2+1]))
			c2 = string[i*2+1]-'a'+10;
		else if(isupper(string[i*2+1]))
			c2 = string[i*2+1]-'A'+10;		
		hash[i] = ((c1 << 4) & 0xF0) | (c2 & 0x0F);
	}
}


void hash2string_ex(const char *hash, char *string, int hashLen)
{
	int i = 0;
	char HEX_TABLE[] = "0123456789ABCDEF";
	for(i=0; i<hashLen; i++){
		string[i*2] = HEX_TABLE[(hash[i]>>4) & 0x0F];
		string[i*2+1] = HEX_TABLE[hash[i] & 0x0F];
	}
	string[i*2] = '\0';
}



/****************************************
* about dir path
****************************************/
bool PathExist(const char *name)
{
	struct  stat  buff; 
	if (lstat(name,&buff) < 0 )
		return false;
	return true;
}


int  IsDir (const char * name) 
{
	struct  stat  buff;  
	
	if (lstat(name,&buff) < 0 )
	  return -1; //if not exist name ,ignore
	
	/*if is directory return 1 ,else return 0*/ 
	if( S_ISDIR(buff.st_mode) )
		return 0;
	else 
		return -1;
}


int CreatePath(const char *path)
{
	char buf[1024];
	int len;
	char *p ;

	len = strlen(path);
	if(len >= 1023)
		return -1;

	strcpy(buf, path);
	strcat(buf, "/");
	if(buf[0] != '/')
		return -1; 
	
 	p = buf;
	for(; *p == '/'; p++);
	if(*p == '\0')
		return 0;
	while(p)
	{
		p = strchr(p, '/');
		if(p)
		{
			*p = '\0';
			if(IsDir(buf) < 0)
				if(mkdir(buf, 0755) < 0)
					return -1;

			*p = '/';
			p++;
			for(; *p == '/'; p++); 
		}
	}

	return 0;
}



/****************************************
* about compress
****************************************/
int deflate_compress(Bytef *data, uint ndata, Bytef *zdata, uint *nzdata)
{
	z_stream c_stream;
	int err = 0;

	if(data && ndata > 0)
	{
		c_stream.zalloc = (alloc_func)0;
		c_stream.zfree = (free_func)0;
		c_stream.opaque = (voidpf)0;
		if(deflateInit2(&c_stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY) != Z_OK)
			return -1;

		c_stream.next_in  = data;
		c_stream.avail_in  = ndata;
		c_stream.next_out = zdata;
		c_stream.avail_out		= *nzdata;
		while (c_stream.avail_in != 0 && c_stream.total_out < *nzdata)
		{
			if(deflate(&c_stream, Z_NO_FLUSH) != Z_OK)
			return -1;
		}
		if(c_stream.avail_in != 0)
			return c_stream.avail_in;
		for (;;)
		{
			if((err = deflate(&c_stream, Z_FINISH)) == Z_STREAM_END)
				break;
			if(err != Z_OK)
				return -1;
		}
		if(deflateEnd(&c_stream) != Z_OK)
			return -1;
		
		*nzdata = c_stream.total_out;
		return 0;
	}
	
	return -1;
}


/*gzip compress function,for pt send peer to client*/
int gzip_compress(char *dst, uint dstlen, const char *src, uint srclen)
{
	int crc;
	uint dstlen0 = dstlen;
	const char gzhead[10]={0x1f, 0x8b, 0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x3};
	memcpy(dst,gzhead,10);
	deflate_compress((Bytef *)src, srclen, (Bytef *)(dst+10), &dstlen0);
	crc=crc32(0L,Z_NULL,0);
	crc=crc32(crc,(Bytef *)src,srclen);
	dstlen0+=10;
	memcpy(dst+dstlen0,&crc,4);
	dstlen0+=4;
	memcpy(dst+dstlen0,&srclen,4);
	dstlen0+=4;

	return dstlen0; 
}


/****************************************
* about socket
****************************************/
//ip v6 compare
int ipv6_cmp(const unsigned char *ip1, const unsigned char *ip2)
{
	int i;
	for(i=0; i<16; ++i)
	{
		if(ip1[i] == ip2[i])
			continue;
		else
			return (int)(ip1[i] - ip2[i]);
	}
	return 0;
}

double ipv6_precise_cmp(const unsigned char *ip1, const unsigned char *ip2)
{
	double n = 0;
	double m;
	int i;
	for(i=0; i<16; ++i)
	{
		m = ((ip1[i] - ip2[i]));
		n = n * 256;
		n += m;
	}
	return n;
}

unsigned long long ntohll(unsigned long long val)
{
	return (((unsigned long long )htonl((int)((val << 32) >> 32))) << 32) | (unsigned int)htonl((int)(val >> 32));
}

unsigned long long htonll(unsigned long long val)
{
	return (((unsigned long long )htonl((int)((val << 32) >> 32))) << 32) | (unsigned int)htonl((int)(val >> 32));
}

int set_socket_nonblock(int fd)
{
	int opts;
	
	opts=fcntl(fd, F_GETFL);
	if(opts<0)
	{
		perror("fcntl(sock, GETFL)");
		return -1;
	}

	opts = opts |O_NONBLOCK;
	if(fcntl(fd, F_SETFL, opts)<0)
	{
		perror("fcntl(sock, SETFL, opts)");
		return -1;
	}
	
	return 0;
}


//listenip: net order
//listenport: net order
int create_udp_socket(u_int32_t listenip, u_int16_t listenport, int nonblock)
{
	int udp_fd = socket( PF_INET, SOCK_DGRAM,0);
	if(udp_fd == -1)
	{
		syslog(LOG_ERR | LOG_USER, "fail to create udp socket. (%s)\n", strerror(errno));
		return -1;
	}

	if (nonblock)
	{
		if (set_socket_nonblock(udp_fd) != 0)
		{
			syslog(LOG_ERR | LOG_USER, "fail to set nonblock to udp socket. (%s)\n", strerror(errno));
			return -1;
		}
	}

	struct sockaddr_in listenaddr;
	bzero(&listenaddr, sizeof(listenaddr));
	listenaddr.sin_family = AF_INET;
	if (listenip == 0)
		listenaddr.sin_addr.s_addr = INADDR_ANY;
	else
		listenaddr.sin_addr.s_addr = listenip;
	listenaddr.sin_port = listenport;

	int ret=bind(udp_fd,(sockaddr *)&listenaddr, sizeof(listenaddr));
	if(ret != 0)
	{
		syslog(LOG_ERR | LOG_USER, "fail to bind addr to udp socket. (%s)\n", strerror(errno));
		return -1;
	}

	return udp_fd;
}


int init_raw_socket(const char *ifname)
{
	int fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (fd < 0)
	{
		printf("Error: Create raw socket fail!\n");
		return -1;
	}

	struct ifreq ifr;
	memset ((void*)&ifr, 0, sizeof (ifr));
	snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), ifname);

	//get iface id
	if (ioctl(fd, SIOCGIFINDEX, &ifr) < 0)
	{
		printf("Error: Ioctl SIOCGIFINDEX error: %s\n", strerror(errno));
		close(fd);
		return -1;
	}

	//bind iface by iface-id
	struct sockaddr_ll	sll;
	memset(&sll, 0, sizeof(sll));
	sll.sll_family		= AF_PACKET;
	sll.sll_ifindex		= ifr.ifr_ifindex;
	sll.sll_protocol	= htons(ETH_P_ALL);

	if (bind(fd, (struct sockaddr *) &sll, sizeof(sll)) == -1)
	{
		printf("Error: bind socket to iface error: %s\n", strerror(errno));
		close(fd);
		return -1;
	}

	int err;
	socklen_t	 errlen = sizeof(err);
	if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &errlen) == -1)
	{
		printf("Error: Socket get error info fail!\n");
		close(fd);
		return -1;
	}

	if (err == ENETDOWN)
	{
		printf("Error: IFace down!\n");
		close(fd);
		return -1;
	}
	else if (err > 0)
	{
		printf("Error: Socket error(%d): %s\n", err, strerror(errno));
		close(fd);
		return -1;
	}

	return fd;
}

/****************************************
* about debug
****************************************/
void dump_debug_data(const char *data, int data_len, const char *file_name)
{
	FILE *fp = fopen(file_name, "wb");
	if(NULL == fp)
	{
		printf("dump file \"%s\" fail!\n", file_name);
		return;
	}
	fwrite(data, 1, data_len, fp);
	printf("dump file \"%s\" success\n", file_name);
	fclose(fp);
}


