echo "run experiment"
/usr/local/springos/bin/master -p razraz -S /home/hoku/QOMET/springos/preamble.sc /root/testtcpdump/test5.sc
echo "close all program"
/usr/local/springos/bin/master -p razraz -S /home/hoku/QOMET/springos/preamble.sc /root/testscript/close_all.sc