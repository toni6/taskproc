#include "io/view_storage.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <system_error>

using json = nlohmann::json;

void ViewStorage::set_filepath(const std::filesystem::path &filepath) noexcept {
  current_filepath_ = filepath;
  history_.clear();
}

void ViewStorage::push_action(ViewAction action) noexcept { history_.emplace_back(std::move(action)); }

const std::vector<ViewAction> &ViewStorage::history() const noexcept { return history_; }

void ViewStorage::clear() noexcept {
  std::filesystem::path target_path = storage_dir_ / storage_filename_;
  std::error_code ec;
  std::filesystem::remove(target_path, ec);
  current_filepath_.reset();
  history_.clear();
}

void ViewStorage::clear_history() noexcept {
  history_.clear();

  // Persist the cleared history to disk while keeping filepath
  if (current_filepath_.has_value()) {
    try {
      persist();
    } catch (const std::exception &e) {
      // Log error but don't throw in noexcept function
      // In production, use proper logging framework
      std::cerr << "Warning: Failed to persist cleared history: " << e.what() << std::endl;
    }
  }
}

void ViewStorage::persist() {
  if (!current_filepath_.has_value()) {
    throw std::runtime_error("Cannot persist: no filepath set");
  }

  // build JSON
  json json_data;
  json_data["filepath"] = current_filepath_->string();
  json_data["history"] = json::array();
  for (const auto &action : history_) {
    json action_json;
    action_json["type"] = to_string(action.type);
    action_json["payload"] = action.payload;
    json_data["history"].emplace_back(std::move(action_json));
  }

  // write JSON to file
  std::filesystem::path target_path = storage_dir_ / storage_filename_;
  const auto tmp = target_path.string() + ".tmp";
  {
    std::ofstream ofs(tmp, std::ios::binary | std::ios::trunc);
    if (!ofs)
      throw std::runtime_error("Failed to open temp storage file for writing: " + tmp);
    ofs << json_data.dump(2);
  }

  // rename temp file to target file
  std::error_code ec;
  std::filesystem::rename(tmp, target_path, ec);
  if (ec)
    throw std::runtime_error("Failed to commit storage file: " + ec.message());
}

bool ViewStorage::load_from_storage() {
  std::filesystem::path target_path = storage_dir_ / storage_filename_;

  if (!std::filesystem::exists(target_path))
    return false;

  json json_data;
  {
    std::ifstream ifs(target_path);
    if (!ifs)
      throw std::runtime_error("Failed to open storage file for reading: " + target_path.string());
    ifs >> json_data;
  }

  const std::string filepath = json_data["filepath"].get<std::string>();

  std::vector<ViewAction> history;
  if (json_data.contains("history") && json_data["history"].is_array()) {
    for (const auto &history_vc : json_data["history"]) {
      if (auto type = view_op_type_from_string(history_vc["type"].get<std::string_view>())) {
        history.emplace_back(*type, history_vc.value("payload", ""));
      }
    }
  }

  current_filepath_ = std::filesystem::path(filepath);
  history_ = std::move(history);

  return true;
}
