

Compilation:
    It should be done using g++.
    One needs to use -lpthread flag to compile as the program involves threads.
    command : $ g++ <source.cpp> -lpthread -o exec

    Now we have executable file ready.
    Create input file with n,k,lambda1,p respectively with name inp-params.txt

Execution:
    command : $ ./exec
    If proper input file is present, it gives out output.txt file.
    It contains every operation start time and completion time.
    It also contains the read value and written value.


