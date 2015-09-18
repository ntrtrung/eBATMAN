if [ $# -lt 1 ]
then
    echo "ERROR: Not enough arguments were provided!"
    echo "Command function: Run QOMET-based WLAN emulation experiment"
    echo "Usage: $(basename $0) <qomet_output_file>"
    exit 1
fi
# Define variables
a=$1
ppid=${a}
echo kill $ppid
for i in `ps -l| awk '$3 == '${ppid}' { print $2 }'`
do
echo killing $i
kill -9 $i
done
kill -9 $a


