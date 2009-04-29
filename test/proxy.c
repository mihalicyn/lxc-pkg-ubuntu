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
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <lxc/network.h>

void usage(const char *cmd)
{
	fprintf(stderr, "%s -i <ifname> -f <on|off>\n", cmd);
}

int main(int argc, char *argv[])
{
	char *ifname = NULL, *flag = NULL;
	int opt, ret, family = AF_INET;

	while ((opt = getopt(argc, argv, "6i:f:")) != -1) {
		switch (opt) {
		case 'i':
			ifname = optarg;
			break;
		case 'f':
			flag = optarg;
			break;
		case '6':
			family = AF_INET6;
			break;
		}
	}

	if (!ifname || !flag) {
		usage(argv[0]);
		return 1;
	}

	if (!strcmp(flag, "on"))
		ret = neigh_proxy_on(ifname, family);
	else if (!strcmp(flag, "off"))
		ret = neigh_proxy_off(ifname, family);
	else {
		usage(argv[0]);
		return 1;
	}

	if (ret) {
		fprintf(stderr, "failed for  %s: %s\n", 
			ifname, strerror(-ret));
		return 1;
	}

	return 0;
}
