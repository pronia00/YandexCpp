#include "test_runner.h"
#include "SecondaryIndex.h"

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

  Record r4("six", "five", "four", 6, 5);
  ASSERT(!(r2 == r4));
}

void TestRangeBoundaries() {
  const int good_karma = 1000;
  const int bad_karma = -10;

  Database db;
  db.Put({"id1", "Hello there", "master", 1536107260, good_karma});
  db.Put({"id2", "O>>-<", "general2", 1536107260, bad_karma});

  int count = 0;
  db.RangeByKarma(bad_karma, good_karma, [&count](const Record&) {
    ++count;
    return true;
  });

  ASSERT_EQUAL(2, count);

  int count2 = 0;
  db.AllByUser("master", [&count2](const Record&) {
    ++count2;
    return true;
  });
  ASSERT_EQUAL(1, count2);

}

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

void TestReplacement() {
  const string final_body = "Feeling sad";

  Database db;
  db.Put({"id", "Have a hand", "not-master", 1536107260, 10});
  db.Erase("id");
  db.Put({"id", final_body, "not-master", 1536107260, -10});

  auto record = db.GetById("id");
  ASSERT(record != nullptr);
  ASSERT_EQUAL(final_body, record->title);
}

void runTests() {
  TestRunner tr;
  RUN_TEST(tr, TestRecordEquality);
  RUN_TEST(tr, TestPut);
  RUN_TEST(tr, TestGetById);
  RUN_TEST(tr, TestErase);
  RUN_TEST(tr, TestSameUser);
  RUN_TEST(tr, TestReplacement);
  RUN_TEST(tr, TestRangeBoundaries);
}