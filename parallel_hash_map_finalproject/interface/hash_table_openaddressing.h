#include "hash_table_interface.h"
#include <iostream>
#include <string>
#include <vector>
using namespace std;

class node{
public:
    std::string key;
    std::string value;
    
    node(){

    }

    node(std::string key_,std::string value_){
        key = key_;
        value = value_;
    }
};

class hash_table_openaddr_interface : public hash_table{
public:
    std::vector<node*> hash_table_;

    virtual pair<bool,int> utility_search_insert(string key)=0;
    
    // virtual pair<bool,int> utility_search_delete(string key)=0;
    virtual pair<bool,int> utility_search(string key)=0;
    // virtual pair<node*,vector<node>*> utility_search(string key) = 0;
};