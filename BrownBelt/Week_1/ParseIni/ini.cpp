#include "ini.h"
#include <string_view>

using namespace std;

namespace Ini {

Section& Document::AddSection(string name) {
    return sections[name];
}

const Section& Document::GetSection(const string& name) const {
    if (sections.find(name) == sections.end()) {
        throw out_of_range("");
    } 
    else {
        return sections.at(name);
    }
}

size_t Document::SectionCount() const {
    return sections.size();
}

pair<string, string> Split(string_view line, char by) {
  size_t pos = line.find(by);
  string_view left = line.substr(0, pos);

  if (pos < line.size() && pos + 1 < line.size()) {
    return {string(left), string(line.substr(pos + 1))};
  } else {
    return {string(left), string()};
  }
}

string_view Lstrip(string_view line) {
  while (!line.empty() && isspace(line[0])) {
    line.remove_prefix(1);
  }
  return line;
}

Document Load(istream& input) {
    Document doc;
    Section* section;
    for (string line; getline(input, line); ) {
        line = Lstrip(line);
        if (line[0] == '[') {
            //cout << line[0] << endl;
            section =
             &doc.AddSection(line.substr(1, line.size() - 2));
             continue;
        }
        auto [name, value] = Split(line, '=');
        if (!name.empty() && !value.empty()) {
            section->insert({move(name), move(value)});
        }
    }
    return doc;
}


}