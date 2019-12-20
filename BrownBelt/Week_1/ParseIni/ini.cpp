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

Document Load(istream& input) {
    Document doc;
    Section* section;
    for (string line; getline(input, line); ) {
        if (!line.empty()) {
            if (line[0] == '[') {
                //cout << line[0] << endl;
                section =
                &doc.AddSection(line.substr(1, line.size() - 2));
            }
            else {
                size_t eq_pos = line.find('=');
                section->insert({line.substr(0, eq_pos), line.substr(eq_pos + 1)});
            }
        }
    }
    return doc;
}


}