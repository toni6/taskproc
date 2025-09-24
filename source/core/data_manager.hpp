#pragma once
#include "../io/reader.hpp"
#include <map>
#include <memory>
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
  std::map<int, Task> tasks_;

public:
  /// Construct a DataManager and register available readers.
  DataManager();

  /**
   * Copying is disabled because `DataManager` contains unique ownership of
   * reader objects (`std::unique_ptr<ITaskReader>`). Attempting to copy will
   * result in a compile-time error.
   *
   * @note This prevents accidental duplication of ownership.
   */
  DataManager(const DataManager &) = delete;
  DataManager &operator=(const DataManager &) = delete;

  /**
   * Move semantics: ownership of readers, tasks and `current_filepath_` is
   * transferred to the destination object.
   *
   * @post The moved-from object is left in a valid but unspecified state.
   * @post Any raw, non-owning pointers previously obtained from
   *       `select_reader()` continue to point to the same underlying reader
   *       objects (the objects themselves are not destroyed by the move),
   *       but those objects are now owned by the moved-to DataManager.
   *       Callers holding non-owning pointers should not assume which
   *       DataManager currently owns them.
   */
  DataManager(DataManager &&) = default;
  DataManager &operator=(DataManager &&) = default;

  /**
   * @brief Load tasks from `filepath` and replace the manager's tasks on success.
   *
   * @pre `filepath` is a path to a readable file and a matching reader exists.
   * @post On success: `tasks_` contains the loaded tasks and `current_filepath_`
   *       equals `filepath`.
   * @post On failure: `tasks_` remains unchanged.
   * @throws std::exception (or implementation-specific exceptions) on I/O or parse errors.
   * @note Implementations may log errors; callers should consider catching exceptions
   *       if they want to handle failures without termination.
   *
   * @param filepath Path to the file to load.
   * @return true if the file was successfully loaded and parsed, false otherwise.
   */
  bool load_from_file(const std::string &filepath);

  /**
   * @brief Reload tasks from the currently loaded file.
   *
   * @pre A file was previously loaded successfully (i.e. `current_filepath_` is non-empty).
   * @post On success: `tasks_` is updated with the loaded tasks (same semantics as load_from_file).
   * @return true if the reload succeeded, false otherwise.
   */
  bool reload_tasks();

  /**
   * @brief Get the number of tasks currently loaded.
   *
   * @return The number of tasks currently loaded.
   */
  size_t task_count() const;

  /**
   * @brief Get the path of the currently loaded file.
   *
   * @return The path of the currently loaded file, or an empty string if no file is loaded.
   */
  std::string current_file_path() const;

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
  ITaskReader *select_reader(const std::string &filename) const;
};
