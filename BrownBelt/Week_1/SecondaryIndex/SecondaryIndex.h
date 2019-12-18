#pragma once

#include <map>
#include <string>
#include <unordered_map>
#include <list>
#include <iterator>

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

    // insert elements if <id> does not exist
    bool Put(const Record& record);

    //return ponter to Record by <id> in O(1)
    const Record* GetById(const string& id) const;

    // erase if <id> does not exist
    bool Erase(const string& id);

    template <typename Container, typename Data>
    class RangeBy {
       public: 
        using const_iter = typename Container::const_iterator;
        RangeBy(const Container& cont, const Data& low) 
          : _begin(cont.lower_bound(low))
          , _end(cont.end())
        {}
        const_iter begin() const { return _begin; }
        const_iter end() const   { return _end; }
      private:
        const_iter _begin;
        const_iter _end;
    };

    // count elements in range [low, high]
    template <typename Callback>
    void RangeByTimestamp(int low, int high, Callback callback) const {
        for (auto& cur : RangeBy<i_SI, int>(timestamp_data, low)) {
            if (cur.second->timestamp > high) break;
            if (!callback(cur.second)) break;
        }
    }
    // count elements in range [low, high]
    template <typename Callback>
    void RangeByKarma(int low, int high, Callback callback) const {
          for (auto& cur : RangeBy<i_SI, int>(karma_data, low)) {
            if (cur.second->karma > high) break;
            if (!callback(*(cur.second))) break;
        }
    }
    // count elements with equal <user>
    template <typename Callback>
    void AllByUser(const string& user, Callback callback) const {
        for (auto& cur : RangeBy<s_SI, string>(user_data, user)) {
            if (cur.second->user > user)  break;
            if (!callback(*(cur.second))) break;
        }
    }

private: 
    using data_iter = typename list<Record>::iterator;
    using i_SI = multimap<int, data_iter>;
    using s_SI = multimap<string, data_iter>;

    struct SecondaryIndexElement {
      data_iter list_iter;
      i_SI::iterator ts_iter; 
      i_SI::iterator karma_iter;
      s_SI::iterator user_iter;
    };
    
    //main data storage 
    list<Record> data;

    // O(1) access to elements by index
    unordered_map<string, SecondaryIndexElement, hash<string>> id_data;

    //Secondary Index Containers
    i_SI timestamp_data;
    i_SI karma_data;
    s_SI user_data;
};
