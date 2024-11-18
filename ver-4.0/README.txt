This is a simple pub-sub model with 1 broker/server serving multiple publishers and subscribers.
To run this first compile the server using:
$ make server

Then run the server:
./server.out <port_number>

Then run the loadgen file and publishers will start publishing and subscribers will start subscribing and receiving messages. 
To stop it just do ctrl+c on the server and then the clients will also exit.

