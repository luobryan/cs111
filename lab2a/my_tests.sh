#!/bin/bash

rm -f lab2_add.csv
rm -f lab2_list.csv
touch lab2_add.csv
touch lab2_list.csv

# first, test add none (on a range of iterations)
for t in 1, 2, 4, 8, 12
do
	for i in 10, 20, 40, 80, 100, 1000, 10000, 100000
	do
		./lab2_add --iterations=$i --threads=$t >> lab2_add.csv
	done
done


#then test add-yield-none
for t in 1, 2, 4, 8, 12
do
        for i in 10, 20, 40, 80, 100, 1000, 10000, 100000
        do
                ./lab2_add --iterations=$i --threads=$t	--yield >> lab2_add.csv
        done
done


#add-m
for t in 1, 2, 4, 8, 12
do
  	for i in 10, 20, 40, 80, 100, 1000, 10000, 100000
        do
          	./lab2_add --iterations=$i --threads=$t --sync="m" >> lab2_add.csv
        done
done

#add-s
for t in 1, 2, 4, 8, 12
do
  	for i in 10, 20, 40, 80, 100, 1000, 10000, 100000
        do
          	./lab2_add --iterations=$i --threads=$t --sync="s" >> lab2_add.csv
        done
done

#add-c
for t in 1, 2, 4, 8, 12
do
  	for i in 10, 20, 40, 80, 100, 1000, 10000, 100000
        do
          	./lab2_add --iterations=$i --threads=$t --sync=c >> lab2_add.csv
        done
done
#add-yield-m
for t in 1, 2, 4, 8, 12
do
  	for i in 10, 20, 40, 80, 100, 1000, 10000, 100000
        do
          	./lab2_add --iterations=$i --threads=$t --yield --sync=m >> lab2_add.csv
        done
done
#add-yield-s
for t in 1, 2, 4, 8, 12
do
  	for i in 10, 20, 40, 80, 100, 1000, 10000, 100000 #100000 might take a while
        do
          	./lab2_add --iterations=$i --threads=$t --yield --sync=s  >> lab2_add.csv
        done
done

#add-yield-c
for t in 1, 2, 4, 8, 12
do
  	for i in 10, 20, 40, 80, 100, 1000, 10000, 100000
        do
          	./lab2_add --iterations=$i --threads=$t --yield --sync=c >> lab2_add.csv
        done
done

#initial lab2_list tests on a single thread 
for i in 10, 100, 1000, 10000, 20000, 50000
do
	./lab2_list --iteration=$i --threads=1 >>lab2_list.csv
done


#multiple threads
for t in 2, 4, 8, 12
do
	for i in 1, 10, 100, 1000
	do
		./lab2_list --iterations=$i --threads=$t >>lab2_list.csv
	done
done

#list yield i
for t in 2, 4, 8, 12
do
        for i in 1, 2, 4, 8, 16, 32
        do
                ./lab2_list --iterations=$i --threads=$t --yield=i >>lab2_list.csv
        done
done

#list yield d
for t in 2, 4, 8, 12
do
  	for i in 1, 2, 4, 8, 16, 32
        do
          	./lab2_list --iterations=$i --threads=$t --yield=d >>lab2_list.csv
        done
done

#list yield il
for t in 2, 4, 8, 12
do
  	for i in 1, 2, 4, 8, 16, 32
        do
          	./lab2_list --iterations=$i --threads=$t --yield=il >>lab2_list.csv
        done
done

#list yield dl

for t in 2, 4, 8, 12
do
  	for i in 1, 2, 4, 8, 16, 32
        do
          	./lab2_list --iterations=$i --threads=$t --yield=dl >>lab2_list.csv
        done
done


#ALL 4 LIST YIELD COMBINATIONS WITH SYNC M
for t in 2, 4, 8, 12
do
        for i in 1, 2, 4, 8, 16, 32
        do
                ./lab2_list --iterations=$i --threads=$t --yield=i --sync=m >>lab2_list.csv
        done
done
for t in 2, 4, 8, 12
do
  	for i in 1, 2, 4, 8, 16, 32
        do
          	./lab2_list --iterations=$i --threads=$t --yield=d --sync=m >>lab2_list.csv
        done
done
for t in 2, 4, 8, 12
do
  	for i in 1, 2, 4, 8, 16, 32
        do
          	./lab2_list --iterations=$i --threads=$t --yield=il --sync=m >>lab2_list.csv
        done
done
for t in 2, 4, 8, 12
do
  	for i in 1, 2, 4, 8, 16, 32
        do
          	./lab2_list --iterations=$i --threads=$t --yield=dl --sync=m >>lab2_list.csv
        done
done

#ALL 4 LIST YIELD COMBINATIONS WITH SYNC S
for t in 2, 4, 8, 12
do
        for i in 1, 2, 4, 8, 16, 32
        do
                ./lab2_list --iterations=$i --threads=$t --yield=i --sync=s >>lab2_list.csv
        done
done
for t in 2, 4, 8, 12
do
  	for i in 1, 2, 4, 8, 16, 32
        do
          	./lab2_list --iterations=$i --threads=$t --yield=d --sync=s >>lab2_list.csv
        done
done
for t in 2, 4, 8, 12
do
  	for i in 1, 2, 4, 8, 16, 32
        do
          	./lab2_list --iterations=$i --threads=$t --yield=il --sync=s >>lab2_list.csv
        done
done
for t in 2, 4, 8, 12
do
  	for i in 1, 2, 4, 8, 16, 32
        do
          	./lab2_list --iterations=$i --threads=$t --yield=dl --sync=s >>lab2_list.csv
        done
done
#overcome startup costs
for t in 1, 2, 4, 8, 12, 16, 24
do	
	./lab2_list --iterations=1000 --threads=$t >>lab2_list.csv 
	./lab2_list --iterations=1000 --threads=$t --sync=m >>lab2_list.csv
	./lab2_list --iterations=1000 --threads=$t --sync=s >>lab2_list.csv
done 
