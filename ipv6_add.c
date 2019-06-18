#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>


void ipv6_addition(unsigned char *ipaddr, int n)
{
	uint32_t *p = (uint32_t *)ipaddr;
	int64_t tmp = 0, tmp2 = 0;
	int i = 0;

	if(n == 0){
		return;
	}
	
	for (i=0; i<4; i++){
		tmp = htonl(*(p+4-1-i));
		//printf("[%s:%d] *(p+4-1-i)=%x\n",__func__, __LINE__, ntohl(*(p+4-1-i)));
		tmp += n;
		printf("[%s:%d] tmp=%lx\n",__func__, __LINE__, tmp);
		if(tmp <0){
			n = -1;
			*(p+4-1-i) = ntohl(tmp&0xffffffff);
			continue;
		}
		tmp2 = tmp - 0xffffffffU;
		//printf("[%s:%d] tmp2=%lx\n",__func__, __LINE__, tmp2);
		if (tmp2 >0){
			*(p+4-1-i) = ntohl(tmp&0xffffffff);
			n = (tmp>>32)&0xffffffff;
			//printf("[%s:%d] n=%d\n",__func__, __LINE__, n);
		}
		else{
			*(p+4-1-i) = ntohl(tmp&0xffffffff);
			break;
		}
		
	}
}

void ipv6_subtraction(unsigned char *ipaddr, int n)
{
}

int print_ipaddr(const unsigned char *ip)
{
	int i = 0;
	for(i=0; i< 16; i++){
		printf("%02X ", ip[i]);
		//if(!((i+1)%4) && i)
		if(i==3 || i==7 || i==11)
		{
			printf(",");
		}
	}
	printf("\n");
	return 0;
}

int ipv6_add_test(const unsigned char *ip, int n)
{
	int i = 0;
	unsigned char tmp[16] = {0};
	for(i=0; i< 16; i++){
		tmp[i] = ip[i];
	}

	ipv6_addition(tmp, n);
	printf("========\nadd %d\n",n);
	print_ipaddr(tmp);
}

int main(void)
{
#if 0
	unsigned short a= 0x03;
	short b = -8;
	
	printf("%u, %x\n", a, a);
	printf("%u, %x, %x\n", a+b, a+b, ~(a+b));
#endif

	unsigned char ip_array[16] = {0x00, 0x00,0x00,0x00,
								0x00, 0x00,0x00,0x00,
								0x00, 0x00,0x00,0x00,
								0xff,0xff,0xff,0xff};
	
	unsigned char ip_array2[16] = {0x00, 0x00,0x00,0x00,
								0x00, 0x00,0x00,0x00,
								0x00, 0x00,0x00,0x00,
								0x00, 0x00,0x00,0x03};

	unsigned char ip_array3[16] = {0xff,0xff,0xff,0xff,
								0xff,0xff,0xff,0xff,
								0xff,0xff,0xff,0xff,
								0xff,0xff,0xff,0xff};
	//ipv6_add_test(ip_array2, -0x2);
	//ipv6_add_test(ip_array2, -0xf3);
	//ipv6_add_test(ip_array2, -0xf4);
	//ipv6_add_test(ip_array3, 1);
	ipv6_add_test(ip_array3, 3);
	return 0;
}




