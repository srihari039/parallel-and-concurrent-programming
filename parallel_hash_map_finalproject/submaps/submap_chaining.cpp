#include "sub_map.h"

int main(){
	ht_submap* hash_table = new ht_submap();

	timer t;
	int testcases;
	cin>>testcases;

	std::string mode;

	int searches = 0,inserts = 0,deletes = 0;
	int search_time = 0,insert_time = 0,delete_time = 0;

	t.start_timer();
	timer t_;
	while(testcases--){
		cin>>mode;
		if(mode == "insert"){
			string key,value;
			cin>>key>>value;
			// t_.start_timer();
			hash_table->execute(mode,key,value);
			// t_.stop_timer();
			inserts++;
			// insert_time += t_.getTime();
		} else if(mode == "delete"){
			string key;
			cin>>key;
			// t_.start_timer();
			hash_table->execute(mode,key);
			// t_.stop_timer();
			deletes++;
			// delete_time += t_.getTime();
		} else if(mode == "search"){
			string key;
			cin>>key;
			// t_.start_timer();
			hash_table->execute(mode,key);
			// t_.stop_timer();
			searches++;
			// search_time += t_.getTime();
		}
	}

	delete hash_table;
	t.stop_timer();
	cout<<endl<<endl;

	cout<<"Time taken for entire operations("<<searches+deletes+inserts<<") : "<<t.getTime()<<" us"<<endl;

	// cout<<"Time taken for inserts("<<inserts<<") : "<<insert_time<<" us"<<endl;
	// cout<<"Avg time taken for insert operation "<<insert_time/inserts<<" us"<<endl;

	// cout<<"Time taken for searches("<<searches<<") : "<<search_time<<" us"<<endl;
	// cout<<"Avg time taken for search operation "<<search_time/searches<<" us"<<endl;

	// cout<<"Time taken for deletes("<<deletes<<") : "<<delete_time<<" us"<<endl;
	// cout<<"Avg time taken for delete operation "<<delete_time/deletes<<" us"<<endl;

	return 0;
}