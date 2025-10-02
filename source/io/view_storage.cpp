#include "io/view_storage.hpp"
#include "core/view_action.hpp"
#include <filesystem>
#include <fstream>
#include <mutex>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void ViewStorage::set_filepath(const std::filesystem::path filepath) noexcept {
  std::lock_guard<std::mutex> lock(mutex_);
  current_filepath_ = filepath;
  history_.clear();
}

std::optional<std::filesystem::path> ViewStorage::filepath() const noexcept {
  std::lock_guard<std::mutex> lock(mutex_);
  return current_filepath_;
}

void ViewStorage::push_action(ViewAction action) noexcept {
  std::lock_guard<std::mutex> lock(mutex_);
  history_.emplace_back(std::move(action));
}

std::vector<ViewAction> ViewStorage::history() const noexcept {
  std::lock_guard<std::mutex> lock(mutex_);
  return history_;
}

void ViewStorage::clear() noexcept {
  std::lock_guard<std::mutex> lock(mutex_);
  std::filesystem::path target_path = storage_dir_ / storage_filename_;
  std::error_code ec;
  std::filesystem::remove(target_path, ec);
  current_filepath_.reset();
  history_.clear();
}

void ViewStorage::clear_history() noexcept {
  std::lock_guard<std::mutex> lock(mutex_);
  history_.clear();
}

void ViewStorage::persist() const {
  // copy under lock
  std::optional<std::filesystem::path> filepath_copy = filepath();
  std::vector<ViewAction> history_copy;
  std::filesystem::path target_path;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    filepath_copy = current_filepath_;
    history_copy = history_;
    target_path = storage_dir_ / storage_filename_;
  }

  // build JSON
  json json_data;
  json_data["filepath"] = filepath_copy->string();
  json_data["history"] = json::array();
  for (const auto &action : history_copy) {
    json action_json;
    action_json["type"] = to_string(action.type);
    action_json["payload"] = action.payload;
    json_data["history"].emplace_back(std::move(action_json));
  }

  // write JSON to file
  const auto tmp = target_path.string() + ".tmp";
  {
    std::ofstream ofs(tmp, std::ios::binary | std::ios::trunc);
    if (!ofs)
      throw std::runtime_error("Failed to open temp storage file for writing: " + tmp);
    ofs << json_data.dump(2);
  }

  // rename temp file to target file
  std::filesystem::rename(tmp, target_path);
}

bool ViewStorage::load_from_storage() {
  std::filesystem::path target_path;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    target_path = storage_dir_ / storage_filename_;
  }

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

  {
    std::lock_guard<std::mutex> lock(mutex_);
    current_filepath_ = std::filesystem::path(filepath);
    history_ = std::move(history);
  }

  return true;
}
