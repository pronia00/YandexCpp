#include "testSI.h"

int main() {
  TestRunner tr;
  
  RUN_TEST(tr, TestRecordEquality);
  RUN_TEST(tr, TestPut);
  RUN_TEST(tr, TestGetById);
  RUN_TEST(tr, TestErase);
  RUN_TEST(tr, TestSameUser);
  RUN_TEST(tr, TestReplacement);
  RUN_TEST(tr, TestRangeBoundaries);

  return 0;
}