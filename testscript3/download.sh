if [ $# -lt 2 ]
then
    echo "ERROR: Not enough arguments were provided!"
    exit 1
fi

# Define variables
a=$1
b=${a}
echo "$b"
a=$2
c=${a}
echo "$c"

scp -r guest@172.16.3.2:/root/$b $c
