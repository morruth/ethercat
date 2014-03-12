#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <unistd.h>
#include <linux/if_packet.h>
#include <sys/ioctl.h>


#define	DEFAULT_ETHER_TYPE 0x9600 /*default ethernet type is com1 port most often used speed ;)*/
#define	DEFAULT_ETHER	"eth0"
/*
 * Convert an ASCII representation of an ethernet address to  binary form.
 */
struct ether_addr *ether_aton(const char *);

/*
 * Convert a binary ethernet address to an ASCII string.
 */

char *ether_ntoa(const struct ether_addr *);
