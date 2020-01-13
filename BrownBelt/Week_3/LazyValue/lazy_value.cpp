#include "test_runner.h"

#include <functional>
#include <string>
#include <optional>

using namespace std;

template <typename T>
class LazyValue {
public:
  explicit LazyValue(std::function<T()> init) 
    : _init_function(move(init)) 
    , _value(nullopt)
  {};

  bool HasValue() const noexcept {
      return _value.has_value();
  }
  const T& Get() const {
      if (!_value) {
          _value = _init_function();
      }
      return *_value;
  }

private:
    mutable optional<T> _value;
    function<T()> _init_function;
};

void IntTest() {
    LazyValue<int> lazy_int([] {return 42; });
    ASSERT(!lazy_int.HasValue());

    ASSERT_EQUAL(lazy_int.Get(), 42);
}

void UseExample() {
  const string big_string = "Giant amounts of memory";

  LazyValue<string> lazy_string([&big_string] { return big_string; });

  ASSERT(!lazy_string.HasValue());
  ASSERT_EQUAL(lazy_string.Get(), big_string);
  ASSERT_EQUAL(lazy_string.Get(), big_string);
}

void TestInitializerIsntCalled() {
  bool called = false;

  {
    LazyValue<int> lazy_int([&called] {
      called = true;
      return 0;
    });
  }
  ASSERT(!called);
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, IntTest);
  RUN_TEST(tr, UseExample);
  RUN_TEST(tr, TestInitializerIsntCalled);
  return 0;
}