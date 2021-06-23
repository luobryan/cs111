#!/bin/bash

#NAME: Bryan Luo
#EMAIL: luobryan@ucla.edu
#ID: 605303956

rm -f lab2b_list.csv
touch lab2b_list.csv

# lab2b_1.png and lab2b_2.png (lab2b_2.png does not require 12 threads case)
for t in 1, 2, 4, 8, 12, 16, 24
do
	./lab2_list --iterations=1000 --threads=$t --sync=s >> lab2b_list.csv
        ./lab2_list --iterations=1000 --threads=$t --sync=m >> lab2b_list.csv     
done

#lab2b_3.png
for t in 1, 4, 8, 12, 16
do
  	for i in 1, 2, 4, 8, 16
        do
          	./lab2_list --iterations=$i --threads=$t --yield=id --lists=4 >> lab2b_list.csv
        done
done

for t in 1, 4, 8, 12, 16
do
  	for i in 10, 20, 40, 80
        do
          	./lab2_list --iterations=$i --threads=$t --yield=id --lists=4 --sync=s >> lab2b_list.csv
                ./lab2_list --iterations=$i --threads=$t --yield=id --lists=4 --sync=m >> lab2b_list.csv
        done
done

#lab2b_4.png and lab2b_5.png 
for t in 1, 2, 4, 8, 12
do
  	for l in 4, 8, 16   #list size of 1 is already handled in the lab2b_1.png / lab2b_2.png block
        do
          	./lab2_list --iterations=1000 --threads=$t --lists=$l --sync=s >> lab2b_list.csv
                ./lab2_list --iterations=1000 --threads=$t --lists=$l --sync=m >> lab2b_list.csv
        done
done
