#pragma once
#include "../io/reader.hpp"
#include <map>
#include <memory>
#include <vector>

class DataManager {
private:
  std::vector<std::unique_ptr<ITaskReader>> readers_;
  std::string current_filepath_;
  std::map<int, Task> tasks_;

public:
  DataManager();
  bool loadFromFile(const std::string &filepath);

private:
  void registerReaders();
  ITaskReader *selectReader(const std::string &filename);
};
