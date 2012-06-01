'''apport package hook for lxc

(c) 2012 Canonical Ltd.
Author:
Serge Hallyn <serge.hallyn@ubuntu.com>
'''

from apport.hookutils import *
from os import path
import re

def add_info(report):
	attach_related_packages(report, ['dnsmasq', 'dnsmasq-base', 'libvirt-bin', 'apparmor', 'libapparmor1', 'apparmor-utils', 'auditd', 'libaudit0'])
	attach_mac_events(report)
	attach_upstart_overrides(report, "lxc")
	command_output(['ls', '-ld', '/bin/sh'])
	attach_conffiles(report, 'lxc')
	report["lxcsyslog"] = recent_syslog(re.compile("lxc"))
	# should we attach all lxc apparmor files
	#command_output(['ls', '-l', '/etc/apparmor.d/lxc']
	#command_output(['cat', '/etc/apparmor.d/lxc/*']
