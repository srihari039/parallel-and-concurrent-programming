#include "../interface/hash_table_openaddressing.h"
#include <mutex>

std::mutex log_lock;

class hash_table_openaddr : public hash_table_openaddr_interface{
protected:
	int init_length = 10001;
	int present_length = 0;
	int hash_a;
	int hash_b;

	void resize_vector(size_t size){
		this->hash_table_.resize(size);
	}

	int hash_function(string key){
		long hasher = 0;
		for(char ch : key){
			hasher += ch * abs(ch-'a');
		}
		long hash_value = hash_a*hasher+hash_b;
		hash_value %= large_prime;
        
		return hash_value % present_length;
	}

	void triggerSeed(){
		// srand(time(NULL));
		hash_a = rand()%large_prime;
		hash_b = rand()%large_prime;
	}

	// vector<node>* get_chain(string key){
	// 	int hash_value = hash_function(key);
	// 	vector<node>* chain = &hash_table_[hash_value].second;
	// 	return chain;
	// }

	// node* low_level_search(vector<node>* chain,string key){
	// 	for(node &item : *chain){
	// 		if(item.key == key){
	// 			return &item;
	// 		}
	// 	}
	// 	return nullptr;
	// }

	// node* low_level_search(vector<node>* chain,string key,int& position){
	// 	for(node &item : *chain){
	// 		if(item.key == key){
	// 			return &item;
	// 		}
	// 		position++;
	// 	}
	// 	return nullptr;
	// }

	pair<bool,int> utility_search_insert(string key){

        int hash_value = hash_function(key);
        bool value=false;

        while(hash_table_[hash_value]!=NULL&&hash_table_[hash_value]->key!="-1")
        {
            if(hash_table_[hash_value]->key==key){
                value = true;
                break;
            }
            hash_value +=1;
            hash_value %=present_length;
        }
    
		return {value,hash_value};
	}

    // pair<bool,int> utility_search_delete(string key){

    //     int hash_value = hash_function(key);
    //     bool value=false;
    //     while(hash_table_[hash_value]!=NULL)
    //     {
    //         if(hash_table_[hash_value]->key==key){
    //             value = true;
    //             break;
    //         }

    //         hash_value +=1;
    //         hash_value %=present_length;
    //     }

	// 	return {value,hash_value};
	// }

    
	pair<bool,int> utility_search(string key){

        int hash_value = hash_function(key);
        bool value=false;
        int count =0;
        while(hash_table_[hash_value]!=NULL)
        {
            if(hash_table_[hash_value]->key==key){
                value = true;
                break;
            }

            if(count>present_length)    break;
            
            count+=1;
            hash_value +=1;
            hash_value %=present_length;
        }

		return {value,hash_value};
	}



	void log_item_count(){
		cout<<"[log] Item count in hashtable : "<<item_count<<endl;
	}

public:
	hash_table_openaddr(){
		present_length = init_length;
		resize_vector(init_length);
		triggerSeed();
	}

	hash_table_openaddr(int init_length_){
		init_length = init_length_;
		present_length = init_length;
		resize_vector(init_length);
        for(int i=0;i<present_length;i++)
        {
            hash_table_[i]=NULL;
        }
		triggerSeed();
	}

	int get_hash_value(string key){
		return hash_function(key);
	}

	void insert(string key,string value){

        pair<bool,int> pack = utility_search_insert(key);
        bool val = pack.first;
        int temp = pack.second;
     
        if(!val&&(hash_table_[temp]==nullptr||hash_table_[temp]->key=="-1"))
        {

            item_count++;
            node*item = new node(key,value);
            hash_table_[temp] = item;
            log_lock.lock();
			cout<<"[Log] (Key,value) inserted. key:"<<key<<" value:"<<value<<endl;
			log_lock.unlock();

           
        }
        else{

            log_lock.lock();
			cout<<"[Log] Key already exists! value overwritten. key:"<<key<<" value:"<<value<<endl;
			log_lock.unlock();
    
        }

		return;
	}

	bool delete_item(string key){

        pair<bool,int> pack = utility_search(key);
        bool val = pack.first;
        int temp = pack.second;

		if(!val){
			log_lock.lock();
			cout<<"[Log] Key not found to delete item. key:"<<key<<endl;
			log_lock.unlock();
		} else {
          
            node * item = new node("-1","-1");
            hash_table_[temp] = item;
            item_count--;
			log_lock.lock();
			cout<<"[Log] Key found and item deleted. key:"<<key<<endl;
			log_lock.unlock();
			
		}

		return val;
	}

	bool search(string key){

        pair<bool,int> pack = utility_search(key);
        bool val = pack.first;

        if(val){
            log_lock.lock();
			cout<<"[Log] Key found. key:"<<key<<endl;
			log_lock.unlock();
        }
        else{
            log_lock.lock();
		    cout<<"[Log] Key not found. key:"<<key<<endl;
		    log_lock.unlock();
        }

        return val;		
	}

	// std::string retrieve(string key){
	// 	pair<node*,vector<node>*> pack = utility_search(key);
	// 	node* item = pack.first;
	// 	if(item != nullptr){
	// 		return item->value;
	// 	} else {
	// 		return "C++:None";
	// 	}
	// }

	void check(){
		cout<<"Size - "<<hash_table_.size()<<endl;
	}

	void execute(string mode,string key,string value = ""){
		if(mode == "insert"){
			insert(key,value);
		} else if(mode == "delete"){
			delete_item(key);
		} else {
			search(key);
		}
	}
};