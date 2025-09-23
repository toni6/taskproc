#include "core/data_manager.hpp"
#include "io/csv_reader.hpp"
#include "io/json_reader.hpp"
#include <iostream>

DataManager::DataManager() { registerReaders(); }

void DataManager::registerReaders() {
  readers_.emplace_back(std::make_unique<CSVReader>());
  readers_.emplace_back(std::make_unique<JSONReader>());
}

/*
 * Load tasks from a file using the appropriate reader.
 *
 * We swap the tasks map with the loaded tasks to guarantee the tasks are fully
 * loaded and replaced or, in case of failure, unchanged.
 *
 * Returns true if the file was successfully loaded and parsed, false otherwise.
 */
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
