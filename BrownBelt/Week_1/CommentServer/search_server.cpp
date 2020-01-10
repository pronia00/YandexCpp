#include "test_runner.h"

#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <utility>
#include <map>
#include <optional>
#include <unordered_set>
#include <unordered_map>

using namespace std;


enum class HttpCode {
  Ok = 200,
  NotFound = 404,
  Found = 302,
};

class HttpResponse {
public:
  explicit HttpResponse(HttpCode code) 
  {
      SetCode(code);
  }

  HttpResponse& AddHeader(string name = "", string value = "") {
      _headers.insert({move(name), move(value)});
      return *this;
  }
  // "HTTP/1.1 200 OK\nContent-Length: 14\nContent-Length: 6\n\nanother_string"
  // "HTTP/1.1 200 OK\nContent-Length: 14\n\nanother_string"
  HttpResponse& SetContent(string a_content) noexcept {
      _content = move(a_content);
      return *this;
  }

  HttpResponse& SetCode(HttpCode a_code) noexcept
  {
      _code = a_code;
      return *this;
  }

  friend ostream& operator << (ostream& output, const HttpResponse& resp) {
      // output code
      output << resp._http_version << " "
             << static_cast<int>(resp._code) << " " 
             << HttpResponse::_code_s.at(resp._code) << "\n";

      // output headers
      for (const auto& [header, value] : resp._headers) {
          output << header << ": " << value << "\n";
      }
      
      if (resp._content != "") {
          output << "Content-Length: " 
                 << to_string(resp._content.size()) << "\n";
      }

      // output content
      output << "\n";
      if (resp._content != "") 
      {
          output << resp._content;
      }

      return output;
  }

private: 
    static const map<HttpCode, string> _code_s;

    static inline const string  _http_version {"HTTP/1.1"};
    HttpCode _code;
    string _content;
    unordered_multimap<string, string> _headers;
};

const map<HttpCode, string> HttpResponse::_code_s {
    { HttpCode::Ok, "OK"}, 
    { HttpCode::NotFound, "Not found"},
    { HttpCode::Found, "Found"}
};


struct HttpRequest {
  string method, path, body;
  map<string, string> get_params;
};

//parse string
pair<string, string> SplitBy(const string& what, const string& by) {
  size_t pos = what.find(by);
  if (by.size() < what.size() && pos < what.size() - by.size()) {
    return {what.substr(0, pos), what.substr(pos + by.size())};
  } else {
    return {what, {}};
  }
}

//extract T from string
template<typename T>
T FromString(const string& s) {
  T x;
  istringstream is(s);
  is >> x;
  return x;
}

pair<size_t, string> ParseIdAndContent(const string& body) {
  auto [id_string, content] = SplitBy(body, " ");
  return {FromString<size_t>(id_string), content};
}

struct LastCommentInfo {
  size_t user_id, consecutive_count;
};

class CommentServer {
private:
  vector<vector<string>> comments_;
  std::optional<LastCommentInfo> last_comment;
  unordered_set<size_t> banned_users;

public:
  HttpResponse ServeRequest(const HttpRequest& req) {
     if (req.method == "POST") {

      // /add_user
      if (req.path == "/add_user") {
        comments_.emplace_back();
        auto response = to_string(comments_.size() - 1);
        return HttpResponse(HttpCode::Ok).SetContent(response);
          // os << "HTTP/1.1 200 OK\n" << "Content-Length: " << response.size() << "\n" << "\n" << response;
      } 
      
      // /add_comment
      if (req.path == "/add_comment") {
        auto [user_id, comment] = ParseIdAndContent(req.body);

        if (!last_comment || last_comment->user_id != user_id) {
          last_comment = LastCommentInfo {user_id, 1};

        } else if (++last_comment->consecutive_count > 3) {
          banned_users.insert(user_id);
        }

        if (banned_users.count(user_id) == 0) {
          comments_[user_id].push_back(string(comment));
          return HttpResponse(HttpCode::Ok);
          // os << "HTTP/1.1 200 OK\n\n";
        } else {
          return HttpResponse(HttpCode::Found).AddHeader("Location", "/captcha");
          // os << "HTTP/1.1 302 Found\n\n"  "Location: /captcha\n" "\n";
        }
      }
      // /checkcaptcha
      if (req.path == "/checkcaptcha") {
        if (auto [id, response] = ParseIdAndContent(req.body); response == "42") {
          banned_users.erase(id);
          if (last_comment && last_comment->user_id == id) {
            last_comment.reset();
          }
          return HttpResponse(HttpCode::Ok);
          // os << "HTTP/1.1 200 OK\n\n";
        }
        else {
          return HttpResponse(HttpCode::Found).AddHeader("Location", "/captcha");
        }
      } 

    }
    // Get
    if (req.method == "GET") {
      // /user_comments
      if (req.path == "/user_comments") {
        auto user_id = FromString<size_t>(req.get_params.at("user_id"));
        string response;
        for (const string& c : comments_[user_id]) {
          response += c + '\n';
        }
        return HttpResponse(HttpCode::Ok).SetContent(response);
        // os << "HTTP/1.1 200 OK\n" << "Content-Length: " << response.size() << response;
      } 
      
      // /captcha
      if (req.path == "/captcha") {
        return HttpResponse(HttpCode::Ok)
          .SetContent("What's the answer for The Ultimate Question of Life, the Universe, and Everything?");
        // os << "HTTP/1.1 200 OK\n" << "Content-Length: 80\n" << "\n" << "What's the answer for The Ultimate Question of Life, the Universe, and Everything?";
      }
    }
    return HttpResponse(HttpCode::NotFound);
  }

  void ServeRequest(const HttpRequest& req, ostream& os) {
    os << ServeRequest(req);
    /*
      if (req.method == "POST") {
        if (req.path == "/add_user") {
          comments_.emplace_back();
          auto response = to_string(comments_.size() - 1);
          os << "HTTP/1.1 200 OK\n" << "Content-Length: " << response.size() << "\n" << "\n"
            << response;
        } else if (req.path == "/add_comment") {
          auto [user_id, comment] = ParseIdAndContent(req.body);

          if (!last_comment || last_comment->user_id != user_id) {
            last_comment = LastCommentInfo {user_id, 1};
          } else if (++last_comment->consecutive_count > 3) {
            banned_users.insert(user_id);
          }

          if (banned_users.count(user_id) == 0) {
            comments_[user_id].push_back(string(comment));
            os << "HTTP/1.1 200 OK\n\n";
          } else {
            os << "HTTP/1.1 302 Found\n\n"
              "Location: /captcha\n"
              "\n";
          }
        } else if (req.path == "/checkcaptcha") {
          if (auto [id, response] = ParseIdAndContent(req.body); response == "42") {
            banned_users.erase(id);
            if (last_comment && last_comment->user_id == id) {
              last_comment.reset();
            }
            os << "HTTP/1.1 200 OK\n\n";
          }
        } else {
          os << "HTTP/1.1 404 Not found\n\n";
        }
      } else if (req.method == "GET") {
        if (req.path == "/user_comments") {
          auto user_id = FromString<size_t>(req.get_params.at("user_id"));
          string response;
          for (const string& c : comments_[user_id]) {
            response += c + '\n';
          }

          os << "HTTP/1.1 200 OK\n" << "Content-Length: " << response.size() << response;
        } else if (req.path == "/captcha") {
          os << "HTTP/1.1 200 OK\n" << "Content-Length: 80\n" << "\n"
            << "What's the answer for The Ultimate Question of Life, the Universe, and Everything?";
        } else {
          os << "HTTP/1.1 404 Not found\n\n";
        }
      }
    
    */
  }
};

struct HttpHeader {
  string name, value;
};

ostream& operator<<(ostream& output, const HttpHeader& h) {
  return output << h.name << ": " << h.value;
}

bool operator==(const HttpHeader& lhs, const HttpHeader& rhs) {
  return lhs.name == rhs.name && lhs.value == rhs.value;
}

struct ParsedResponse {
  int code;
  vector<HttpHeader> headers;
  string content;
};

istream& operator >>(istream& input, ParsedResponse& r) {
  string line;
  getline(input, line);

  {
    istringstream code_input(line);
    string dummy;
    code_input >> dummy >> r.code;
  }

  size_t content_length = 0;

  r.headers.clear();
  while (getline(input, line) && !line.empty()) {
    if (auto [name, value] = SplitBy(line, ": "); name == "Content-Length") {
      istringstream length_input(value);
      length_input >> content_length;
    } else {
      r.headers.push_back( {std::move(name), std::move(value)});
    }
  }

  r.content.resize(content_length);
  input.read(r.content.data(), r.content.size());
  return input;
}

void Test(CommentServer& srv, const HttpRequest& request, const ParsedResponse& expected) {
  stringstream ss;
  srv.ServeRequest(request, ss);
  ParsedResponse resp;
  ss >> resp;
  ASSERT_EQUAL(resp.code, expected.code);
  ASSERT_EQUAL(resp.headers, expected.headers);
  ASSERT_EQUAL(resp.content, expected.content);
}

template <typename CommentServer>
void TestServer() {
  CommentServer cs;

  const ParsedResponse ok{200};
  const ParsedResponse redirect_to_captcha{302, {{"Location", "/captcha"}}, {}};
  const ParsedResponse not_found{404};

  Test(cs, {"POST", "/add_user"}, {200, {}, "0"});
  Test(cs, {"POST", "/add_user"}, {200, {}, "1"});
  Test(cs, {"POST", "/add_comment", "0 Hello"}, ok);
  Test(cs, {"POST", "/add_comment", "1 Hi"}, ok);
  Test(cs, {"POST", "/add_comment", "1 Buy my goods"}, ok);
  Test(cs, {"POST", "/add_comment", "1 Enlarge"}, ok);
  Test(cs, {"POST", "/add_comment", "1 Buy my goods"}, redirect_to_captcha);
  Test(cs, {"POST", "/add_comment", "0 What are you selling?"}, ok);
  Test(cs, {"POST", "/add_comment", "1 Buy my goods"}, redirect_to_captcha);
  Test(
    cs,
    {"GET", "/user_comments", "", {{"user_id", "0"}}},
    {200, {}, "Hello\nWhat are you selling?\n"}
  );
  Test(
    cs,
    {"GET", "/user_comments", "", {{"user_id", "1"}}},
    {200, {}, "Hi\nBuy my goods\nEnlarge\n"}
  );
  Test(
    cs,
    {"GET", "/captcha"},
    {200, {}, {"What's the answer for The Ultimate Question of Life, the Universe, and Everything?"}}
  );
  Test(cs, {"POST", "/checkcaptcha", "1 24"}, redirect_to_captcha);
  Test(cs, {"POST", "/checkcaptcha", "1 42"}, ok);
  Test(cs, {"POST", "/add_comment", "1 Sorry! No spam any more"}, ok);
  Test(
    cs,
    {"GET", "/user_comments", "", {{"user_id", "1"}}},
    {200, {}, "Hi\nBuy my goods\nEnlarge\nSorry! No spam any more\n"}
  );

  Test(cs, {"GET", "/user_commntes"}, not_found);
  Test(cs, {"POST", "/add_uesr"}, not_found);
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestServer<CommentServer>);

  return 0;
}