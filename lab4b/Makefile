#NAME: Bryan Luo
#EMAIL: luobryan@ucla.edu
#ID: 605303956

SHELL:=/bin/bash
.ONESHELL:

default:
	uname -a | grep -q 'armv7l'
	if  [[ $$? -ne 0 ]]
	then
		gcc lab4b.c -o lab4b -g -Wall -Wextra -DDUMMY -lm
	else
		gcc lab4b.c -o lab4b -g -Wall -Wextra -lmraa -lm
	fi

dist: default
	tar -cvzf lab4b-605303956.tar.gz Makefile README lab4b.c smoke-test.sh

clean:
	rm -f *.tar.gz lab4b
check: default
	chmod 777 smoke-test.sh
	./smoke-test.sh

