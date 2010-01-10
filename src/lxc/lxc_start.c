/*
 * lxc: linux Container library
 *
 * (C) Copyright IBM Corp. 2007, 2008
 *
 * Authors:
 * Daniel Lezcano <dlezcano at fr.ibm.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#define _GNU_SOURCE
#include <stdio.h>
#undef _GNU_SOURCE
#include <libgen.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>

#include <lxc/lxc.h>
#include <lxc/log.h>
#include <lxc/utils.h>

#include "arguments.h"
#include "config.h"

lxc_log_define(lxc_start, lxc);

static int my_parser(struct lxc_arguments* args, int c, char* arg)
{
	switch (c) {
	case 'd': args->daemonize = 1; break;
	case 'f': args->rcfile = arg; break;
	}
	return 0;
}

static const struct option my_longopts[] = {
	{"daemon", no_argument, 0, 'd'},
	{"rcfile", required_argument, 0, 'f'},
	LXC_COMMON_OPTIONS
};

static struct lxc_arguments my_args = {
	.progname = "lxc-start",
	.help     = "\
--name=NAME -- COMMAND\n\
\n\
lxc-start start COMMAND in specified container NAME\n\
\n\
Options :\n\
  -n, --name=NAME      NAME for name of the container\n\
  -d, --daemon         daemonize the container\n\
  -f, --rcfile=FILE    Load configuration file FILE\n",
	.options   = my_longopts,
	.parser    = my_parser,
	.checker   = NULL,
	.daemonize = 0,
};

static int save_tty(struct termios *tios)
{
	if (!isatty(0))
		return 0;

	if (tcgetattr(0, tios))
		WARN("failed to get current terminal settings : %s",
		     strerror(errno));

	return 0;
}

static int restore_tty(struct termios *tios)
{
	struct termios current_tios;
	void (*oldhandler)(int);
	int ret;

	if (!isatty(0))
		return 0;

	if (tcgetattr(0, &current_tios)) {
		ERROR("failed to get current terminal settings : %s",
		      strerror(errno));
		return -1;
	}

	if (!memcmp(tios, &current_tios, sizeof(*tios)))
		return 0;

	oldhandler = signal(SIGTTOU, SIG_IGN);
	ret = tcsetattr(0, TCSADRAIN, tios);
	if (ret)
		ERROR("failed to restore terminal attributes");
	signal(SIGTTOU, oldhandler);

	return ret;
}

int main(int argc, char *argv[])
{
	char *const *args;
	int err = -1;
	struct termios tios;

	char *const default_args[] = {
		"/sbin/init",
		'\0',
	};

	char *rcfile = NULL;

	if (lxc_arguments_parse(&my_args, argc, argv))
		return err;

	if (!my_args.argc)
		args = default_args; 
	else
		args = my_args.argv;

	if (lxc_log_init(my_args.log_file, my_args.log_priority,
			 my_args.progname, my_args.quiet))
		return err;

	/* rcfile is specified in the cli option */
	if (my_args.rcfile)
		rcfile = (char *)my_args.rcfile;
	else {
		if (!asprintf(&rcfile, LXCPATH "/%s/config", my_args.name)) {
			SYSERROR("failed to allocate memory");
			return err;
		}

		/* container configuration does not exist */
		if (access(rcfile, F_OK)) {
			free(rcfile);
			rcfile = NULL;
		}
	}

	if (my_args.daemonize) {

                /* do not chdir as we want to open the log file,
		 * change the directory right after.
		 * do not close 0, 1, 2, we want to do that
		 * ourself because we don't want /dev/null
		 * being reopened.
		 */
		if (daemon(1, 1)) {
			SYSERROR("failed to daemonize '%s'", my_args.name);
			return err;
		}

		lxc_close_inherited_fd(0);
		lxc_close_inherited_fd(1);
		lxc_close_inherited_fd(2);

		if (my_args.log_file) {
			open(my_args.log_file, O_WRONLY | O_CLOEXEC);
			open(my_args.log_file, O_RDONLY | O_CLOEXEC);
			open(my_args.log_file, O_RDONLY | O_CLOEXEC);
		}
	}

	save_tty(&tios);

	err = lxc_start(my_args.name, args, rcfile);

	restore_tty(&tios);

	return err;
}

