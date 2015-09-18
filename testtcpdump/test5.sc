user "razvan" "info@starbed.org"
project "qomet-10"
encd ipaddr "172.16.3.2"
sparenodemin 0
sparenoderatio 100


assure num_nodes = 51
export num_nodes

nodeclass ex_class{
	method "thru"
	partition 2
	ostype "FreeBSD"
	
	scenario{
	
           
	    recv my_id
	    #wait for start message
	    recv start_msg
	    callw "/bin/sh" "/root/ntrtrung/run_ex.sh" 
  	    callw "/bin/sh" "/root/ntrtrung/run_tcpdump.sh" my_id
	    sleep 180

	}
}

nodeset client class ex_class num num_nodes

for(i=0;i<12;i++){
	client[i].agent.ipaddr = "172.16.1."+tostring(1+i)
	client[i].agent.port = "2345"
} 
for(i=13;i<40;i++){
	client[i].agent.ipaddr = "172.16.1."+tostring(1+i)
	client[i].agent.port = "2345"
} 
	client[50].agent.ipaddr = "172.16.1."+tostring(51)
	client[50].agent.port = "2345"

scenario{

	#send id to nodes
	for(i=0;i<12;i++){
		send client[i] tostring(i)
	}
	send client[50] tostring(12)
	for(i=13;i<40;i++){
		send client[i] tostring(i)
	}
       sleep 2
	#send start_msg to all clients
	multisend client "start"
	sleep 200
}
	
