#os202-lab

### Compiling 

```
g++ -std=c++0x main.cpp -o main
```

### Running
Provide input the 6 data in the command line

M, the machine size in words.
P, the page size in words.
S, the size of each process, i.e., the references are to virtual addresses 0..S-1.
J, the ‘‘job mix’’, which determines A, B, and C, as described below.
N, the number of references for each process.
R, the replacement algorithm, FIFO, RANDOM, or LRU

```
./main 10 10 20 1 10 lru
```
