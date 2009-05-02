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
#include <stdio.h>
#include <libgen.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>

#include <lxc/lxc.h>
#include "confile.h"

void usage(char *cmd)
{
	fprintf(stderr, "%s <command>\n", basename(cmd));
	fprintf(stderr, "\t -n <name>    : name of the container\n");
	fprintf(stderr, "\t -f <confile> : path of the configuration file\n");
	_exit(1);
}

int main(int argc, char *argv[])
{
	const char *name = NULL, *file = NULL;
	struct lxc_conf lxc_conf;
	int err, opt;

	while ((opt = getopt(argc, argv, "f:n:")) != -1) {
		switch (opt) {
		case 'n':
			name = optarg;
			break;
		case 'f':
			file = optarg;
			break;
		}
	}

	if (!name)
		usage(argv[0]);

	if (lxc_conf_init(&lxc_conf)) {
		fprintf(stderr, "failed to initialize the configuration\n");
		return 1;
	}

	if (file && lxc_config_read(file, &lxc_conf)) {
		fprintf(stderr, "invalid configuration file\n");
		return 1;
	}

	err = lxc_create(name, &lxc_conf);
	if (err) {
		fprintf(stderr, "%s\n", lxc_strerror(err));
		return 1;
	}

	return 0;
}

