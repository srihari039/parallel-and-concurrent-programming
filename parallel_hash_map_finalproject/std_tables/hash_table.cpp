#include <bits/stdc++.h>
using namespace std;

class timer{
private:
    std::chrono::time_point<std::chrono::system_clock> start;
    std::chrono::time_point<std::chrono::system_clock> stop;
public:
    void start_timer(){
        start = std::chrono::system_clock::now();
    }

    void stop_timer(){
        stop = std::chrono::system_clock::now();
    }

    int getTime(){
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop-start).count();
        return duration;
    }
};

int main(){
	unordered_map<string,string> mp;

	string mode;
	int testcases;
	cin>>testcases;

	int searches = 0,inserts = 0,deletes = 0;
	int search_time = 0,insert_time = 0,delete_time = 0;

	timer t;
	t.start_timer();
	timer t_;
	while(testcases--){
		cin>>mode;
		if(mode == "insert"){
			string key,value;
			cin>>key>>value;
			t_.start_timer();
			mp[key] = value;
			cout<<"[Log] (Key,value) inserted. key:"<<key<<" value:"<<value<<endl;
			t_.stop_timer();
			inserts++;
			insert_time += t_.getTime();
		} else if(mode == "delete"){
			string key;
			cin>>key;
			t_.start_timer();
			if(mp.find(key) == mp.end()){
				cout<<"[Log] Key not found to delete item. key:"<<key<<endl;
			} else {
				mp.erase(key);
				cout<<"[Log] Key found and item deleted. key:"<<key<<endl;
			}
			t_.stop_timer();
			deletes++;
			delete_time += t_.getTime();
		} else if(mode == "search"){
			string key;
			cin>>key;
			t_.start_timer();
			bool found = mp.find(key) != mp.end();

			if(not found){
				cout<<"[Log] Key not found. key:"<<key<<endl;
			} else {
				cout<<"[Log] Key found. key:"<<key<<endl;
			}

			t_.stop_timer();
			searches++;
			search_time += t_.getTime();
		}
	}

	t.stop_timer();
	cout<<endl<<endl;
	cout<<"Time taken for entire operations("<<searches+deletes+inserts<<") : "<<t.getTime()<<" us"<<endl;

	cout<<"Time taken for inserts("<<inserts<<") : "<<insert_time<<" us"<<endl;
	cout<<"Avg time taken for insert operation "<<insert_time/inserts<<" us"<<endl;

	cout<<"Time taken for searches("<<searches<<") : "<<search_time<<" us"<<endl;
	cout<<"Avg time taken for searche operation "<<search_time/searches<<" us"<<endl;

	cout<<"Time taken for deletes("<<deletes<<") : "<<delete_time<<" us"<<endl;
	cout<<"Avg time taken for delete operation "<<delete_time/deletes<<" us"<<endl;

	return 0;
}