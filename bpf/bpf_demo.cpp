#include <iostream>

//#include <stdio.h>
#if 1
#include <errno.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>

//#include <pcap.h>
#include <linux/filter.h>
#include <linux/if_packet.h>

#include <netinet/if_ether.h>
#endif // 0

#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

using namespace std;

#if 0
struct sock_filter code_tcp[] = {
//{ 0x5, 0, 0, 0x00000001 },//0    //jump to 2,dirty hack from tcpdump -d's output
//{ 0x5, 0, 0, 0x00000000 },//1
{ 0x30, 0, 0, 0x00000009 },//2
{ 0x15, 0, 6, 0x00000006 },//3
{ 0x28, 0, 0, 0x00000006 },//4
{ 0x45, 4, 0, 0x00001fff },//5
{ 0xb1, 0, 0, 0x00000000 },//6
{ 0x48, 0, 0, 0x00000002 },//7
{ 0x15, 0, 1, 0x0000fffe },//8   //modify this fffe to the port you listen on
{ 0x6, 0, 0, 0x0000ffff },//9
{ 0x6, 0, 0, 0x00000000 },//10
};
int code_tcp_port_index = 6;

#else

struct sock_filter code_tcp[] = {
//{ 0x28, 0, 0, 0x0000000c },
//{ 0x15, 0, 9, 0x00000800 },
{ 0x30, 0, 0, 0x00000009 },
{ 0x15, 0, 7, 0x00000006 },
{ 0x28, 0, 0, 0x00000006 },
{ 0x45, 5, 0, 0x00001fff },
{ 0xb1, 0, 0, 0x00000000 },
{ 0x48, 0, 0, 0x00000002 },
{ 0x35, 0, 2, 0x00000064 },
{ 0x25, 1, 0, 0x000000c8 },
{ 0x6, 0, 0, 0x00040000 },
{ 0x6, 0, 0, 0x00000000 },

};
int code_tcp_port_index = 5;
#endif

void init_filter(int sock, int port, int port2)
{
	sock_fprog bpf;

	bpf.len = sizeof(code_tcp)/sizeof(code_tcp[0]);
	printf("filter len=%d\n", bpf.len);

	/*
	printf("%d, %d, %d\n", code_tcp[5].k, code_tcp[13].k, code_tcp[14].k);
    code_tcp[code_tcp_port_index].k=port;

    code_tcp_port_index = 13;
    code_tcp[code_tcp_port_index].k=port;
    code_tcp[code_tcp_port_index+1].k=port2;

    printf("%d, %d, %d\n", code_tcp[3].k, code_tcp[13].k, code_tcp[14].k);
    */
    bpf.filter = code_tcp;
    int dummy;

	int ret=setsockopt(sock, SOL_SOCKET, SO_DETACH_FILTER, &dummy, sizeof(dummy)); //in case i forgot to remove
	if (ret != 0)
	{
		printf("error remove fiter\n");
		//perror("filter");
		//exit(-1);
	}
	ret = setsockopt(sock, SOL_SOCKET, SO_ATTACH_FILTER, &bpf, sizeof(bpf));
	if (ret != 0)
	{
		printf("error set fiter\n");
		//perror("filter");
		exit(-1);
	}

}

int main()
{

    sockaddr_in ipv4_addr;
    socklen_t sockaddr_len = sizeof(sockaddr);
    int g_packet_buf_len;
    unsigned char packet_buf[2000] = {0};

    cout << "Hello world!" << endl;
#if 1
    int raw_recv_fd= socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP));
    if (raw_recv_fd < 0){
        cout << "socket "<< std::endl;
        return 0;
    }

    init_filter(raw_recv_fd, 5000, 5500);

    while(1){

        g_packet_buf_len = recvfrom(raw_recv_fd, packet_buf, sizeof(packet_buf), 0 ,(sockaddr*)&ipv4_addr , &sockaddr_len);
        if (g_packet_buf_len < 0){
            perror("recvfrom");
            break;
        }

        for (int i=0; i<g_packet_buf_len; ++i){
            printf("%02X ", packet_buf[i]);
        }
        printf("\n");

        struct iphdr *ip = (struct iphdr *)packet_buf;
        struct tcphdr *tcp = nullptr;
        struct udphdr *udp = nullptr;
        switch (ip->protocol){
        case IPPROTO_TCP:
            tcp = (struct tcphdr *)(packet_buf + (ip->ihl <<2));
            printf("tcp dst port: %d\n", ntohs(tcp->dest));
            break;
        case IPPROTO_UDP:
            udp = (struct udphdr *)(packet_buf + (ip->ihl <<2));
            printf("udp dst port: %d\n", ntohs(udp->dest));
            break;
        }


    }

#endif // 0
    return 0;
}
