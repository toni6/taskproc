#include "core/data_manager.hpp"
#include "io/csv_reader.hpp"
#include "io/json_reader.hpp"
#include "io/view_storage.hpp"
#include <iostream>

DataManager::DataManager() : storage_() {
  register_readers();
  try {
    if (storage_.load_from_storage()) {
      current_filepath_ = storage_.filepath().value_or("");
    }
  } catch (const std::exception &e) {
    std::cerr << "Warning: unable to read view from storage: " << e.what() << "\n";
  }
}

void DataManager::register_readers() {
  readers_.emplace_back(std::make_unique<CSVReader>());
  readers_.emplace_back(std::make_unique<JSONReader>());
}

bool DataManager::load_from_file(const std::string &filepath) {
  ITaskReader *reader = select_reader(filepath);
  if (!reader) {
    std::cerr << "No reader found for file: " << filepath << "\n";
    return false;
  }

  std::vector<Task> tasks;
  try {
    tasks = reader->read_tasks(filepath);
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
    int id = task.id;
    tasks_map.insert_or_assign(id, std::move(task));
  }

  tasks_.swap(tasks_map);
  current_filepath_ = filepath;

  // Store the filepath and clears previous history (set_filepath clears history)
  try {
    storage_.set_filepath(filepath);
    storage_.persist();
  } catch (const std::exception &e) {
    std::cerr << "Warning: failed to persist view storage: " << e.what() << "\n";
    return false;
  }

  return true;
}

ITaskReader *DataManager::select_reader(const std::string &filepath) const {
  for (auto &reader : readers_) {
    if (reader->can_handle(filepath)) {
      return reader.get();
    }
  }
  return nullptr;
}

bool DataManager::reload_tasks() {
  if (current_filepath_.empty()) {
    try {
      if (storage_.load_from_storage()) {
        current_filepath_ = storage_.filepath().value_or("");
      }
    } catch (const std::exception &e) {
      std::cerr << "Error reading view storage: " << e.what() << "\n";
      return false;
    }
    if (current_filepath_.empty()) {
      std::cerr << "The file path is empty\n";
      return false;
    }
  }
  return true;
}

size_t DataManager::task_count() const noexcept { return tasks_.size(); }

const std::string DataManager::current_file_path() const noexcept { return current_filepath_; }

void DataManager::reset_storage() {
  tasks_.clear();
  current_filepath_.clear();
  try {
    storage_.clear();
  } catch (const std::exception &e) {
    std::cerr << "Warning: failed to clear storage: " << e.what() << "\n";
  }
}
