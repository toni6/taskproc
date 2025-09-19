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
  std::cout << "Selected reader: " << reader << "\n";
  if (reader) {
    std::vector<Task> tasks = reader->readTasks(filepath);
    std::cout << "Loaded " << tasks.size() << " tasks\n";
    if (!tasks.empty()) {
      tasks_.clear();
      for (const auto &task : tasks) {
        tasks_.insert_or_assign(task.id, task);
      }
      current_filepath_ = filepath;
      return true;
    }
  }
  return false;
}

ITaskReader *DataManager::selectReader(const std::string &filepath) {
  for (auto &reader : readers_) {
    if (reader->canHandle(filepath)) {
      return reader.get();
    }
  }
  return nullptr;
}
