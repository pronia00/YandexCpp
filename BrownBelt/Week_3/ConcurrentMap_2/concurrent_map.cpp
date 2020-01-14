#include "test_runner.h"
#include "profiler.h"

#include <future>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <utility>
#include <algorithm>
#include <random>
#include <functional>
using namespace std;

template <typename T>
T Abs(T x) {
  return x < 0 ? -x : x;
}

auto Lock(mutex& m) {
  return lock_guard<mutex>{m};
}

template <
    typename K, 
    typename V, 
    typename Hasher = hash<K>
> class ConcurrentMap {
public:
  // static_assert(
  //   is_convertible_v<K, uint64_t>,
  //   "ConcurrentMap supports only integer keys"
  // );
  // struct _hasher {
  //   size_t operator()(const K& k) const {
  //     const hash<K> hasher;
  //     const size_t coef = 514'229;

  //     return coef * hasher(k);
  //   };
  // };
  struct Access {
    lock_guard<mutex> guard;
    V& ref_to_value;

    Access(const K& key, pair<mutex, unordered_map<K, V, Hasher>>& bucket_content)
      : guard(bucket_content.first)
      , ref_to_value(bucket_content.second[key])
    {}
    Access(K&& key, pair<mutex, unordered_map<K, V, Hasher>>& bucket_content)
      : guard(bucket_content.first)
      , ref_to_value(bucket_content.second[move(key)])
    {}
  };

  struct ReadAccess {
    lock_guard<mutex> guard;
    const V& ref_to_value;

    ReadAccess(const K& key, pair<mutex, unordered_map<K, V, Hasher>>& bucket_content)
      : guard(bucket_content.first)
      , ref_to_value(bucket_content.second.at(key))
    {}
  };

  explicit ConcurrentMap(size_t bucket_count)
    : data(bucket_count)
  {
  }

  Access operator[](const K& key) {
    auto& bucket = data[_index(key)];
    return {key, bucket};
  }

  Access operator[](K&& key) {
    auto& bucket = data[_index(key)];
    return {key, bucket};
  }

  ReadAccess At(const K& key) const {
    return {key, data[_index(key)] };
  }

  bool Has(const K& key) const {
    return 0 != data[_index(key)].second.count(key);
  }
  
  auto BuildOrdinaryMap() const {
    unordered_map<K, V, Hasher> result;
    for (auto& [mtx, mapping] : data) {
      auto g = Lock(mtx);
      result.insert(begin(mapping), end(mapping));
    }
    return result;
  }

private:
  Hasher _hasher {};
  mutable vector<pair<mutex, unordered_map<K, V, Hasher>>> data;

  size_t _index(const K& key) const noexcept {
    return _hasher(key) % data.size();
  }
};


void BaseTest() {
  ConcurrentMap<int, int>cm(4);
  size_t result = 0;

  for (int i = 0; i < 10; ++i) {
    cm[i].ref_to_value = i;
  }
  for (int i = 0; i < 10; ++i) {
   result += cm[i].ref_to_value; // 45
  }
  ASSERT(result == 45);  

  result = 0;
  for (int i = 0; i < 10; ++i) {
   result += cm.At(i).ref_to_value; // 45
  }
  ASSERT(result == 45);  

  result = 0;
  try { cm.At(999);}
  catch(const std::out_of_range& e) { result++; }

  ASSERT(result == 1);

  bool res = false;
  for (int i = 0; i < 10; ++i) {
    res = cm.Has(i);
    ASSERT(res == true);  
  }

  for (int i = 0; i < 10; ++i) {
    res = cm.Has(100 + i);
    ASSERT(res == false);  
  }

}

void StringTest() {
  ConcurrentMap<string, string> cm(4);
  vector<string> keys { "one_k", "two_k", "three_k", "four_k", "five_k"};
  vector<string> values { "one_v", "two_v", "three_v", "four_v", "five_v"};

  for (size_t i = 0; i < 5; ++i) {
    cm[keys[i]].ref_to_value = values[i];
  }

  bool result = false;

  for (size_t i = 0; i < 5; ++i) {
   result = 
        (cm[keys[i]].ref_to_value == values[i]);

  }
  ASSERT(true == result);

}
void RunConcurrentUpdates(
    ConcurrentMap<int, int>& cm, size_t thread_count, int key_count
) {
  auto kernel = [&cm, key_count](int seed) {
    vector<int> updates(key_count);
    iota(begin(updates), end(updates), -key_count / 2);
    shuffle(begin(updates), end(updates), default_random_engine(seed));

    for (int i = 0; i < 2; ++i) {
      for (auto key : updates) {
        cm[key].ref_to_value++;
      }
    }
  };

  vector<future<void>> futures;
  for (size_t i = 0; i < thread_count; ++i) {
    futures.push_back(async(kernel, i));
  }
}

void TestConcurrentUpdate() {
  const size_t thread_count = 3;
  const size_t key_count = 50000;

  ConcurrentMap<int, int> cm(thread_count);
  RunConcurrentUpdates(cm, thread_count, key_count);

  const auto result = std::as_const(cm).BuildOrdinaryMap();
  ASSERT_EQUAL(result.size(), key_count);
  for (auto& [k, v] : result) {
    AssertEqual(v, 6, "Key = " + to_string(k));
  }
}

void TestReadAndWrite() {
  ConcurrentMap<size_t, string> cm(5);

  auto updater = [&cm] {
    for (size_t i = 0; i < 50000; ++i) {
      cm[i].ref_to_value += 'a';
    }
  };
  auto reader = [&cm] {
    vector<string> result(50000);
    for (size_t i = 0; i < result.size(); ++i) {
      result[i] = cm[i].ref_to_value;
    }
    return result;
  };

  auto u1 = async(updater);
  auto r1 = async(reader);
  auto u2 = async(updater);
  auto r2 = async(reader);

  u1.get();
  u2.get();

  for (auto f : {&r1, &r2}) {
    auto result = f->get();
    ASSERT(all_of(result.begin(), result.end(), [](const string& s) {
      return s.empty() || s == "a" || s == "aa";
    }));
  }
}

void TestSpeedup() {
  {
    ConcurrentMap<int, int> single_lock(1);

    LOG_DURATION("Single lock");
    RunConcurrentUpdates(single_lock, 4, 50000);
  }
  {
    ConcurrentMap<int, int> many_locks(100);

    LOG_DURATION("100 locks");
    RunConcurrentUpdates(many_locks, 4, 50000);
  }
}

void TestConstAccess() {
  const unordered_map<int, string> expected = {
    {1, "one"},
    {2, "two"},
    {3, "three"},
    {31, "thirty one"},
    {127, "one hundred and twenty seven"},
    {1598, "fifteen hundred and ninety eight"}
  };

  const ConcurrentMap<int, string> cm = [&expected] {
    ConcurrentMap<int, string> result(3);
    for (const auto& [k, v] : expected) {
      result[k].ref_to_value = v;
    }
    return result;
  }();

  vector<future<string>> futures;
  for (int i = 0; i < 10; ++i) {
    futures.push_back(async([&cm, i] {
      try {
        return cm.At(i).ref_to_value;
      } catch (exception&) {
        return string();
      }
    }));
  }
  futures.clear();

  ASSERT_EQUAL(cm.BuildOrdinaryMap(), expected);
}

void TestStringKeys() {
  const unordered_map<string, string> expected = {
    {"one", "ONE"},
    {"two", "TWO"},
    {"three", "THREE"},
    {"thirty one", "THIRTY ONE"},
  };

  const ConcurrentMap<string, string> cm = [&expected] {
    ConcurrentMap<string, string> result(2);
    for (const auto& [k, v] : expected) {
      result[k].ref_to_value = v;
    }
    return result;
  }();

  ASSERT_EQUAL(cm.BuildOrdinaryMap(), expected);
}

struct Point {
  int x, y;
};

struct PointHash {
  size_t operator()(Point p) const {
    std::hash<int> h;
    return h(p.x) * 3571 + h(p.y);
  }
};

bool operator==(Point lhs, Point rhs) {
  return lhs.x == rhs.x && lhs.y == rhs.y;
}

void TestUserType() {
  ConcurrentMap<Point, size_t, PointHash> point_weight(5);
  
  vector<future<void>> futures;
  for (int i = 0; i < 1000; ++i) {
    futures.push_back(async([&point_weight, i] {
      point_weight[Point{i, i}].ref_to_value = i;
    }));
  }

  futures.clear();

  for (int i = 0; i < 1000; ++i) {
    ASSERT_EQUAL(point_weight.At(Point{i, i}).ref_to_value, i);
  }

  const auto weights = point_weight.BuildOrdinaryMap();
  for (int i = 0; i < 1000; ++i) {
    ASSERT_EQUAL(weights.at(Point{i, i}), i);
  }
}

void TestHas() {
  ConcurrentMap<int, int> cm(2);
  cm[1].ref_to_value = 100;
  cm[2].ref_to_value = 200;

  const auto& const_map = std::as_const(cm);
  ASSERT(const_map.Has(1));
  ASSERT(const_map.Has(2));
  ASSERT(!const_map.Has(3));
}

// int main() {
//   TestRunner tr;
//   RUN_TEST(tr, TestConcurrentUpdate);
//   RUN_TEST(tr, TestReadAndWrite);
//   RUN_TEST(tr, TestSpeedup);
//   RUN_TEST(tr, TestConstAccess);
//   RUN_TEST(tr, TestStringKeys);
//   RUN_TEST(tr, TestUserType);
//   RUN_TEST(tr, TestHas);
// }

int main() {
  TestRunner tr;
  RUN_TEST(tr, BaseTest);
  RUN_TEST(tr, StringTest);
  RUN_TEST(tr, TestStringKeys);
  RUN_TEST(tr, TestConcurrentUpdate);
  RUN_TEST(tr, TestReadAndWrite);
  RUN_TEST(tr, TestSpeedup);
  RUN_TEST(tr, TestConstAccess);
  RUN_TEST(tr, TestHas);
  RUN_TEST(tr, TestUserType);
  
  
  return 0;
}