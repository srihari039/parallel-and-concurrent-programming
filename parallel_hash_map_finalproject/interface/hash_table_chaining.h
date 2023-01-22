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

class hash_table_chaining_interface : public hash_table{
public:
    std::vector<pair<int,vector<node>>> hash_table_;
    virtual pair<node*,vector<node>*> utility_search(string key,int &position) = 0;
    virtual pair<node*,vector<node>*> utility_search(string key) = 0;
};