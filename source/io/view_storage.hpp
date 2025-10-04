#pragma once
#include "../core/view_action.hpp"
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

/**
 * @brief Persisted view state: filepath + history of view-modifying commands.
 *
 * ViewStorage stores in-memory state and optionally persists it to a small
 * storage file ("./.taskproc.storage"). The state contains the
 * path to the last-loaded tasks file and an ordered list of view-modifying
 * actions that should be replayed on top of the file to reconstruct the
 * current view.
 */
class ViewStorage {
public:
  ViewStorage() = default;

  /**
   * @brief Set the tasks file path in memory.
   * @pre `path` is syntactically valid. Caller may validate existence.
   * @post `filepath()` returns the provided path.
   * @throws none (noexcept).
   * @note Does not persist automatically; call `persist()` to save to disk.
   */
  void set_filepath(const std::filesystem::path &filepath) noexcept;

  /**
   * @brief Get the current tasks filepath (if any).
   * @pre none
   * @post Returns copy of stored path or std::nullopt.
   * @throws none (noexcept).
   */
  std::optional<std::filesystem::path> filepath() const noexcept { return current_filepath_; }

  /**
   * @brief Add a view-modifying action to the history (in-memory).
   * @pre `action` describes a view-modifying operation.
   * @post The action is appended to the in-memory history.
   * @throws none (noexcept).
   */
  void push_action(ViewAction action) noexcept;

  /**
   * @brief Return a copy of the current history.
   * @pre none
   * @post Returns vector of actions in append order (oldest first).
   * @throws none (noexcept).
   */
  const std::vector<ViewAction> &history() const noexcept;

  /**
   * @brief Clear in-memory filepath and history.
   * @pre none
   * @post `filepath()` == std::nullopt and history() is empty.
   * @throws none (noexcept).
   */
  void clear() noexcept;

  /**
   * @brief Clear in-memory history.
   * @pre none
   * @post history() is empty.
   * @throws none (noexcept).
   */
  void clear_history() noexcept;

  /**
   * @brief Persist the current in-memory state to the storage file atomically.
   * @pre Called when caller wants to save state.
   * @post On success: storage file contains JSON with `filepath` and `history`.
   * @post On failure: storage file is unchanged.
   * @throws std::runtime_error on I/O errors.
   */
  void persist();

  /**
   * @brief Load state from the storage file into memory.
   * @pre Storage file may or may not exist.
   * @post On success: in-memory state reflects storage and returns true.
   * @post If storage missing: no change and returns false.
   * @throws std::runtime_error on I/O errors or malformed storage.
   * @return true if a value was loaded, false if file absent.
   */
  bool load_from_storage();

private:
  std::optional<std::filesystem::path> current_filepath_;
  std::vector<ViewAction> history_;

  // Storage config (storage_dir_ is captured at contstruction time)
  std::filesystem::path storage_dir_{std::filesystem::current_path()};
  std::string storage_filename_{".taskproc.storage"};
};
