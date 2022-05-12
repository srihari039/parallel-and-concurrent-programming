

Compilation:
    It should be done using g++.
    One needs to use -lpthread and -latomic flag to compile as the program involves threads.
    command : $ g++ <source.cpp> -lpthread -latomic -o exec

    Now we have executable file ready.
    Create input file with nw,ns,M,uw,us and k respectively with name inp-params.txt

Execution:
    command : $ ./exec
    If proper input file is present, it gives the output.
    It gives the value written by writer thread w_th at timestamp t.
	It also gives the snapshot collected by collector thread c_th at timestamp t.
	It also gives the time elapsed for operations(total time,average time for a snap and worst case time for a snap).



