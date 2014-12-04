
#define _GNU_SOURCE
#define __USE_GNU

#include <net/if.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include "logging.h"
#include "ether.h"
#include "config.h"



void print_help(char *);

pid_t pid=0;

bool pseudo_tty;
char ttyName[IFNAMSIZ+8]="/dev/tty";

/* log signal number, kill receiver and exit */

static void sig_handler(int sig, siginfo_t *si, void *unused){
    logit(LOG_NOTICE,"Got signal %d", sig);
    kill(pid,sig);
    if(pseudo_tty){
	unlink(ttyName);
    }
    exit(0);
}



#define PTYNAMSIZ 12
int main(int argc, char *argv[],char **envp){

	char ptyName[PTYNAMSIZ+1];
        struct ifreq if_idx;
        struct ifreq if_mac;
	
	int c,i;
	int sockfd;
	char packet_buffer[ETHER_MAX_LEN+1];
	
	char *packet_data=packet_buffer+sizeof(struct ether_header)+sizeof(u_int16_t);
	u_int16_t *pdata_len=(u_int16_t *)(packet_buffer+sizeof(struct ether_header));
	struct ether_header *eh=(struct ether_header *) packet_buffer;
	struct sockaddr_ll socket_address;
	
	pid_t dpid;
	
	int fdin=STDIN_FILENO;
	int fdout=STDOUT_FILENO;
	
	struct etherconf *config=config_read(NULL);
/*	setproctitle_init(argc,argv,envp);*/
	if(config == NULL) { /*errors in config file, use defaults */
	    config=&defconfig;
	}
	
		/* Parse command line options */
	while (1) {
		int option_index=0;
		static struct option long_options[] = {
		   {"interface", required_argument, 0, 'i' },
		   {"uid", required_argument, 0, 'u' },
		   {"mac", required_argument, 0, 'm' },
		   {"verbose", no_argument, 0, 'v' },
		   {"type", required_argument, 0, 't' },
		   {"help", no_argument, 0, 'h' },
		   {"tty", no_argument,0, 'T' },
		   {"section", required_argument,0,'S'},
		   {0, 0, 0, 0 }
		};

		c = getopt_long(argc, argv, "i:m:t:u:vhTS:",
			long_options, &option_index);

		if (c == -1)
		   break;

		if (!getuid()){ /* Use this command-line options ONLY if runned by root */
			switch (c) {
			case 'i':
				/* set interface name */
				config->iface=malloc(strlen(optarg)+1);
				strcpy(config->iface,optarg);
				break;
			case 'm':
				/* set mac address of other end */
				config->mac=ether_aton(optarg);
				if(NULL == config->mac) {
					logit(LOG_ERR,"Wrong MAC format <%s>\n",optarg);
					exit(2);
				}
				break;
			case 't':
				/* set ethernet type of packet */
				sscanf(optarg,"%04hx",&config->ether_type);
				break;
			case 'v':
				config->verbose++;
				break;
			case 'u':
				config->work_uid=atoi(optarg);
				break;
			case 'h':
				print_help(argv[0]);
				exit(0);
			case 'T':
				pseudo_tty=config->pseudo_tty=1;
				break;
			case 'S':
				config=config_section(optarg);
				break;
			default:
				printf ("\nUse %s --help for options list\n", argv[0]);
				exit(1);
			}
		} else { /* non-root users can ONLY select section from config file */
			if(c != 'S') {
				print_help (argv[0]);
				exit(1);
			} else {
				config=config_section(optarg);
				if( config == NULL){
				    printf("\nWrong section name\n");
				    exit(1);
				}
			}
		}
	}
	if(NULL == config->mac) {
		logit(LOG_ERR,"No MAC address of other end\n");
		exit(3);
	}

        /* Open RAW socket to send on */
        if ((sockfd = socket(AF_PACKET, SOCK_RAW, htons(config->ether_type))) == -1) {
            perror("socket");
	    exit(4);
        }

        /* Get the index of the interface to send on */
        memset(&if_idx, 0, sizeof(struct ifreq));
        strncpy(if_idx.ifr_name, config->iface, IFNAMSIZ-1);
        if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0){
            perror("SIOCGIFINDEX");
	    exit(5);
	}
        /* Get the MAC address of the interface to send on */
        memset(&if_mac, 0, sizeof(struct ifreq));
        strncpy(if_mac.ifr_name, config->iface, IFNAMSIZ-1);
        if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) < 0){
            perror("SIOCGIFHWADDR");
            exit(6);
	}

	/*set pseudotty if needed */
	
	if(config->pseudo_tty){
	    strncat(ttyName,config->iface,IFNAMSIZ);
	    fdin=open("/dev/ptmx",O_RDWR|O_NOCTTY);
	    unlockpt(fdin);/*FIXME! Add check result values*/
	    strncpy(ptyName,ptsname(fdin),PTYNAMSIZ);
	    if (config->verbose) logit(LOG_DEBUG,"%s\n",ptyName);
	    chown(ptyName,config->owner,config->group);
	    chmod(ptyName,config->rights);

	    symlink(ptyName,ttyName);
	    fdout=fdin;
	    if((dpid =fork()) != 0){
		/*parent or error */
		if( dpid == -1 ){
		    perror("Fork failed");
		    exit(1);
		}
		/*setproctitle("%s control",ifName);*/
		waitpid(dpid, NULL, 0);
		unlink(ttyName);
		exit(0);
	    }
	    /* daemonize */
	    daemon(0,0);
	}

	
	/* drop privilegies */
	seteuid(config->work_uid);

        /* Construct the Ethernet header */
        memset(packet_buffer, 0, ETH_FRAME_LEN);
        /* Ethernet header */

	/*Fill header mac addresses */
	/* Source MAC */
	for(i=0;i<ETH_ALEN;i++){
		eh->ether_shost[i] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[i];
	}

	/* Destination MAC */
	memcpy(eh->ether_dhost,config->mac,ETH_ALEN);
	memcpy(&socket_address.sll_addr,config->mac,ETH_ALEN);

        /* Ethertype field */
        eh->ether_type = htons(config->ether_type);

	/* Index of the network device */
        socket_address.sll_ifindex = if_idx.ifr_ifindex;
        /* Address length*/
        socket_address.sll_halen = ETH_ALEN;

	/*fork between sender and receiver */
	pid=fork();
	if(pid == 0 ){
		/* Child, receiver */
	        logit(LOG_NOTICE,"Start receiver thread %d\n",getpid());
		/*setproctitle("%s receiver",ifName);*/

		ssize_t datalen;
		while( (datalen= recvfrom(sockfd,packet_buffer,ETH_DATA_LEN,0,NULL,NULL)) ){
			logit(LOG_DEBUG,"%s",dumppacket(packet_buffer,datalen));

			if(eh->ether_type==htons(config->ether_type)){
			/* need checking of address FIXME */
				if(ismymac(eh->ether_dhost)&& maceq(eh->ether_shost,config->mac)){
					write(fdout,packet_data,ntohs(*pdata_len));
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
		struct sigaction sigact;
		
		logit(LOG_NOTICE,"Start sending in thread %d\n",getpid());
		/*setproctitle("%s sender",ifName);*/

		
		/* set signals reaction 
		 SIGHUP, SIGPIPE, SIGINT - kill receiver and exit */
		
		sigemptyset(&sigact.sa_mask);
		sigact.sa_sigaction = sig_handler;

		
		if (sigaction(SIGHUP, &sigact, NULL) == -1 || sigaction(SIGPIPE, &sigact, NULL) == -1 
			|| sigaction(SIGHUP, &sigact, NULL) == -1 ){
		    /*oops, can't set signal */
		    perror("sigaction");
		    kill(pid,SIGTERM);
		    exit(7);
		}

		/*read data from stdin*/
		while( ( datalen=read(fdin,packet_data,ETH_DATA_LEN-sizeof(u_int16_t)) ) ){
			if(datalen== -1 ){
				if(config->pseudo_tty && errno == EIO){ /*PTY was closed */
					sleep(1);
				}else{
					logit(LOG_ERR,"I/O error %d from stdin",errno);
				}
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

return 0;
}


void print_help(char *runname){
	
	printf("Usage:\n %s [options]\n\
			Options:\n\
				--interface|-i Name of used interface (default: %s)\n\
				--mac|-m Mac address of other end (required)\n\
				--type|-t ethernet type\n\
				--uid|-u working uid (default: 65535)\n\
				--verbose|-v increase verbosity\n\
				--help|-h This message\n\
		\n",
		runname,DEFAULT_ETHER);
	
}
