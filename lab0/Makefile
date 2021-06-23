#NAME: Bryan Luo
#EMAIL: luobryan@ucla.edu
#ID: 605303956
default: lab0
lab0: lab0.c
	gcc lab0.c -o lab0 -Wall -Wextra
check:
	gcc lab0.c -o lab0 -Wall -Wextra
	touch one.txt two.txt
	echo "AAa" >one.txt
	echo "BBb" >two.txt
	./lab0 --input=one.txt --output=two.txt;\
	if [ `$$?` -eq `0` ] ;\
		then echo 'passed exit code 0 test';\
		else echo 'failed exit code 0 test';\
	fi;
	if cmp -s one.txt two.txt;\
		then echo 'passed --input and --output option test';\
		else echo 'failed --input and --output option test';\
	fi;
	echo "11" >one.txt
	echo "22" >two.txt
	./lab0 <one.txt >two.txt;\
	if cmp -s one.txt two.txt;\
		then echo 'passed stdin and stdout test';\
	    else echo 'failed stdin and stdout test';\
	fi;
	./lab0 -i --unrecognizedoption;\
	if [ `$$?` -eq `1` ] ;\
	then\
		echo 'passed exit code 1 test';\
	else\
		echo 'failed exit code 1 test';\
	fi;
	./lab0 --input=asdf.txt;\
	if [ `$$?` -eq `2` ] ; \
	then\
		echo 'passed exit code 2 test';\
	else\
		echo 'failed exit code 2 test';\
	fi;
	touch asdf.txt
	chmod 444 asdf.txt
	./lab0 --output=asdf.txt;\
	if [ `$$?` -eq `3` ] ;\
	then\
		echo 'passed exit code 3 test';\
	else\
		echo 'failed exit code 3 test';\
	fi;
	./lab0 --catch --segfault;\
	if [ `$$?` -eq `4` ] ;\
	then\
		echo 'passed exit code 4 test';\
	else\
		echo 'failed exit code 4 test';\
	fi; \
        chmod 777 asdf.txt
	rm one.txt two.txt asdf.txt
clean:
	rm -f *.o *.gz lab0
dist:
	tar -zcvf lab0-605303956.tar.gz lab0.c Makefile README backtrace.png breakpoint.png
