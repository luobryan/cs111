#NAME: Bryan Luo
#EMAIL: luobryan@ucla.edu
#ID: 605303956
default: lab1b
lab1b: lab1b-server.c lab1b-client.c
	gcc lab1b-server.c -o lab1b-server -Wall -Wextra -lz
	gcc lab1b-client.c -o lab1b-client -Wall -Wextra -lz
clean:
	rm -f *.o *.gz lab1b-server lab1b-client
dist:
	tar -zcvf lab1b-605303956.tar.gz lab1b-client.c lab1b-server.c Makefile README
