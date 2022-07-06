a.	Name: Kaiyuan Yu
b.	USC ID: 4748635080

c.	Finished everything except for the extra credit part. 

d. 	client.cpp: sends check wallet or transfer commands to serverM via TCP connection, then gets response from serverM wether the action was successful.
	monitor.cpp: sends TXLIST command to serverM to let serverM output a file called txchain.txt, which contains all transaction history from 3 backend servers (A, B, C), in ascending order of the transaction number.
	serverM.cpp: receives commands from client and monitor with TCP connection, and perform actions accordingly, retrieves information from 3 back servers via UDP connection. It is multithreaded. One thread listening to client messages and the other for monitor. Does encryption&decryption when needed. 
	serverA.cpp: reads block1.txt to retrieve the data entries the serverM wants. Sends data to serverM using UDP (line containing specific user, or all if it is TXLIST). 
	serverB.cpp: same as serverA except for it reads block2.txt and a different UDP port number.
	serverC.cpp:same as serverA except for it reads block3.txt and a different UDP port number.

e.	Client sends "B USER_NAME" or "T USER_NAME_1 USER_NAME2 Amount", serverM reads the first character and knows what to do with the rest of the data.
	Monitor only sends "TXLIST" and the serverM will pull all data from back servers. If you try to pass wrong message it will be discarded.
	ServerM encrypts the names. It sends the encrypted name and gets back all entries associated with that name. It then decrypts the received entries for calculations. When it receives an entry starting with '#', and followed by a number. It knows that this is the end of that block. And the number is the maximum serial number in that block, so it can compare the 3 serial numbers to determine what is the serial number for the next transaction. 
	Server A, B & C sends the requested entries (lines), encryted and unchanged, one at a time to serverM. When it finishes sending it sends a "# <number>" message indicating that this is the end, and the maximum serial number in this block is included in this message.

g.	My program used 2 threads for client and monitor (one for each). Through my testing (sequetial client and monitor requests) it is working fine. But if you try to flood my program with many client and monitor commands at the same time it might have problem 	because it is multithreaded and the serverM does use the same udp port to get data from back servers for both monitor and client. 

h.	Yes. I wrote encrypt() and decrypt() as a separate function in serverM and used it serveral places in serverM.cpp when I need to do encryption and decryption.
	I also had a setup_socket(int port) function in serverM to set up monitor and client connection since they are both TCP connection and have a lot in common in setup.
	Then I have this int "pull_from_back_server(int &send_fd, sockaddr_in &sender_addr, int &recv_fd, std::string name, std::vector<Transaction*> &records)" function for communicating between serverM and the 3 back servers because the 3 servers are mostly the same and can be handled in the same procedure but with different file descriptor, address, etc. parameters.