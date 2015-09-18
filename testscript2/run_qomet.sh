if [ $# -lt 1 ]
then
    echo "ERROR: Not enough arguments were provided!"
    exit 1
fi

# Define variables
a=$1
b=${a}
echo "$b"
/root/ntrtrung/qomet-1.6-beta/do_wireconf -q /root/Read_Config/scenario_file.xml.bin -i $b -m 0.5 -b 10.0.0.255 -s /root/Read_Config/setting_file_4qomet >>/dev/null