#include "animals.h"
#include "test_runner.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>

using namespace std;

using Zoo = vector<Animal>;

Zoo CreateZoo(istream& in) {
  Zoo zoo;
  string word;
  while (in >> word) {
    if (word == "Tiger") {
      Tiger t;
      zoo.push_back(t);
    } else if (word == "Wolf") {
      Wolf w;
      zoo.push_back(w);
    } else if (word == "Fox") {
      Fox f;
      zoo.push_back(f);
    } else {
      throw runtime_error("Unknown animal!");
    }
  }
  return zoo;
}

void Process(const Zoo& zoo, ostream& out) {
  for (const auto& animal : zoo) {
    out << animal.Voice() << "\n";
  }
}

void TestZoo() {
  istringstream input("Tiger Wolf Fox Tiger");
  ostringstream output;
  Process(CreateZoo(input), output);

  const string expected =
    "Rrrr\n"
    "Wooo\n"
    "Tyaf\n"
    "Rrrr\n";

  ASSERT_EQUAL(output.str(), expected);
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestZoo);
}