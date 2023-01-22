#include "../hash_tables/hash_table_chaining.h"
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <set>
#include <semaphore.h>

class ht_submap : public hash_table_chaining{
private:
	int num_threads_supported;
	thread* worker_threads;
	hash_table_chaining* sub_maps;
	queue<tuple<string,string,string>>* sub_map_queue;
	sem_t* semaphore_signal_main_to_thread;
	bool* semaphore_flags;
	bool* running;
	timer insert_timer;
	timer search_timer;
	timer delete_timer;
	int executes = 0;
	set<int> ids;
public:
	ht_submap(){
		num_threads_supported = std::thread::hardware_concurrency();
		worker_threads = new thread[num_threads_supported];
		sub_maps = new hash_table_chaining[num_threads_supported];
		sub_map_queue = new queue<tuple<string,string,string>>[num_threads_supported];
		semaphore_signal_main_to_thread = new sem_t[num_threads_supported];
		semaphore_flags = new bool[num_threads_supported];
		running = new bool[num_threads_supported];

		for(int i = 0 ; i < num_threads_supported ; i++){
			semaphore_flags[i] = true;
			sem_init(&semaphore_signal_main_to_thread[i],1,0);
		}
	}

	int simple_hash(string key){
		int hash = 0;
		for(char ch : key){
			hash += ch;
		}
		return hash%num_threads_supported;
	}

	void handle_thread(int id){
		sem_wait(&semaphore_signal_main_to_thread[id]);
		if(sub_map_queue[id].size()){
			tuple<string,string,string> operation = sub_map_queue[id].front();
			sub_map_queue[id].pop();
			if(get<0>(operation) == "insert"){
				sub_maps[id].insert(get<1>(operation),get<2>(operation));
			} else if(get<0>(operation) == "delete"){
				sub_maps[id].delete_item(get<1>(operation));
			} else if(get<0>(operation) == "search"){
				sub_maps[id].search(get<1>(operation));
			}
			handle_thread(id);
		} else if(not semaphore_flags[id]){
			return;
		}
	}

	void execute(string mode,string key,string value=""){
		int id = simple_hash(key);
		ids.insert(id);
		if(not running[id]){
			worker_threads[id] = thread(&ht_submap::handle_thread,this,id);
			running[id] = true;
		}
		sub_map_queue[id].push(make_tuple(mode,key,value));
		sem_post(&semaphore_signal_main_to_thread[id]);
	}

	~ht_submap(){
		cout<<endl;
		for(int i = 0 ; i < num_threads_supported ; i++){
			semaphore_flags[i] = false;
			sem_post(&semaphore_signal_main_to_thread[i]);
			worker_threads[i].join();
		}
		for(int i = 0 ; i < num_threads_supported ; i++){
			sem_destroy(&semaphore_signal_main_to_thread[i]);
		}
		cout<<"All worker threads joined"<<endl;
	}
};