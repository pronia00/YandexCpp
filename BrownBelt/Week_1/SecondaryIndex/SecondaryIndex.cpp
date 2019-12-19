#include "SecondaryIndex.h"

bool Database::Put(const Record& record) {
    if (id_data.find(record.id) != id_data.end()) { 
      return false; 
    }
    SecondaryIndexElement index;
    index.list_iter  = data.insert(data.begin(), record);
  	index.ts_iter    = timestamp_data.insert({ record.timestamp, index.list_iter });
    index.karma_iter = karma_data.insert({ record.karma, index.list_iter });
    index.user_iter  = user_data.insert({record.user, index.list_iter});

    id_data[record.id] = move(index);
    return true;
  }

  const Record* Database::GetById(const string& id) const {
    auto res = id_data.find(id);
    return (res == id_data.end())
      ? nullptr
      : &(*(res->second.list_iter));
  }

  bool Database::Erase(const string& id) {
    auto res = id_data.find(id);
    if (res == id_data.end()) {
      return false;
    }
    timestamp_data.erase(res->second.ts_iter);
    karma_data.erase(res->second.karma_iter);
    user_data.erase(res->second.user_iter);
    data.erase(res->second.list_iter);
    id_data.erase(res);

    return true;
  }