if [ $# -lt 1 ]
then
    echo "ERROR: Not enough arguments were provided!"
    exit 1
fi

# Define variables
a=$1
b=${a}
echo "$b"
/usr/sbin/tcpdump -vvv -e -n -i tap0 >>/root/logfile/$b.txt &