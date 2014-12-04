#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <net/ethernet.h>
#include <arpa/inet.h>

char *hexdump(void *,int);

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

int ismymac( const unsigned char *mac){
    /* STUB FIXME */
    return 1;
}

int maceq(const void *mac1, const void *mac2){
    unsigned char *bytep1=(unsigned char*)mac1,*bytep2=(unsigned char *)mac2;
    for(;*bytep1 == *bytep2 && bytep1-(unsigned char*)mac1 < ETH_ALEN ; bytep1++){
	bytep2++;
    }
    return (bytep1 != (unsigned char *)mac1);
}

char *dumppacket( void *packet, int packet_len){
    static char printbuf[4096]; /* size of buffer enougth? */
    struct ether_header *eh=(struct ether_header*)packet;

    sprintf(printbuf,"%s -> %s, type: %02x length: %hd\n%s",
            ether_ntoa((struct ether_addr *)eh->ether_shost),
            ether_ntoa((struct ether_addr *)eh->ether_dhost), 
            ntohs(eh->ether_type),ntohs(*(u_int16_t *)((u_int8_t *)packet+ETHER_HDR_LEN)),
            hexdump((u_int8_t *)packet+ETHER_HDR_LEN+sizeof(u_int16_t),
	    packet_len - ETHER_HDR_LEN));
    return printbuf;
}

#define BYTESPERLINE 32

char *hexdump( void *data, int datalen){
    u_int8_t *wdata=(u_int8_t *)data;
    static char buffer[ETHERMTU*2 + ETHERMTU/BYTESPERLINE+1];
    int i,j=0,bufidx=0;

    if(datalen> ETHERMTU){
	return NULL;
    }
    for(i=0;i<datalen;i++){
	j++;
	if(j>BYTESPERLINE){
	    j=0;
	    /*add newline */
	    buffer[bufidx++]=(char)'\n';
	}
	bufidx+=sprintf(buffer+bufidx,"%02x",wdata[i]);
    }
    return buffer;
}
