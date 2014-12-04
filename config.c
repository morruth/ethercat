#include <dotconf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "logging.h"
#include "config.h"
#include "ether.h"

static DOTCONF_CB(cb_iface);
static DOTCONF_CB(cb_uid);
static DOTCONF_CB(cb_mac);
static DOTCONF_CB(cb_verbose);
static DOTCONF_CB(cb_etype);
static DOTCONF_CB(cb_tty);
static DOTCONF_CB(cb_ttyown);
static DOTCONF_CB(cb_ttymod);
static DOTCONF_CB(cb_section);
static DOTCONF_CB(cb_name);
static FUNC_ERRORHANDLER(errorhandler);



static const configoption_t options[] = {
	{"Interface", ARG_STR, cb_iface, NULL, CTX_ALL},
	{"Name", ARG_STR, cb_name, NULL, CTX_ALL},
	{"Uid", ARG_INT, cb_uid, NULL, CTX_ALL},
	{"Mac", ARG_STR, cb_mac, NULL, CTX_ALL},
	{"Verbose", ARG_INT, cb_verbose, NULL, CTX_ALL},
	{"Ethertype", ARG_STR, cb_etype, NULL, CTX_ALL},
	{"Tty", ARG_TOGGLE, cb_tty, NULL, CTX_ALL},
	{"Ttyown", ARG_STR, cb_ttyown, NULL, CTX_ALL},
	{"Ttymod", ARG_INT, cb_ttymod, NULL, CTX_ALL},
	{"[Section]", ARG_NONE, cb_section, NULL, CTX_ALL},
	LAST_OPTION
};

static struct etherconf *configs;
static struct etherconf *config;
static int sections_idx=0;

struct etherconf *config_read(char *config_filename){
	configfile_t *configfile;

	configfile = dotconf_create(config_filename ? config_filename : DEFAULT_CONFIGFILE,
				    options, NULL, CASE_INSENSITIVE);
	if (!configfile) {
		return NULL;
	}
	configs=malloc(sizeof(struct etherconf));
	configs[sections_idx]=defconfig;
	config=configs;
	config->name=DEFAULT_SECTION;

        configfile->errorhandler = (dotconf_errorhandler_t) errorhandler;

	if (dotconf_command_loop(configfile) == 0){
		logit(LOG_ERR, "Error reading config file\n");
		dotconf_cleanup(configfile);
		return NULL;
	}
	if(config->name == NULL){ /* no name for last section */
		logit(LOG_ERR,"Section unnamed");
		return NULL;
	}

	if(configfile)dotconf_cleanup(configfile);
	return configs;
}

struct etherconf *config_section(char *section_name){
	int i;
	for(i=0;i<=sections_idx;i++){
	    if(strcmp(configs[sections_idx].name,section_name) == 0 ){
		return configs+i;
	    }
	}
	return NULL;
}


DOTCONF_CB(cb_iface)
{
/*	printf("%s:%ld: %s: [%s]\n",
	       cmd->configfile->filename, cmd->configfile->line,
	       cmd->name, cmd->data.str);*/
	config->iface=malloc(strlen(cmd->data.str)+1);
	strcpy(config->iface,cmd->data.str);
	return NULL;
}

DOTCONF_CB(cb_uid)
{
	config->work_uid=cmd->data.value;
	return NULL;
}

DOTCONF_CB(cb_verbose)
{
	config->verbose=cmd->data.value;
	return NULL;
}

DOTCONF_CB(cb_tty)
{
	config->pseudo_tty=cmd->data.value;
	return NULL;
}

DOTCONF_CB(cb_ttymod)
{
	config->rights=cmd->data.value;
	return NULL;
}


DOTCONF_CB(cb_ttyown)
{
	uid_t owner;
	gid_t group;
	int count=0;
	count=sscanf(cmd->data.str," %d:%d ",&owner,&group);
	if (count == 2){
		config->owner=owner;
		config->group=group;
	}else{
		return "Wrong tty owner";
	}

	return NULL;
}

DOTCONF_CB(cb_etype)
{
	if(sscanf(cmd->data.str," %04hx ",&config->ether_type)==1){
		return NULL;
	}else {
		return "Wrong ethernet type";
	}
}



FUNC_ERRORHANDLER(errorhandler)
{
        logit(LOG_ERR,"Config Error: %s", msg);
        return 0;
}

DOTCONF_CB(cb_section)
{
	struct etherconf *new_configs;
	if(configs[sections_idx].name == NULL ){
		/* No name in previous section */
		return "Section unnamed";
	}
	sections_idx++;
	new_configs=realloc(configs,sizeof(struct etherconf)*(sections_idx+1));
	if( new_configs == NULL){
		return "Memory allocation for section failed";
	}
	configs=new_configs;
	configs[sections_idx]=defconfig;
	config=configs+sections_idx;

	return NULL;
}

DOTCONF_CB(cb_name)
{
	config->name=malloc(strlen(cmd->data.str)+1);

	return NULL;
}

DOTCONF_CB(cb_mac)
{
	config->mac=ether_aton(cmd->data.str);
        if(NULL == config->mac) {
		return "Wrong MAC format\n";
        }

	return NULL;
}


/*
  vim:set ts=4:
  vim:set shiftwidth=4:
*/
