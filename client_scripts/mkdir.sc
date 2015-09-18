user "razvan" "info@starbed.org"
project "qomet-10"
encd ipaddr "172.16.3.2"
sparenodemin 0
sparenoderatio 100


assure num_nodes = 10
export num_nodes

nodeclass ex_class{
	method "thru"
	partition 2
	ostype "FreeBSD"
	
	scenario{
	
           
	    recv my_id
	    #wait for start message
	    recv start_msg
	    callw "/bin/mkdir" "/root/logfile"     

	}
}

nodeset client class ex_class num num_nodes

for(i=0;i<num_nodes;i++){
	client[i].agent.ipaddr = "172.16.1."+tostring(1+i)
	client[i].agent.port = "2345"
} 


scenario{

	#send id to nodes
	for(i=0;i<num_nodes;i++){
		send client[i] tostring(1+i)
	}
       sleep 2
	#send start_msg to all clients
	multisend client "start"
	
	sleep 210
}
	
