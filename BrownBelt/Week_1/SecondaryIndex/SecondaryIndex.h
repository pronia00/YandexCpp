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
  
  bool operator==(const Record& rv) const {
    return (
      make_tuple(id, title, user, timestamp, karma) ==
      make_tuple(rv.id, rv.title, rv.user, rv.timestamp, rv.karma)
    );
  }
};

struct HashRecord {
  hash<string> string_hash;
  const size_t coef = 2'953'737;

  size_t operator() (const Record& rec) const {
    return string_hash(rec.id) * coef;
  }
};
class Database {
public:
    // insert elements if <id> does not exist
    bool Put(const Record& record);
    //return ponter to Record by <id> in O(1)
    const Record* GetById(const string& id) const;
    // erase if <id> does not exist
    bool Erase(const string& id);

    template <typename Container>
    class Range {
       public: 
        using const_iter = typename Container::const_iterator;

        Range(const Container& cont, const int low, const int high) 
          : _begin(cont.lower_bound(low))
          , _end(cont.upper_bound(high))
        {}

        Range(const Container& cont, const string& low) 
          : _begin(cont.lower_bound(low))
          , _end(cont.upper_bound(low))
        {}

        const_iter begin() const { return _begin; }
        const_iter end() const   { return _end; }

      private:
        const_iter _begin;
        const_iter _end;
    };

    template <typename Callback, typename Container> 
    void RangeBy(const int low, const int high, const Container& data, Callback callback) const {
      for (auto& cur : Range<Container>(data, low, high)) {
            if (!callback(*cur.second)) break;
        }
    }
    // count elements in range [low, high]
    template <typename Callback>
    void RangeByTimestamp(int low, int high, Callback callback) const {
        RangeBy(low, high, timestamp_data, callback);
    }
    // count elements in range [low, high]
    template <typename Callback>
    void RangeByKarma(int low, int high, Callback callback) const {
        RangeBy(low, high, karma_data, callback);
    }
    // count elements with equal [user, user]
    template <typename Callback>
    void AllByUser(const string& user, Callback callback) const {
        for (auto& cur : Range<Index<string>>(user_data, user)) {
            if (!callback(*cur.second)) break;
        }
    }

private: 
    using data_iter = typename list<Record>::iterator;
    template <typename Type>
    using Index = multimap<Type, data_iter>;

    struct SecondaryIndexElement {
      data_iter list_iter;
      typename Index<int>::iterator ts_iter; 
      typename Index<int>::iterator karma_iter;
      typename Index<string>::iterator user_iter;
    };
    
    //main data storage 
    list<Record> data;

    // O(1) access to elements by index
    unordered_map<string, SecondaryIndexElement, hash<string>> id_data;

    //Secondary Index Containers
    Index<int> timestamp_data;
    Index<int> karma_data;
    Index<string> user_data;
};
