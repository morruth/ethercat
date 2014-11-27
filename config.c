#include <dotconf.h>
#include <stdio.h>
#include <string.h>
#include "logging.h"
#include "config.h"

static DOTCONF_CB(cb_iface);
static DOTCONF_CB(cb_uid);
static DOTCONF_CB(cb_mac);
static DOTCONF_CB(cb_verbose);
static DOTCONF_CB(cb_etype);
static DOTCONF_CB(cb_tty);
static DOTCONF_CB(cb_ttyown);
static DOTCONF_CB(cb_ttymod);



static const configoption_t options[] = {
	{"Interface", ARG_STR, cb_iface, NULL, CTX_ALL},
	{"Uid", ARG_INT, cb_uid, NULL, CTX_ALL},
	{"Mac", ARG_STR, cb_mac, NULL, CTX_ALL},
	{"Verbose", ARG_INT, cb_verbose, NULL, CTX_ALL},
	{"Ethertype", ARG_STR, cb_etype, NULL, CTX_ALL},
	{"Tty", ARG_TOGGLE, cb_tty, NULL, CTX_ALL},
	{"Ttyown", ARG_STR, cb_ttyown, NULL, CTX_ALL},
	{"Ttymod", ARG_IN, cb_ttymod, NULL, CTX_ALL},
	LAST_OPTION
};


static const etherconf config={
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

int config_read(char *config_filename){
	configfile_t *configfile;

	configfile = dotconf_create(config_filename ? config_filename : DEFAULT_CONFIG,
				    options, NULL, CASE_INSENSITIVE);
	if (!configfile) {
		logit(LOG_ERR, "Error opening config file\n");
		dotconf_cleanup(configfile);
		return NULL;
	}

	if (dotconf_command_loop(configfile) == 0){
		logit(LOG_ERR, "Error reading config file\n");
		dotconf_cleanup(configfile);
		return NULL;
	}

	dotconf_cleanup(configfile);
	return &config;
}



DOTCONF_CB(cb_list)
{
	int i;
	printf("%s:%ld: %s: [  ",
	       cmd->configfile->filename, cmd->configfile->line, cmd->name);
	for (i = 0; i < cmd->arg_count; i++)
		printf("(%d) %s  ", i, cmd->data.list[i]);
	printf("]\n");
	return NULL;
}

DOTCONF_CB(cb_str)
{
	printf("%s:%ld: %s: [%s]\n",
	       cmd->configfile->filename, cmd->configfile->line,
	       cmd->name, cmd->data.str);
	return NULL;
}

/*
  vim:set ts=4:
  vim:set shiftwidth=4:
*/
