#NAME: Bryan Luo
#EMAIL: luobryan@ucla.edu
#ID: 605303956
#!/bin/bash

#check to make sure there's only 1 argument
if [ $# -ne 1 ]
then
	echo "must use only 1 argument to specify csv file"
	exit 1
fi

python3 lab3b.py $1
exit $?
