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


/*
 * Convert an ASCII representation of an ethernet address to  binary form.
 */
struct ether_addr *ether_aton(const char *);

/*
 * Convert a binary ethernet address to an ASCII string.
 */

char *ether_ntoa(const struct ether_addr *);
char *dumppacket( void *, int );
int ismymac( const unsigned char *mac);
int maceq(const void *mac1, const void *mac2);

