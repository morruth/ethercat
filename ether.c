#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <net/ethernet.h>
/*
 * Convert an ASCII representation of an ethernet address to  binary form.
 */
struct ether_addr *ether_aton(const char *straddr) {
        int scancount;
	static struct ether_addr binaddr;
	unsigned int b0, b1, b2, b3, b4, b5;

        scancount = sscanf(straddr, "%02x:%02x:%02x:%02x:%02x:%02x", &b0, &b1, &b2, &b3, &b4, &b5);

        if (scancount != 6)
                return (NULL);

        binaddr.ether_addr_octet[0]=b0;
	binaddr.ether_addr_octet[1]=b1;
	binaddr.ether_addr_octet[2]=b2;
	binaddr.ether_addr_octet[3]=b3;
	binaddr.ether_addr_octet[4]=b4;
	binaddr.ether_addr_octet[5]=b5;

        return ((struct ether_addr *)&binaddr);
}

/*
 * Convert a binary ethernet address to an ASCII string.
 */

char *ether_ntoa(const struct ether_addr *binaddr) {
        int printcount;
	static char straddr[18];

        printcount = sprintf(straddr, "%02x:%02x:%02x:%02x:%02x:%02x",
	    binaddr->ether_addr_octet[0], binaddr->ether_addr_octet[1], binaddr->ether_addr_octet[2],
	    binaddr->ether_addr_octet[3], binaddr->ether_addr_octet[4], binaddr->ether_addr_octet[5]);
        if (printcount < 17)
                return (NULL);
        return ((char *)&straddr);
}
