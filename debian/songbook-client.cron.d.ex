#
# Regular cron jobs for the songbook-client package
#
0 4	* * *	root	[ -x /usr/bin/songbook-client_maintenance ] && /usr/bin/songbook-client_maintenance
