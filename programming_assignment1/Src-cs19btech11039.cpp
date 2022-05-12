// including useful headers
#include <bits/stdc++.h>
#include <chrono>
#include <fstream>
#include <thread>
#include <mutex>
#include <set>
using namespace std;

// global variables
long long iterable = 0;
std::mutex locker;

// output files to dump the output
ofstream times("Times.txt");
ofstream dam_output("Primes-DAM.txt");
ofstream sam1_output("Primes-SAM1.txt");
ofstream sam2_output("Primes-SAM2.txt");

// timer class to calculate time in the program
// clock starts when memory is allocated
// clock stops and outputs time elapsed when memory is deleted
class Timer{
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime,stopTime;
    string method_name;
    // stop function
    void stop(){
        stopTime = std::chrono::high_resolution_clock::now();
        auto start = std::chrono::time_point_cast<std::chrono::microseconds>(startTime).time_since_epoch().count();
        auto stop = std::chrono::time_point_cast<std::chrono::microseconds>(stopTime).time_since_epoch().count();
        auto duration = stop-start;
        times<<"Time elapsed for "<<method_name<<" : "<<duration<<"us"<<endl;
    }
public:
    // constructor
    Timer(string name){
        startTime = std::chrono::high_resolution_clock::now();
        method_name = name;
    }
    // destructor
    ~Timer(){
        stop();
    }
};

// boolean function to check if a number is prime or not
bool isPrime(int n){
    if(n < 2) return false;
    if(n%2 == 0) return n == 2;

    int maxCap = (int)sqrt((double)n);
    for(int i = 3 ; i <= maxCap ; i+=2){
        if(n%i == 0) return false;
    }

    return true;
}

// Dynamic allocation method
void dam(long long upperBound){
    long long num = 0;
    // while under limit, increment and check if it's prime
    while(num < upperBound){
        // using lock to handle race conditions
        locker.lock();
        num = iterable++;
        locker.unlock();
        if(num >= upperBound) return;
        else if(isPrime(num)){
            locker.lock();
            dam_output<<num<<" ";
            locker.unlock();
        }
    }
    return;
}

// Static allocation method
void sam1(long long upperBound,int index,int no_th){
    long long it = 0;
    // loop over a certain numbers in each thread and check if numbers are prime
    for(it = index+1 ; it < upperBound ; it += no_th){
        if(isPrime(it)){
            locker.lock();
            sam1_output<<it<<" ";
            locker.unlock();
        }
    }
    return;
}

// improvised static allocation method
void sam2(long long upperBound,int index,int no_th){
    long long it = 0;
    // set increment correctly and follow sam1
    int increment = 0;
    if(index%2 == 0 and no_th%2 == 0 or index%2 == 1 and no_th%2 == 1) increment = no_th;
    else if(index%2 == 1 and no_th%2 == 0) return;
    else increment = 2*no_th;
    for(it = index+1 ; it < upperBound ; it += increment){
        if(isPrime(it)){
            locker.lock();
            sam2_output<<it<<" ";
            locker.unlock();
        }
    }
    return;
}

// main function
int main(){

    // input file
    ifstream input;

    int n,m;
    long long upperBound;

    // open the input file and read the data
    input.open("inp-params.txt");
    input>>n>>m;
    input.close();

    // create the threads as per the number in the input
    std::thread child_threads[m];
    // calculate the upper limit
    upperBound = pow(10,n);

    // instantiate timer
    Timer* measureTime = new Timer("DAM");

    // create threads for DAM method
    for(int i = 0 ; i < m ; i++){
        child_threads[i] = std::thread(dam,upperBound);
    }
    // join the created threads
    for(int i = 0 ; i < m ; i++){
        child_threads[i].join();
    }
    // stop the timer
    delete measureTime;

    // instantiate timer
    measureTime = new Timer("SAM1");

    // create threads for SAM method
    for(int i = 0 ; i < m ; i++){
        child_threads[i] = std::thread(sam1,upperBound,i,m);
    }
    // join the created threads
    for(int i = 0 ; i < m ; i++){
        child_threads[i].join();
    }
    // stop the timer
    delete measureTime;

    // instantiate timer
    measureTime = new Timer("SAM2");
    // feeding the only even prime number into the output file
    sam2_output<<2<<" ";

    // create threads for SAM method
    for(int i = 0 ; i < m ; i++){
        child_threads[i] = std::thread(sam2,upperBound,i,m);
    }
    // join the created threads
    for(int i = 0 ; i < m ; i++){
        child_threads[i].join();
    }
    // stop the timer
    delete measureTime;

    // end of the program
    return 0;
}