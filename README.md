# A_PLUS_Q
Queuing simulation for ECE 579 

## BUILD
```
make
```

## CLEAN
```
make clean
```

## RUN PROJECT 1
Project 1 -> Simulate FCFS MM1K queue with poisson arrivals
```
./run.o --proj1 Lambda K C L

EG

./run.o --proj1 .5 20 100000 50000

Lambda = Parameter of exponentially distributed inter-arrival times 
         (higher = faster arrivals. Stable for 0 < Lambda < 1)
K = Size of queue
C = Customers to service before exiting
L = Arbitrary integer of which customers to print
```

## TEST
```
./run.o --test

or

./clean_test.sh
```

