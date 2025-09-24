#include "core/data_manager.hpp"
#include "io/csv_reader.hpp"
#include "io/json_reader.hpp"
#include <iostream>

DataManager::DataManager() { registerReaders(); }

void DataManager::registerReaders() {
  readers_.emplace_back(std::make_unique<CSVReader>());
  readers_.emplace_back(std::make_unique<JSONReader>());
}

bool DataManager::loadFromFile(const std::string &filepath) {
  ITaskReader *reader = selectReader(filepath);
  if (!reader) {
    std::cerr << "No reader found for file: " << filepath << "\n";
    return false;
  }

  std::vector<Task> tasks;
  try {
    tasks = reader->readTasks(filepath);
  } catch (const std::exception &e) {
    std::cerr << "Error reading file: " << filepath << "\n";
    std::cerr << e.what() << "\n";
    return false;
  }

  if (tasks.empty()) {
    std::cerr << "No tasks found in file: " << filepath << "\n";
    return false;
  }

  std::map<int, Task> tasks_map;
  for (auto &task : tasks) {
    tasks_map.insert_or_assign(task.id, std::move(task));
  }

  tasks_.swap(tasks_map);
  // TODO: store the filepath in a storage (filesystem, db, ...)
  current_filepath_ = filepath;
  return true;
}

ITaskReader *DataManager::selectReader(const std::string &filepath) const {
  for (auto &reader : readers_) {
    if (reader->canHandle(filepath)) {
      return reader.get();
    }
  }
  return nullptr;
}

bool DataManager::reloadTasks() {
  // TOOD: the filepath must be stored (filesystem, db, ...) and obtained from the storage here
  if (current_filepath_.empty()) {
    std::cerr << "No file loaded\n";
    return false;
  }
  return loadFromFile(current_filepath_);
}

size_t DataManager::getTaskCount() const { return tasks_.size(); }

std::string DataManager::getCurrentFilePath() const { return current_filepath_; }
