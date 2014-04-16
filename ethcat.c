#include <net/if.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include "logging.h"
#include "ether.h"


void print_help(char *);

int main(int argc, char *argv[]){

	char ifName[IFNAMSIZ]=DEFAULT_ETHER;
	u_int16_t ether_type=DEFAULT_ETHER_TYPE;
        struct ifreq if_idx;
        struct ifreq if_mac;
	
	int c,i;
	int verbose = 0;
	int sockfd;
	char packet_buffer[ETHER_MAX_LEN+1];
	
	char *packet_data=packet_buffer+sizeof(struct ether_header)+sizeof(u_int16_t);
	u_int16_t *pdata_len=packet_buffer+sizeof(struct ether_header);
	struct ether_header *eh=(struct ether_header *) packet_buffer;
	struct ether_addr *other_mac=NULL;
	pid_t pid=0;
	struct sockaddr_ll socket_address;
	
	
	/* Parse commanf line options */
	while (1) {
		int option_index=0;
		static struct option long_options[] = {
                   {"interface", required_argument, 0, 'i' },
                   {"mac", required_argument, 0, 'm' },
                   {"verbose", no_argument, 0, 'v' },
                   {"type", no_argument, 0, 't' },
                   {"help", no_argument, 0, 'h' },
                   {0, 0, 0, 0 }
	        };

		c = getopt_long(argc, argv, "i:m:vh",
                        long_options, &option_index);

                if (c == -1)
                   break;

		switch (c) {
		case 'i':
			/* set interface name */
			strcpy(ifName,optarg);
			break;
		case 'm':
			/* set mac address of other end */
			other_mac=ether_aton(optarg);
			if(NULL == other_mac) {
				logit(LOG_ERR,"Wrong MAC format <%s>\n",optarg);
				exit(2);
			}
			break;
		case 't':
			/* set ethernet type of packet */
			sscanf(optarg,"%04hx",&ether_type);
			break;
		case 'v':
			verbose++;
			break;
		case 'h':
			print_help(argv[0]);
			exit(0);
		default:
			printf ("\nUse %s --help for options list\n", argv[0]);
			exit(1);
		}
	}
	if(NULL == other_mac) {
		logit(LOG_ERR,"No MAC address of other end\n");
		exit(3);
	}

        /* Open RAW socket to send on */
        if ((sockfd = socket(AF_PACKET, SOCK_RAW, htons(ether_type))) == -1) {
            perror("socket");
	    exit(4);
        }

        /* Get the index of the interface to send on */
        memset(&if_idx, 0, sizeof(struct ifreq));
        strncpy(if_idx.ifr_name, ifName, IFNAMSIZ-1);
        if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0){
            perror("SIOCGIFINDEX");
	    exit(5);
	}
        /* Get the MAC address of the interface to send on */
        memset(&if_mac, 0, sizeof(struct ifreq));
        strncpy(if_mac.ifr_name, ifName, IFNAMSIZ-1);
        if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) < 0){
            perror("SIOCGIFHWADDR");
            exit(6);
	}

        /* Construct the Ethernet header */
        memset(packet_buffer, 0, ETH_FRAME_LEN);
        /* Ethernet header */

	/*Fill header mac addresses */
	/* Source MAC */
	for(i=0;i<ETH_ALEN;i++){
		eh->ether_shost[i] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[i];
	}

	/* Destination MAC */
	memcpy(eh->ether_dhost,other_mac,ETH_ALEN);
	memcpy(&socket_address.sll_addr,other_mac,ETH_ALEN);

        /* Ethertype field */
        eh->ether_type = htons(ether_type);

	/* Index of the network device */
        socket_address.sll_ifindex = if_idx.ifr_ifindex;
        /* Address length*/
        socket_address.sll_halen = ETH_ALEN;

	/*fork between sender and receiver */
	pid=fork();
	if(pid == 0 ){
		/* Child, receiver */
	        logit(LOG_NOTICE,"Start receiver thread %d\n",getpid());

		ssize_t datalen;
		while( datalen= recvfrom(sockfd,packet_buffer,ETH_DATA_LEN,0,NULL,NULL)){
			logit(LOG_DEBUG,"%s",dumppacket(packet_buffer,datalen));

			if(eh->ether_type==htons(ether_type)){
			/* need checking of address FIXME */
				if(ismymac(eh->ether_dhost)&& maceq(eh->ether_shost,other_mac)){
					write(STDIN_FILENO,packet_data,ntohs(*pdata_len));
				}
			}
		}
	}else if (pid == -1) {
		/* parent, error */	
		perror("Fork error\n");
		exit(1);
	}else{
		/*parent, fork ok, sender*/
		ssize_t datalen;
		
		logit(LOG_NOTICE,"Start sending in thread %d\n",getpid());

		/*read data from stdin*/
		while( datalen=read(STDIN_FILENO,packet_data,ETH_DATA_LEN-sizeof(u_int16_t))){
			if(datalen== -1 ){
				perror("I/O error from stdin");
			}else{
			/*send to network*/
				*pdata_len=htons(datalen);
				if (sendto(sockfd, packet_buffer, sizeof(struct ether_header)+sizeof(u_int16_t)+datalen, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0)
				    printf("Send failed\n");
			}
		}

		logit(LOG_NOTICE,"End of input, finishing\n");

		kill(pid, SIGTERM); 
	}


}

void print_help(char *runname){
	
	printf("Usage:\n %s [options]\n\
			Options:\n\
				--interface|-i Name of used interface (default: %s)\n\
				--mac|-m Mac address of other end (required)\n\
				--verbose|-v increase verbosity\n\
				--help|-h This message\n\
		\n",
		runname,DEFAULT_ETHER);
	
}
