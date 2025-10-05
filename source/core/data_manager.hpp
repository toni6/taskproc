#pragma once
#include "../io/reader.hpp"
#include "../io/view_storage.hpp"
#include "core/database.hpp"
#include <memory>
#include <string_view>
#include <vector>

/**
 * @brief DataManager manages the loading and reloading of tasks from files.
 *
 * Use this class to load tasks from supported file formats (CSV, JSON, etc.).
 */
class DataManager {
private:
  std::vector<std::unique_ptr<ITaskReader>> readers_;
  std::string current_filepath_;
  ViewStorage storage_;
  Database database_;

public:
  /// Construct a DataManager and register available readers.
  DataManager();

  /**
   * @brief Load tasks from `filepath` and replace the manager's tasks on success.
   *
   * @pre `filepath` is a path to a readable file and a matching reader exists.
   * @post On success: `tasks_` contains the loaded tasks and `current_filepath_`
   *       equals `filepath`.
   * @post On failure: `tasks_` remains unchanged.
   * @throws std::exception (or implementation-specific exceptions) on I/O or parse errors.
   * @param filepath Path to the file to load.
   * @return true if the file was successfully loaded and parsed, false otherwise.
   */
  bool load_from_file(std::string_view filepath);

  /*
   * @brief Reload tasks from the currently loaded file.
   *
   * @pre A file was previously loaded successfully (i.e. `current_filepath_` is non-empty).
   * @post On success: `tasks_` is updated with the loaded tasks (same semantics as load_from_file).
   * @return true if the reload succeeded, false otherwise.
   */
  bool reload_tasks();

  /**
   * @brief Apply a filter expression to the current view and record it.
   * @pre `expr` is a valid filter expression for the engine to interpret.
   * @post On success: the action is appended to history and persisted.
   * @throws none (returns false if no current file is known).
   */
  bool apply_filter(std::string_view expr);

  /**
   * @brief Apply a sort expression to the current view and record it.
   * @pre `expr` is a valid sort expression (e.g., "due_date desc").
   * @post On success: the action is appended to history and persisted.
   * @throws none (returns false if no current file is known).
   */
  bool apply_sort(std::string_view expr);

  /**
   * @brief Get the number of tasks currently loaded.
   *
   * @return The number of tasks currently loaded.
   */
  size_t task_count() const noexcept;

  /**
   * @brief Get the path of the currently loaded file.
   *
   * @return The path of the currently loaded file, or an empty string if no file is loaded.
   */
  std::string current_file_path() const noexcept;

  /**
   * @brief Get the current filtered/sorted view of tasks.
   *
   * @pre none
   * @post Returns const reference to vector of non-owning task pointers.
   * @throws none (noexcept).
   * @note Returned pointers remain valid until next load() or reload() call.
   * @note Do not store pointers beyond DataManager's lifetime.
   *
   * @return Const reference to vector of task pointers.
   */
  const std::vector<const Task *> &current_view() const noexcept;

  /**
   * @brief Reset the view of tasks (removes filters and sorts)
   *
   * @post The tasks view are cleared
   */
  void reset_view();

private:
  /// Register built-in readers (CSV, JSON, ...).
  void register_readers();

  /**
   * @brief Select the appropriate reader for a given filename.
   *
   * @pre This DataManager must remain alive while the caller uses the returned pointer.
   * @post Returns a non-owning pointer to a reader stored in `readers_`, or `nullptr` if none matches.
   * @note The returned pointer is non-owning (it is owned by this DataManager's `readers_`
   *       vector of `std::unique_ptr<ITaskReader>`). Do NOT delete the pointer.
   *       The pointer remains valid only while this DataManager owns the reader.
   *
   * @param filename The name of the file to select a reader for.
   * @return A pointer to the selected reader, or nullptr if no reader is available.
   */
  ITaskReader *select_reader(std::string_view filename) const;
};
