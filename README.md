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

## RUN PROJECT 2
Project 2 -> Simulate MM1K queue with poisson arrivals (various disciplines)
Simulate "Webserver"

```
             ------------------------------------------------------------------<--|
            |                                                                     |
            v                                           -> [IO-Q-1]->[IO-SERV1]-->|
            |                                          /p=.1                       \
            |                                         /                              \
[customers] -> [CPU-Q]->[CPU-SERV]->[LD-BLNC]-------->------->[IO-Q-2]->[IO-SERV-2]-->|
                                                   |  \     p=.1                     / 
                                                   |   \p=.1                        /
                                                   |    -> [IO--3]->[IO-SERV-3]---->
                                                   v
                                         (leave system with .7 probability)
```

```
./run.o --proj2 Lambda K-cpu K-io C L M

EG

./run.o --proj2 .3 40 5 3333 0 3

Lambda = Parameter of exponentially distributed inter-arrival times 
         (higher = faster arrivals. Stable for 0 < Lambda < 1)
K-cpu = Size of CPU Queue
K-IO = size of IO queues
C = Customers to service before exiting
L = Select mode (0 is mm1k, 1 is CPU)
M = Select Discipline (1 – FCFS, 2 – LCFS-NP, 3 – SJF-NP, 4 – Prio-NP, 5 – Prio-P)
```

## TEST
```
./run.o --test

or

./clean_test.sh
```

