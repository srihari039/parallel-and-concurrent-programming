// including all useful libraries
#include <iostream>
#include <vector>
#include <random>
#include <unistd.h>
#include <atomic>
#include <time.h>
#include <thread>
#include <fstream>
#include <mutex>
#include <iomanip>
#include <map>
#include <sstream>
#include <algorithm>
#include <climits>
using namespace std;

// writer and snap thread functions
void writer(int);
void snapshot(int);

// input variables
int nw,ns,M,k;
double us,uw;

// atomic term for terminating writing threads
atomic<bool> term(false);
atomic<long> total_time_taken(0);
atomic<int> worst_case_time(0);

// locks for storing log messages
mutex writer_lock;
mutex snapper_lock;

// containers for log messages for both writer threads and snapper threads
map<string,vector<string>> writerLog;
map<string,vector<pair<int,vector<int>>>> snapperLog;

// utility function which gives time in HH:MM:SS.microseconds
string time_in_HH_MM_SS_MMM() {
    using namespace std::chrono;
    // get current time
    std::chrono::_V2::system_clock::time_point now = system_clock::now();
    // get number of micrpseconds for the current second
    // (remainder after division into seconds)
    std::chrono::duration<int64_t, std::micro> ms = duration_cast<microseconds>(now.time_since_epoch())%1000000;
    // convert to std::time_t in order to convert to std::tm (broken time)
    time_t timer_ = system_clock::to_time_t(now);
    // convert to broken time
    std::tm bt = *std::localtime(&timer_);
	// format the string using stringstream
    std::ostringstream oss;
    oss << std::put_time(&bt, "%H:%M:%S"); // HH:MM:SS
    oss << '.' << std::setfill('0') << std::setw(6) << ms.count();
    return oss.str();
}

// class which gets us time elapsed between an operation/function/code block
class timer{
private:
    std::chrono::time_point<std::chrono::system_clock> start;
    std::chrono::time_point<std::chrono::system_clock> stop;
public:
	// record the time stamp for start
    void start_timer(){
        start = std::chrono::system_clock::now();
    }

	// record the time stamp for stop 
    void stop_timer(){
        stop = std::chrono::system_clock::now();
    }

	// gives the time elapsed
    int getTime(){
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop-start).count();
        return duration;
    }
};

//using in built functions to get a random real time simulation time
double exponential_mimic(double lambda){
	// using built-in generator, random_device and distributor to get a time	
	uniform_real_distribution<double> distribution(pow(lambda,-1));
	random_device randomdevice;
	default_random_engine generate(randomdevice());
	double distribute = distribution(generate);
	return distribute;
}


// interface class
class Snapshot{
public:
	virtual void update(int v,int id) = 0;
	virtual vector<int> scan() = 0;
};

// stamped value class, implemented based on the textbook
class StampedValue{
public:
    long stamp;
    int value;
	int size;
    // constructors
    StampedValue() noexcept {
    }

    StampedValue(int init){
        stamp = 0;
        value = init;
    }

    StampedValue(long stamp_,int value_){
        stamp = stamp_;
        value = value_;
    }

	bool check_equality(StampedValue res){
		return res.stamp == stamp and res.value == value;
	}

    // static function which returns max stamped value
    static StampedValue max(StampedValue&x,StampedValue&y){
        return x.stamp > y.stamp ? x:y;
    }
};

// class stamped snap
class StampedSnap{
public:
	long stamp;
	int value;
	int* snap;
	int size;

	// constructors
	StampedSnap() noexcept{
		
	}
	StampedSnap(int value_){
		stamp = 0;
		value = value_;
	}
	StampedSnap(long label,int value_,int size_,int* snap_){
		stamp = label;
		value = value_;
		size = size_;
		snap = new int[size];
		for(int i = 0 ; i < size ; i++){
			snap[i] = snap_[i];
		}
	}
};

// Obstruction free snap shot
class ObstructionFreeSnapShot : public Snapshot{
private:
	atomic<StampedValue>* a_table;
	int size;
public:
	// constructor
	ObstructionFreeSnapShot(int capacity,int init){
		size = capacity;
		a_table = new atomic<StampedValue>[capacity];
		for(int i = 0 ; i < capacity ; i++){
			StampedValue s(init);
			a_table[i] = s;
		}
	}

	// update function
	void update(int value,int id){
		StampedValue oldValue = a_table[id];
		StampedValue newValue(oldValue.stamp+1,value);
		a_table[id] = newValue;
	}

	// collect function
	vector<StampedValue> collect(){
		vector<StampedValue> copy(size);
		for(int i = 0 ; i < size ; i++){
			copy[i] = a_table[i];
		}
		return copy;
	}

	// scan function
	vector<int> scan(){
		vector<StampedValue> oldCopy,newCopy;
		oldCopy = collect();
		collect_label_of: while(true){
			newCopy = collect();

			bool are_equal = true;
			for(int i = 0 ; i < oldCopy.size() ; i++){
				are_equal = are_equal and oldCopy[i].check_equality(newCopy[i]);
				if(are_equal) continue;
				else break;
			}

			if(not are_equal){
				oldCopy = newCopy;
				goto collect_label_of;
			}

			vector<int> result(size);
			for(int j = 0 ; j < size ; j++){
				result[j] = newCopy[j].value;
			}
			return result;		
		}
	}
};

// wait free snap shot class
class WaitFreeSnapShot : public Snapshot{
private:
	atomic<StampedSnap>* a_table;
	int size;
public:
	// constructor
	WaitFreeSnapShot(int capacity,int init){
		size = capacity;
		a_table = new atomic<StampedSnap>[capacity];
		for(int i = 0 ; i < capacity ; i++){
			StampedSnap s(init);
			a_table[i] = s;
		}
	}

	// collect function
	vector<StampedSnap> collect(){
		vector<StampedSnap> copy(size);
		for(int i = 0 ; i < size ; i++){
			copy[i] = a_table[i];
		}
		return copy;
	}

	// update function
	void update(int value,int id){
		vector<int> snap = scan();
		StampedSnap oldValue = a_table[id];
		int* snap_ = new int[snap.size()];
		for(int i = 0 ; i < snap.size() ; i++){
			snap_[i] = snap[i];
		}
		StampedSnap newValue(oldValue.stamp+1,value,size,snap_);
		a_table[id] = newValue;
	}

	// scan function
	vector<int> scan(){
		vector<StampedSnap> oldCopy,newCopy;
		vector<bool> moved(size,false);
		oldCopy = collect();

		collect_label_wf : while(true){
			newCopy = collect();
			for(int j = 0 ; j < size ; j++){
				if(oldCopy[j].stamp != newCopy[j].stamp){
					if(moved[j]){
						vector<int> res(newCopy[j].size);
						for(int i = 0 ; i < size ; i++){
							res[i] = newCopy[j].snap[i];
						}
						return res;
					} else {
						moved[j] = true;
						oldCopy = newCopy;
						goto collect_label_wf;
					}
				}
			}
			vector<int> result(size);
			for(int j = 0 ; j < size ; j++){
				result[j] = newCopy[j].value;
			}
			return result;
		}
	}
};

// register of type Snapshot(interface class) works with multiple derived class virtually
Snapshot* register_;

// utility function rest
void reset(){
	term = false;
	// clear the log containers if it contains any data
	writerLog.clear();
	snapperLog.clear();
	// set times to 0
	total_time_taken = 0;
	worst_case_time = 0;
}

// utility function to execute as described in problem statement
void execute(){

	term = false;

	// create and assign threads
	thread* writerThreads = new thread[nw];
	thread* collectorThreads = new thread[ns];

	for(int i = 0 ; i < nw ; i++){
		writerThreads[i] = thread(writer,i);
	}
	for(int i = 0 ; i < ns ; i++){
		collectorThreads[i] = thread(snapshot,i);
	}

	// join all the collector threads
	for(int i = 0 ; i < ns ; i++){
		collectorThreads[i].join();
	}
	// flip the atomic boolean
	term = true;
	// join all the writer threads
	for(int i = 0 ; i < nw ; i++){
		writerThreads[i].join();
	}

	// log out the writer log
	cout<<"Writer log : \n";
	for(auto itr = writerLog.begin() ; itr != writerLog.end() ; itr++){
		for(string log : itr->second){
			cout<<log<<itr->first<<endl;
		}
	}

	// log out the snapper log
	cout<<"Snapper log : \n";
	for(auto itr = snapperLog.begin() ; itr != snapperLog.end() ; itr++){
		for(pair<int,vector<int>> pair_ : itr->second){
			cout<<"thread Id - "<<pair_.first<<endl;
			for(int i = 0 ; i < pair_.second.size() ; i++){
				cout<<"L"<<i<<"-"<<pair_.second[i]<<" ";
			}
			cout<<endl<<"recorded at "<<itr->first<<endl;
		}
	}
}


// main thread
int main(){

	// fetch the input file with given name
	string file_name = "inp-params.txt";
	ifstream input_file(file_name,ios::in);

	// if the file is openable, then open, read and process
	if(input_file.is_open()){
		input_file>>nw>>ns>>M>>uw>>us>>k;

		reset();
		// create a new ObstructionFreeSnapShot
		register_ = new ObstructionFreeSnapShot(M,0);
		cout<<"Obstruction free snapshot register logs : \n";
		// start a timer and execute the thread functions
		timer* timer_ = new timer;
		timer_->start_timer();
		execute();
		timer_->stop_timer();
		cout<<"Time taken for obs-free register -> "<<timer_->getTime()<<" microseconds"<<endl;
		delete timer_;
		// delete register_;

		double average_time = (double)total_time_taken/(k*ns);
		cout<<"Average time taken for the snapshot : "<<average_time<<endl;
		cout<<"Worst case time taken for the snapshot : "<<worst_case_time<<endl;
		cout<<endl<<endl;
		cout<<"Wait free snapshot register logs : \n";

		// create a new WaitFreeSnapShot
		register_ = new WaitFreeSnapShot(M,0);
		reset();

		// start a timer and execute the thread functions
		timer* timer__ = new timer;
		timer__->start_timer();
		execute();
		timer__->stop_timer();
		cout<<"Time taken for wait-free register -> "<<timer__->getTime()<<" microseconds"<<endl;
		delete register_;

		average_time = (double)total_time_taken/(k*ns);
		cout<<"Average time taken for the snapshot : "<<average_time<<endl;
		cout<<"Worst case time taken for the snapshot : "<<worst_case_time<<endl;

	// else print an error message 
	} else {
		cout<<"[Error] Loading input file"<<endl;
	}
}

// writer thread function as described in the problem statement
void writer(int id){
	// value variable
	int value;
	// repeat the process until the atomic bool variable is changed from the parent thread
	while(!term){
		// generate a random value
		value = random()%(20*M);
		// get a random location to update the value
		int location = random()%M;
		// create a log msg
		string log_msg = "Thread t"+to_string(id)+" write of "+to_string(value)+" on location "+to_string(location)+" at ";
		// update the register
		register_->update(value,location);
		// storing to log(printing in between processes makes the log inconsistent, this is much better)
		writer_lock.lock();
		writerLog[time_in_HH_MM_SS_MMM()].emplace_back(log_msg);
		writer_lock.unlock();
		// mimic the time which is exponenitally distributed
		sleep(exponential_mimic(uw));
	}
}

// snapshot thread function as described in the problem statement
void snapshot(int id){
	// iterator
	int i = 0;

	vector<int> time_taken;
	// repeat K times
	while(i < k){
		// record the snapshot from the scan method of the register
		timer *Timer = new timer;
		Timer->start_timer();
		vector<int> snapshot_ = register_->scan();
		Timer->stop_timer();
		// storing to log(printing in between processes makes the log inconsistent, this is much better)
		snapper_lock.lock();
		time_taken.emplace_back(Timer->getTime());
		snapperLog[time_in_HH_MM_SS_MMM()].emplace_back(make_pair(id,snapshot_));
		snapper_lock.unlock();
		delete Timer;
		// mimic the time which is exponenitally distributed
		sleep(exponential_mimic(us));
		// incrementer
		i++;
	}

	int total_time = std::accumulate(time_taken.begin(),time_taken.end(),0);
	total_time_taken += total_time;
	int max_element = -1;
	for(int element : time_taken){
		if(max_element < element) max_element = element;
	}
	worst_case_time = max_element;
}