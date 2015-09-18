if [ $# -lt 1 ]
then
    echo "ERROR: Not enough arguments were provided!"
    echo "Command function: Run QOMET-based WLAN emulation experiment"
    echo "Usage: $(basename $0) <qomet_output_file>"
    exit 1
fi

# Define variables
a=$1
b=${a}
echo "$b"
ping $b
