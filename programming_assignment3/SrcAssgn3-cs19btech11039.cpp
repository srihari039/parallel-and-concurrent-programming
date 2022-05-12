//including header files
#include <iostream>
#include <thread>
#include <fstream>
#include <random>
#include <unistd.h>
#include <mutex>
#include <time.h>
#include <cmath>
#include <chrono>
#include <atomic>
#include <climits>
using namespace std;

//global variables for number of threads, repetition count and lambdas
int number_of_threads,repetition_count;
double lambda,probability;
ofstream outfile("output.txt");
std::mutex file_lock;

//using in built functions to get a random real time simulation time
double exponential_mimic(double lambda){
	uniform_real_distribution<double> distribution(pow(lambda,-1));
	random_device randomdevice;
	default_random_engine generate(randomdevice());
	double distribute = distribution(generate);
	return distribute;
}

//function to get time in milli seconds
int get_milliseconds(){
	using namespace std::chrono;
	auto now = system_clock::now();
	auto ms = duration_cast<milliseconds>(now.time_since_epoch())%1000;
	return ms.count();
}

// interface class for registers
class Register{
public:
    virtual int read() = 0;
    virtual void write(int,int) = 0;
    long read_time = 0;
    long write_time = 0;
    int reads = 0;
    int writes = 0;
};

// stamped value class, implemented based on the textbook
class StampedValue{
public:
    long stamp;
    int value;

    // constructors
    StampedValue(){
    }

    StampedValue(int init){
        stamp = 0;
        value = init;
    }

    StampedValue(long stamp_,int value_){
        stamp = stamp_;
        value = value_;
    }

    // static function which returns max stamped value
    static StampedValue max(StampedValue&x,StampedValue&y){
        return x.stamp > y.stamp ? x:y;
    }
};

// atomic mrmw register which shapes on the base interface register
class atomicMRMWregister : public Register{
private:
    StampedValue* table;
    int capacity = 0;
public:
    // constructor
    atomicMRMWregister(int capacity_,int init){
        capacity = capacity_;
        table = new StampedValue[capacity_];
        StampedValue value = StampedValue(init);
        for(int i = 0 ; i < capacity_ ; i++){
            table[i] = value;
        }
    }
    // read function
    int read(){
        StampedValue max = StampedValue(INT_MIN);
        for(int i = 0 ; i < capacity ; i++){
            max = max.max(max,table[i]);
        }
        return max.value;
    }
    // write function
    void write(int value,int id){
        StampedValue max = StampedValue(INT_MIN);
        for(int i = 0 ; i < capacity ; i++){
            max = max.max(max,table[i]);
        }
        table[id] = StampedValue(max.stamp+1,value);
    }
};

// inbuilt atomic register which shapes from the base register
class inbuiltatomicregister : public Register{
    atomic<int> sharedVariable;
public:
    // constructor
    inbuiltatomicregister(int value){
        sharedVariable = value;
    }
    // read function
    int read(){
        int ret_value = sharedVariable.load(std::memory_order_relaxed);
        return ret_value;
    }
    // write function
    void write(int value_,int id){
        sharedVariable = value_;
    }
};


// function which tests and measures the registers
void testAtomic(int id,Register* register_){

    int local_variable;
    bool read;
    time_t cs_entry_time, cs_exit_time;
    // re-loop 
    for(int i = 0 ; i < repetition_count ; i++){
        double prob = ((double)rand()/(RAND_MAX));
        read = prob <= probability;
        cs_entry_time = time(NULL);

        // recording the start time
        tm* ltm = localtime(&cs_entry_time);
        file_lock.lock();
		outfile<<i<<"th action requested at time: "<<ltm->tm_hour<<":"<<ltm->tm_min<<":"<<ltm->tm_sec<<":"<<get_milliseconds()<<" by thread"<<id<<endl;
        file_lock.unlock();

        if(read) {
            auto start = chrono::high_resolution_clock::now();
            local_variable = register_->read();
            auto elapsed = chrono::high_resolution_clock::now()-start;
            long timetaken = chrono::duration_cast<chrono::microseconds>(elapsed).count();
            register_->read_time += timetaken;
            register_->reads += 1;
            // register_->avg_read_times.emplace_back(timetaken);

            file_lock.lock();
            outfile<<"Value read : "<<local_variable<<endl;
            file_lock.unlock();

        } else {

            local_variable = repetition_count*id;
            auto start = chrono::high_resolution_clock::now();
            register_->write(local_variable,id);
            auto elapsed = chrono::high_resolution_clock::now()-start;
            long timetaken = chrono::duration_cast<chrono::microseconds>(elapsed).count();
            register_->write_time += timetaken;
            register_->writes += 1;
            // register_->avg_write_times.emplace_back(timetaken);
            file_lock.lock();
            outfile<<"Value written : "<<local_variable<<endl;
            file_lock.unlock();
        }

        // recording the completion time
        cs_exit_time = time(NULL);
        ltm = localtime(&cs_exit_time);
        file_lock.lock();
		outfile<<i<<"th action completed at time: "<<ltm->tm_hour<<":"<<ltm->tm_min<<":"<<ltm->tm_sec<<":"<<get_milliseconds()<<" by thread"<<id<<endl;
        file_lock.unlock();

        // delete ltm;
        ltm = nullptr;
        // mimicing the sleep
        sleep(exponential_mimic(lambda));
    }
}

// utility funtion which creates threads and assigns functions to it
void execute(Register* register_){

    // creates new threads
    thread* worker_threads = new thread[number_of_threads];
    for(int i = 0 ; i < number_of_threads ; i++){
        worker_threads[i] = thread(testAtomic,i,register_);
    }
    // joins all the threads
    for(int i = 0 ; i < number_of_threads ; i++){
        worker_threads[i].join();
    }
    delete[] worker_threads;

    // get number of reads and writes
    int reads = register_->reads;
    int writes = register_->writes;

    // get the total time and calculate average times
    double total_reading_time = register_->read_time;
    double avg_reading_time = total_reading_time/reads;

    double total_writing_time = register_->write_time;
    double avg_writing_time = total_writing_time/writes;

    double total_time = total_reading_time+total_writing_time;
    double avg_time = total_time/(reads+writes);

    // printing the observed attributes
    cout<<"Average reading time for register : "<<avg_reading_time<<"us"<<endl;
    cout<<"Average writing time for register : "<<avg_writing_time<<"us"<<endl;
    cout<<"Average total time for register : "<<avg_time<<"us"<<endl;
    return;
}

// main function where the program starts
int main(){
    // seeding the random generator with NULL
    srand(time(NULL));

    // open the input file
    ifstream infile("inp-params.txt");

    // proceed if the file is openable and valid
    if(infile.is_open()){
        // read the input from the input file
        infile>>number_of_threads>>repetition_count>>lambda>>probability;

        // creating an atomic mrmw register
        cout<<"MRMW register : "<<endl;
        Register* mrmwregister = new atomicMRMWregister(number_of_threads,0);
        // executing the created register
        execute(mrmwregister);
        cout<<endl;

        // creating the inbuilt register
        cout<<"Inbuilt atomic register : "<<endl;
        Register* atomicregister = new inbuiltatomicregister(0);
        // executing the created register
        execute(atomicregister);

        // deleting the memory allocated
        delete mrmwregister;
        delete atomicregister;
    // else exit from the program
    } else {
        cout<<"Unable to open the file"<<endl;
    }
    infile.close();
    return 0;
}
