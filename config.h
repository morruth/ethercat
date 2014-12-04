#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define	DEFAULT_ETHER_TYPE 0x9600 /*default ethernet type is com1 port most often used speed ;)*/
#define	DEFAULT_ETHER	"eth0"
#define DEFAULT_UID 65535
#define DEFAULT_VERBOSITY 0
#define DEFAULT_TTY 0
#define DEFAULT_TTYOWNER 10 /* uucp FIXME!!! Debianism*/
#define DEFAULT_TTYGROUP 10
#define DEFAULT_TTYRIGHTS 0660
#define DEFAULT_CONFIGFILE "/etc/ethercat.conf"
#define DEFAULT_SECTION "default"


#define bool int

struct etherconf {
	char *name;
	char *iface;
	uid_t work_uid;
	struct ether_addr *mac;
	int verbose;
	u_int16_t ether_type;
	bool pseudo_tty;
	uid_t owner;
	gid_t group;
	mode_t rights;
};


static struct etherconf defconfig={
	NULL,
	DEFAULT_ETHER,
	DEFAULT_UID,
	NULL,
	DEFAULT_VERBOSITY,
	DEFAULT_ETHER_TYPE,
	DEFAULT_TTY,
	DEFAULT_TTYOWNER,
	DEFAULT_TTYGROUP,
	DEFAULT_TTYRIGHTS
};

struct etherconf *config_read(char *config_filename);
struct etherconf *config_section(char *config_section);
