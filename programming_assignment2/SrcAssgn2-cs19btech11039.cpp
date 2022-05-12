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
using namespace std;

//global variables for number of threads, repetition count and lambdas
int number_of_threads,repetition_count;
double lambda1,lambda2;
ofstream file("output.txt");
// lock for proper output
std::mutex file_lock;

// pure abstract class locker 
class locker{
public:
    atomic<double> entry_waiting_times;
    atomic<double> exit_waiting_times;
    virtual void lock(int) = 0;
    virtual void unlock(int) = 0;
    virtual string get_method_name() = 0;
};

// filter lock class
class filter_lock : public locker{
private:
    int* level;
    int* victim;
    int size;
    string methodName = "Filter Lock";
public:
    // constructor
    filter_lock(int number_of_threads){
        size = number_of_threads;
        level = new int[size];
        victim = new int[size];
        for(int i = 0 ; i < size ; i++){
            level[i] = 0;
        }
        entry_waiting_times = 0;
        exit_waiting_times = 0;
    }
    // function which returns the method name
    string get_method_name(){
        return methodName;
    }
    // checker for busy wait loop
    bool check_status(int id,int itr){
        bool flag = false;
        for(int i = 1 ; i < size ; i++){
            if(i == id) continue;
            else{
                if(level[i] >= itr and victim[itr] == id){
                    flag = true;
                } else {
                    return false;
                }
            }
        }
        return flag;
    }
    // lock method
    void lock(int id){
        for(int i = 1 ; i < size ; i++){
            level[id] = i;
            victim[i] = id;
            while(check_status(id,i)){
                
            }
        }
        return;
    }
    // unlock method
    void unlock(int id){
        level[id] = 0;
        return;
    }
};

// petersons lock class
class petersons_lock : public locker{
private:
    bool* flag;
    int* victim;
    int num_threads;
    int top_level;
    string methodName = "Petersons Lock";
public:
    // constructor
    petersons_lock(int number_of_threads){
        num_threads = number_of_threads;
        top_level = log2(num_threads);
        flag = new bool[2*num_threads-1];
        victim = new int[num_threads];
    }

    // lock method to just override the method mentioned in pure abstract class
    void lock(int id){

    }

    // function which returns method name
    string get_method_name(){
        return methodName;
    }

    // check for busy wait loop
    bool check_status(int temp_thread,int parent,int index){
        return flag[temp_thread] and victim[parent] == index;
    }

    // lock method
    void lock(int index,int level,int parent){
        while(level != top_level){
            flag[index] = true;
            victim[parent] = index;
            int temp_thread = index == 2*parent+1 ? index+1 : index-1;
            while(check_status(temp_thread,parent,index)){

            }
            level += 1;
            parent = (parent-1)/2;
        }
        flag[index] = true;
        return;
    }
    // unlock method
    void unlock(int current_index){
        vector<int> indices;
        while(current_index){
            indices.emplace_back(current_index);
            current_index = (current_index-1)/2;
        }
        flag[0] = false;
        while(indices.size()){
            flag[indices.back()] = false;
            indices.pop_back();
        }
    }
};

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

//the test Critical Section function
void testCS(int id,locker* lock) {
	//a for loop for repeated entry of a thread
	for(int i = 0 ; i < repetition_count ; i++) {
		//time stamps for request time, entry time and exit time
		time_t cs_req_time, cs_entry_time, cs_req_exit_time,cs_exit_time;
	
		//get request time
		cs_req_time = time(NULL);
		tm* ltm = localtime(&cs_req_time);

		//printing the request time
        file_lock.lock();
		file<<i<<"th cs request at time: "<<ltm->tm_hour<<":"<<ltm->tm_min<<":"<<ltm->tm_sec<<":"<<get_milliseconds()<<" by thread"<<id<<endl;
        file_lock.unlock();

		//lock section
        lock->lock(id);

		//get entry time
		cs_entry_time = time(NULL);
		ltm = localtime(&cs_entry_time);

        double entry_wait_gap = cs_entry_time-cs_req_time;
        lock->entry_waiting_times = lock->entry_waiting_times+entry_wait_gap;

		//printing the entry time
        file_lock.lock();
		file<<i<<"th cs entry at time: "<<ltm->tm_hour<<":"<<ltm->tm_min<<":"<<ltm->tm_sec<<":"<<get_milliseconds()<<" by thread"<<id<<endl;
        file_lock.unlock();

		//mimicing the functionality of critical section
		sleep(exponential_mimic(lambda1));

		// get request exit time
		cs_req_exit_time = time(NULL);
		ltm = localtime(&cs_req_exit_time);

        file_lock.lock();
		file<<i<<"th cs exit request at time: "<<ltm->tm_hour<<":"<<ltm->tm_min<<":"<<ltm->tm_sec<<":"<<get_milliseconds()<<" by thread"<<id<<endl;
        file_lock.unlock();

		//unlock section
        lock->unlock(id);

        // get exit time
        cs_exit_time = time(NULL);
        ltm = localtime(&cs_exit_time);

        file_lock.lock();
		file<<i<<"th cs exit at time: "<<ltm->tm_hour<<":"<<ltm->tm_min<<":"<<ltm->tm_sec<<":"<<get_milliseconds()<<" by thread"<<id<<endl;
        file_lock.unlock();

        double exit_wait_gap = cs_exit_time-cs_req_exit_time;
        lock->exit_waiting_times = lock->exit_waiting_times + exit_wait_gap;
		sleep(exponential_mimic(lambda2));
	}
}

//function to create new threads of desired size
thread* create_threads(int size) {
	thread* new_threads = new thread[size];
	return new_threads;
}

// Function which creates threads and calls testCS method with the lock type accordingly
void execute_cs_using_lock(int number_of_threads,locker* lock){
    // creating new threads
    thread* worker_threads = create_threads(number_of_threads);
    file<<lock->get_method_name()<<" :"<<endl;
    // assigning each thread the Critical Section with the lock sent
    for(int i = 0 ; i < number_of_threads ; i++){
        worker_threads[i] = thread(testCS,i+1,lock);
    }
    // joining all the threads created
    for(int i = 0 ; i < number_of_threads ; i++){
        worker_threads[i].join();
    }

    file<<"Average waiting time for entry with "<<number_of_threads<<" threads using "<<lock->get_method_name()<<" : "<<lock->entry_waiting_times/number_of_threads<<endl;
    file<<"Average waiting time for exit with "<<number_of_threads<<" threads using "<<lock->get_method_name()<<" : "<<lock->exit_waiting_times/number_of_threads<<endl;

    // delete the assigned memory
    delete[] worker_threads;
    delete lock;
}

int main() {
	//opening the file "inp-params.txt"
	ifstream file_to_be_opened("inp-params.txt");

	//if it is openable without any errors
	if(file_to_be_opened.is_open()) {
		//fetching the inputs for number of threads, repetition count and the lambdas
		file_to_be_opened>>number_of_threads>>repetition_count>>lambda1>>lambda2;
        
        // execute CS using filter Lock
        locker* filterLock = new filter_lock(number_of_threads);
        execute_cs_using_lock(number_of_threads,filterLock);

        // execute CS using petersons lock
        locker* petersonsLock = new petersons_lock(number_of_threads);
        execute_cs_using_lock(number_of_threads,petersonsLock);

        //if it is not openable, prints an error message
	} else {
	    cout<<"Failed to open file \"inp-params.txt\""<<endl;
    }

	return 0;
}