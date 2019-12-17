#include "test_runner.h"

#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <list>
#include <iterator>
#include <algorithm>

using namespace std;

struct Record {
  string id;
  string title;
  string user;
  int timestamp;
  int karma;

  Record(string _id, string _title, string _user, int _ts, int _karma) 
      : id(_id)
      , title(_title)
      , user(_user)
      , timestamp(_ts)
      , karma(_karma)
  {};
  void operator<<(ostream& os) {
    os << id << title << user << timestamp << karma;
  }
  bool operator==(const Record& other) const {
    return 
      make_tuple(id, title, user, timestamp, karma) ==
      make_tuple(other.id, other.title, other.user, other.timestamp, other.karma);
  }
};

struct HashRecord {
  hash<string> string_hash;
  hash<int> int_hash;
  const size_t coef = 2'953'737;

  size_t operator() (const Record& rec) const {
    // return ( 
    //     coef * coef * coef * coef * string_hash(rec.id) +
    //            coef * coef * coef * string_hash(rec.title) +
    //                   coef * coef * string_hash(rec.user) +
    //                          coef * int_hash(rec.timestamp) +
    //                                 int_hash(rec.karma)
    // );
    return string_hash(rec.id);
  }
};

struct SecondaryIndex {
    list<Record>::iterator list_iter;
    multimap<int, list<Record>::iterator>::iterator ts_iter; 
    multimap<int, list<Record>::iterator>::iterator karma_iter;
    multimap<string, list<Record>::iterator>::iterator user_iter;
};
// Реализуйте этот класс
class Database {
public:
  bool Put(const Record& record) {
    if (id_data.find(record.id) != id_data.end()) {
        return false;
    }
    SecondaryIndex index;
    //insert data to main storage 
    index.list_iter = data.insert(data.begin(), record);

    //insert data to secondary storages
 		index.ts_iter = timestamp_data.insert({ record.timestamp, index.list_iter });
    index.karma_iter = karma_data.insert({ record.karma, index.list_iter });
    index.user_iter = user_data.insert({record.user, index.list_iter});
  
    //move secondary index to hash_map for O(1) access by index
    id_data[record.id] = move(index);
    return true;
  }

  const Record* GetById(const string& id) const {
    auto res = id_data.find(id);
    return (res == id_data.end())
      ? nullptr
      : &(*(res->second.list_iter));

  }
  bool Erase(const string& id) {
    auto res = id_data.find(id);
    if (res == id_data.end()) {
      return false;
    }
    timestamp_data.erase(res->second.ts_iter);
    karma_data.erase(res->second.karma_iter);
    user_data.erase(res->second.user_iter);
    data.erase(res->second.list_iter);

    //delete other secondary index contatiners
    id_data.erase(res);

    return true;
  }

  template <typename Callback>
  void RangeByTimestamp(int low, int high, Callback callback) const {
      auto begin_iter = timestamp_data.lower_bound(low);
      auto cur = begin_iter;
      auto end_iter = timestamp_data.end();

      for (int i = low; i <= high, cur != end_iter; ++cur, ++i) {
        // if (callback() != true) {
        //   break;
        // } 
      }
  }

  template <typename Callback>
  void RangeByKarma(int low, int high, Callback callback) const {
    auto begin_iter = karma_data.lower_bound(low);
      auto cur = begin_iter;
      auto end_iter = karma_data.end();

      for (int i = low; i <= high, cur != end_iter; ++cur, ++i) {
        // if (callback() != true) {
        //   break;
        // } 
      }
  }

  template <typename Callback>
  void AllByUser(const string& user, Callback callback) const {
    auto range_begin = user_data.lower_bound(user);
    auto range_end = user_data.end();

    for (auto cur = range_begin; cur != range_end; ++cur) {
      if (callback(*(cur->second)) != true) {
        break;
      };
    }
  }

private: 
  list<Record> data;
  unordered_map<string, SecondaryIndex, hash<string>> id_data;
  multimap<const int, list<Record>::iterator> timestamp_data;
  multimap<const int, list<Record>::iterator> karma_data;
  multimap<const string, list<Record>::iterator> user_data;

};

void TestErase() {
  Database db;
  Record r1{"1", "title", "user", 1, 1};
  Record r2{"2", "title", "user", 1, 1};
  Record r3{"3", "title", "user", 1, 1};

  db.Put(r1);
  db.Put(r2);
  
  ASSERT(db.Erase(r1.id));
  ASSERT(db.Erase(r2.id));
  ASSERT(!db.Erase(r1.id));
  ASSERT(!db.Erase(r3.id));
}

void TestPut() {
  Database db;
  Record r1{"1", "title", "user", 1, 1};
  Record r2{"2", "title", "user", 1, 1};

  bool result = db.Put(r1);
  ASSERT(result);

  result = db.Put(r2);
  ASSERT(result);

  result = db.Put(r1);
  ASSERT(!result);
}

void TestGetById() {
  Database db;
  Record r1{"1", "title", "user", 1, 1};
  Record r2{"2", "title", "user", 1, 1};
  Record r3{"3", "title", "user", 1, 1};

  db.Put(r1);
  db.Put(r2);
  db.Put(r3);
  
  ASSERT(r1 == *(db.GetById("1")));
  ASSERT(r2 == *(db.GetById("2")));
  ASSERT(!(r1 == *(db.GetById("2"))));
}

void TestRecordEquality() {
  Record r1("one", "two", "tree", 1, 2);
  Record r2("one", "two", "tree", 1, 2);

  ASSERT(r1 == r2);

  Record r3("one", "two", "tree", 1, 2);
  Record r4("six", "five", "four", 6, 5);

  ASSERT(!(r3 == r4));
}

// void TestRangeBoundaries() {
//   const int good_karma = 1000;
//   const int bad_karma = -10;

//   Database db;
//   db.Put({"id1", "Hello there", "master", 1536107260, good_karma});
//   db.Put({"id2", "O>>-<", "general2", 1536107260, bad_karma});

//   int count = 0;
//   db.RangeByKarma(bad_karma, good_karma, [&count](const Record&) {
//     ++count;
//     return true;
//   });

//   ASSERT_EQUAL(2, count);
// }

void TestSameUser() {
  Database db;
  db.Put({"id1", "Don't sell", "master", 1536107260, 1000});
  db.Put({"id2", "Rethink life", "master", 1536107260, 2000});

  int count = 0;
  db.AllByUser("master", [&count](const Record&) {
    ++count;
    return true;
  });

  ASSERT_EQUAL(2, count);
}

// void TestReplacement() {
//   const string final_body = "Feeling sad";

//   Database db;
//   db.Put({"id", "Have a hand", "not-master", 1536107260, 10});
//   db.Erase("id");
//   db.Put({"id", final_body, "not-master", 1536107260, -10});

//   auto record = db.GetById("id");
//   ASSERT(record != nullptr);
//   ASSERT_EQUAL(final_body, record->title);
// }

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestRecordEquality);
  RUN_TEST(tr, TestPut);
  RUN_TEST(tr, TestGetById);
  RUN_TEST(tr, TestErase);
  RUN_TEST(tr, TestSameUser);
  //RUN_TEST(tr, TestRangeBoundaries);
  //RUN_TEST(tr, TestReplacement);
  return 0;
}
