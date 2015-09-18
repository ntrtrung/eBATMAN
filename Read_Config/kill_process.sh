if [ $# -lt 1 ]
then
    echo "Usage: kill_process <process_id>"
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


