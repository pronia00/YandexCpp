#pragma once

#include <map>
#include <string>
#include <unordered_map>
#include <list>

using namespace std;

struct Record {  
  string id;
  string title;
  string user;
  int timestamp;
  int karma;

  Record() {}
  Record(string _id, string _title, string _user, int _ts, int _karma) 
      : id(_id)
      , title(_title)
      , user(_user)
      , timestamp(_ts)
      , karma(_karma)
  {};
  
  bool operator==(const Record& other) const {
    return 
      make_tuple(id, title, user, timestamp, karma) ==
      make_tuple(other.id, other.title, other.user, other.timestamp, other.karma);
  }
};

struct HashRecord {
  hash<string> string_hash;
  const size_t coef = 2'953'737;

  size_t operator() (const Record& rec) const {
    return string_hash(rec.id) * coef;
  }
};

// Реализуйте этот класс
class Database {
public:
    using data_iter = list<Record>::iterator;

    // insert elements if <id> does not exist
    bool Put(const Record& record);

    //return ponter to Record by <id> in O(1)
    const Record* GetById(const string& id) const;

    // erase if <id> does not exist
    bool Erase(const string& id);

    // count elements in range [low, high]
    template <typename Callback, typename CheckMax>
    void RangeByTimestamp(int low, int high, Callback callback) const {
        auto range_begin = timestamp_data.lower_bound(low);
        auto range_end = timestamp_data.end();

        for (auto cur = range_begin; cur != range_end; cur++) {
            if (cur->second->timestamp > high) break;
            if (!callback(*(cur->second))) break;
        }
    }
    // count elements in range [low, high]
    template <typename Callback>
    void RangeByKarma(int low, int high, Callback callback) const {
        auto range_begin = karma_data.lower_bound(low);
        auto range_end = karma_data.end();

        for (auto cur = range_begin; cur != range_end; ++cur) {
            if (cur->second->karma > high) break;
            if (!callback(*(cur->second))) break;
        }
    }
    // count elements with equal <user>
    template <typename Callback>
    void AllByUser(const string& user, Callback callback) const {
        auto range_begin = user_data.lower_bound(user);
        auto range_end = user_data.end();
        
        for (auto cur = range_begin; cur != range_end; ++cur) {
            if (cur->second->user > user)  break;
            if (!callback(*(cur->second))) break;
        }
    }

private: 
    struct SecondaryIndexElement {
      data_iter list_iter;
      multimap<int, data_iter>::iterator ts_iter; 
      multimap<int, data_iter>::iterator karma_iter;
      multimap<string, data_iter>::iterator user_iter;
    };
    
    //main data storage 
    list<Record> data;

    // O(1) access to elements by index
    unordered_map<string, SecondaryIndexElement, hash<string>> id_data;
    
    //Secondary Index Containers
    multimap<int, data_iter> timestamp_data;
    multimap<int, data_iter> karma_data;
    multimap<string, data_iter> user_data;
};
