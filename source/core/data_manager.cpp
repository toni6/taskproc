#include "core/data_manager.hpp"
#include "core/expr_parser.hpp"
#include "io/csv_reader.hpp"
#include "io/json_reader.hpp"
#include "io/view_storage.hpp"
#include <iostream>
#include <string>
#include <string_view>

DataManager::DataManager() : storage_{} {
  register_readers();
  try {
    if (storage_.load_from_storage()) {
      auto saved_path = storage_.filepath();
      if (saved_path.has_value()) {
        current_filepath_ = saved_path->string();
      }

      // 1. Load tasks from file
      ITaskReader *reader = select_reader(current_filepath_);
      if (!reader) {
        std::cerr << "No reader found for file: " << current_filepath_ << "\n";
        return;
      }

      std::vector<Task> tasks = reader->read_tasks(current_filepath_);

      // 2. Load into databse
      database_.load(tasks);
      std::cerr << "Loaded " << tasks.size() << " tasks\n";

      // 3. Replay history to reconstruct view
      const auto &history = storage_.history();
      if (!history.empty()) {
        database_.replay_history(history);
      }

      std::cerr << "Replaying " << history.size() << " actions\n";
    }
  } catch (const std::exception &e) {
    std::cerr << "Warning: unable to read view from storage: " << e.what() << "\n";
  }
}

void DataManager::register_readers() {
  readers_.emplace_back(std::make_unique<CSVReader>());
  readers_.emplace_back(std::make_unique<JSONReader>());
}

bool DataManager::load_from_file(std::string_view filepath) {
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

  // Store the filepath and clears previous history (set_filepath clears history)
  try {
    storage_.set_filepath(filepath);
    storage_.persist();
  } catch (const std::exception &e) {
    std::cerr << "Warning: failed to persist view storage: " << e.what() << "\n";
    return false;
  }

  // 1. Load tasks into database
  database_.load(tasks);
  // 2. Store the filepath (this clears history for new loads)
  current_filepath_ = filepath;
  // 3. Perist (saves empty history for new loads)
  try {
    storage_.persist();
  } catch (const std::exception &e) {
    std::cerr << "Warning: failed to persist view storage: " << e.what() << "\n";
    return false;
  }

  return true;
}

ITaskReader *DataManager::select_reader(std::string_view filepath) const {
  for (const auto &reader : readers_) {
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
  return load_from_file(current_filepath_);
}

bool DataManager::apply_filter(std::string_view filter) {
  auto filter_spec = ExpressionParser::parse_filter(filter);
  if (!filter_spec) {
    std::cerr << "Invalid filter expression\n";
    return false;
  }
  database_.apply_filter(*filter_spec);

  storage_.push_action(ViewAction{ViewOpType::Filter, std::string(filter)});

  try {
    storage_.persist();
  } catch (const std::exception &e) {
    std::cerr << "Warning: failed to persist view storage: " << e.what() << "\n";
  }

  return true;
}

bool DataManager::apply_sort(std::string_view sort) {
  auto sort_spec = ExpressionParser::parse_sort(sort);
  if (!sort_spec) {
    std::cerr << "Invalid sort expression\n";
    return false;
  }
  database_.apply_sort(*sort_spec);

  storage_.push_action(ViewAction{ViewOpType::Sort, std::string(sort)});

  try {
    storage_.persist();
  } catch (const std::exception &e) {
    std::cerr << "Warning: failed to persist view storage: " << e.what() << "\n";
  }

  return true;
}

size_t DataManager::task_count() const noexcept { return database_.total_task_count(); }

std::string DataManager::current_file_path() const noexcept { return current_filepath_; }

const std::vector<const Task *> &DataManager::current_view() const noexcept { return database_.current_view(); }

void DataManager::reset_view() {
  storage_.clear_history();
  database_.reset_view();
}
