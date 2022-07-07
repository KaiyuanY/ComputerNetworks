# ComputerNetworks

* Labs and projects for EE 450 Computer Networks (C++11)

* Lab1
	* Shortest path implemented with DFS.
	* Input: Node1, Node2, max_hop.
	* Output: Shortest distance between Node1 and Node2 within max_hop.

* Project - TX Chain, mimics the basics of a block chain
	* client.cpp - send check balance/transfer requests to serverM (the main server), connecting using TCP.
	* monitor.cpp - send "TXLIST" request to serverM which commands serverM to compile a list of all transactions happened in all blocks.
	* serverM.cpp - receives commands from clients and monitor. Does encryption and decrypetion of the data, and pull data from and send new transaction data to the back servers. Transfers data back and forth with back servers A, B and C with UDP protocol.
	* serverA.cpp - server A, B and C are essentailly identical the only difference is that they each got their own block file to store transactions. ServerM handles a new transaction and randomly picks one of the three to store the encrypted transactions.