#NAME: Bryan Luo
#EMAIL: luobryan@ucla.edu
#ID: 605303956

build:
	gcc -Wall -Wextra -o lab2_add -pthread lab2_add.c
	gcc -Wall -Wextra -o lab2_list -pthread lab2_list.c SortedList.c
tests: build
	chmod 777 my_tests.sh
	./my_tests.sh
graphs: tests
	chmod 777 lab2_add.gp
	chmod 777 lab2_list.gp
	./lab2_add.gp
	./lab2_list.gp
dist: graphs
	tar -cvzf lab2a-605303956.tar.gz lab2_add.c SortedList.h SortedList.c lab2_list.c Makefile lab2_add.csv lab2_list.csv *.png my_tests.sh README lab2_add.gp lab2_list.gp
clean:
	rm -f lab2_add lab2_list lab2a-605303956.tar.gz
