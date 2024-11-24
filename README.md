# pub-sub

It is a distributed pub-sub model with multiple brokers, a load balancer, and multiple publishers and subscribers.

Instructions to run:

To run this do the following steps:
1. First compile the server using:
$ make server
You can then copy the executable on different computers on the same network, then run those servers.
I copied it to different sl-2 machines then ran the servers there using ssh.

2. Compile the load-balancer using:
$ make load-balancer
then run the load-balancer, enter the ip:port of all the brokers to the load-balancer

3. Run the subscribers and publishers as before.
Enter the ip:port of the load-balancer to the subscribers and publishers.
They will communicate to the load balancer to ask the ip:port of the broker
they need to communicate to, the load-balancer will return the ip then they
will connect to the broker.
Use the loadgen.sh to generate load for the servers.

Brokers are distributed over the topics by the load-balancer, so one topic is handled by only one broker.

