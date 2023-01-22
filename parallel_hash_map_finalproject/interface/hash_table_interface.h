#include <iostream>
#include <string>
#include <chrono>
// #include <sempahore.h>

class hash_table{
protected:
    int large_prime = 1e9+7;
    int item_count = 0;
public:
    virtual void insert(std::string key,std::string value) = 0;
    virtual bool search(std::string key) = 0;
    virtual bool delete_item(std::string key) = 0;
};

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