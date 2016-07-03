#
# Regular cron jobs for the ramblerpcutility package
#
0 4	* * *	root	[ -x /usr/bin/ramblerpcutility_maintenance ] && /usr/bin/ramblerpcutility_maintenance
