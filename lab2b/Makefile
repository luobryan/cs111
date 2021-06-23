#NAME: Bryan Luo
#EMAIL: luobryan@ucla.edu
#ID: 605303956

build:
	gcc -Wall -Wextra -g -o lab2_list -pthread lab2_list.c SortedList.c
tests: build
	chmod 777 my_tests.sh
	-./my_tests.sh
graphs: tests
	chmod 777 lab2-list.gp
	-./lab2-list.gp
profile: 
	rm -f raw.gperf
	LD_PRELOAD=/usr/lib64/libprofiler.so CPUPROFILE=./raw.gperf ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text ./lab2_list ./raw.gperf > profile.out 
	pprof --list=thread_worker ./lab2_list ./raw.gperf >> profile.out
	rm -f raw.gperf
dist: graphs profile
	tar -cvzf lab2b-605303956.tar.gz SortedList.h SortedList.c lab2_list.c Makefile lab2b_list.csv profile.out *.png lab2-list.gp README my_tests.sh
clean:
	rm -f lab2_list lab2b-605303956.tar.gz
