#NAME: Bryan Luo
#EMAIL: luobryan@ucla.edu
#ID: 605303956

SHELL:=/bin/bash
.ONESHELL:

default:
	uname -a | grep -q 'armv7l'
	if  [[ $$? -ne 0 ]]
	then
		gcc lab4c_tcp.c -o lab4c_tcp -g -Wall -Wextra -DDUMMY -lm
		gcc lab4c_tls.c -o lab4c_tls -g -Wall -Wextra -DDUMMY -lm -lssl -lcrypto
	else
		gcc lab4c_tcp.c -o lab4c_tcp -g -Wall -Wextra -lmraa -lm
		gcc lab4c_tls.c -o lab4c_tls -g -Wall -Wextra -lmraa -lm -lssl -lcrypto
	fi

dist: 
	tar -cvzf lab4c-605303956.tar.gz Makefile README lab4c_tcp.c lab4c_tls.c  

clean:
	rm -f *.tar.gz lab4c_tcp lab4c_tls


