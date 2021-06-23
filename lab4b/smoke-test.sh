#!/bin/bash

#NAME: Bryan Luo
#EMAIL: luobryan@ucla.edu
#ID: 605303956


{ sleep 2; echo STOP; sleep 1; echo START; sleep 1; echo SCALE=C; sleep 1; echo OFF; } | ./lab4b --log=my_log.txt

if [[ $? -ne 0 ]]
then
	echo "wrong return value"
else
	echo "correct return value"
fi

grep STOP my_log.txt

if [[ $? -ne 0 ]]
then
        echo "STOP not logged"
else
        echo "STOP logged properly"
fi

grep START my_log.txt

if [[ $? -ne 0 ]]
then
        echo "START not logged"
else
        echo "START logged properly"
fi

grep SCALE=C my_log.txt

if [[ $? -ne 0 ]]
then
        echo "SCALE=C not logged"
else
        echo "SCALE=C logged properly"
fi

grep OFF my_log.txt

if [[ $? -ne 0 ]]
then
        echo "OFF not logged"
else
        echo "OFF logged properly"
fi

rm -f my_log.txt
