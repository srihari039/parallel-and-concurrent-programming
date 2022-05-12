
Compilation:
    It should be done using g++.
    One needs to use -lpthread flag to compile as the program involves threads.
    command : $ g++ source.cpp -lpthread -o exec

    Now we have executable file ready.
    Create input file with n and m respectively.

Execution:
    command : $ ./exec
    If proper input file is present, it gives out 
        1 . Times.txt which contains times taken for each of the methods
        2 . Primes-DAM.txt which has primes obtained from DAM
        3 . Primes-SAM1.txt which has primes obtained from SAM1
        4 . Primes-SAM2.txt which has primes obtained from SAM2

