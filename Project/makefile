all: serverA serverB serverC serverM client monitor
serverA: serverA.cpp
	g++ -std=c++11 -o serverA serverA.cpp
serverB: serverB.cpp
	g++ -std=c++11 -o serverB serverB.cpp
serverC: serverC.cpp
	g++ -std=c++11 -o serverC serverC.cpp
serverM: serverM.cpp
	g++ -std=c++11 -o serverM serverM.cpp -lpthread

client: client.cpp
	g++ -std=c++11 -o client client.cpp
monitor: monitor.cpp
	g++ -std=c++11 -o monitor monitor.cpp

clean:
	rm serverA serverB serverC serverM monitor client