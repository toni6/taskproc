#pragma once
#include "io/view_storage.hpp"
#include "task.hpp"
#include <functional>
#include <map>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

// ============================================================================
// Filter Specification Types
// ============================================================================

/// Comparison operators for filter expressions
enum class FilterOp {
  Equal,              ///< ==
  NotEqual,           ///< !=
  GreaterThan,        ///< >
  GreaterThanOrEqual, ///< >=
  LessThan,           ///< <
  LessThanOrEqual     ///< <=
};

/// Field identifiers for filtering
enum class FilterField { Id, Title, Status, Priority, CreatedDate, DueDate, Assignee, Description };

/**
 * @brief Specification for a single filter predicate.
 *
 * Represents expressions like "priority>=3" or "status=todo".
 * Special filters (tag-based, text search) use specialized methods.
 */
struct FilterSpec {
  FilterField field;
  FilterOp op;
  std::string value;

  FilterSpec(FilterField field_, FilterOp op_, std::string value_) noexcept :
      field(field_), op(op_), value(std::move(value_)) {}
};

// ============================================================================
// Sort Specification Types
// ============================================================================

/// Sort direction
enum class SortDirection {
  Ascending, ///< Ascending order (low to high)
  Descending ///< Descending order (high to low)
};

/// Field identifiers for sorting
enum class SortField { Id, Title, Status, Priority, CreatedDate, DueDate };

/**
 * @brief Specification for sorting the current view.
 *
 * Example: sort priority descending, or sort due_date ascending.
 */
struct SortSpec {
  SortField field;
  SortDirection direction;

  SortSpec(SortField field_, SortDirection direction_ = SortDirection::Ascending) noexcept :
      field(field_), direction(direction_) {}
};

// ============================================================================
// Statistics Types
// ============================================================================

/**
 * @brief Aggregate statistics grouped by task status.
 *
 * Provides counts for each status value found in the current view.
 */
struct StatusStats {
  size_t todo_count{0};
  size_t in_progress_count{0};
  size_t done_count{0};
  size_t other_count{0}; ///< Tasks with non-standard status values

  /// Total count across all statuses
  size_t total() const noexcept { return todo_count + in_progress_count + done_count + other_count; }
};

// ============================================================================
// Database Class
// ============================================================================

/**
 * @brief In-memory task database with filtering, sorting, and query capabilities.
 *
 * The Database maintains:
 * - Canonical storage: all loaded tasks indexed by ID
 * - Current view: filtered/sorted subset of tasks
 * - Secondary indices: for efficient status and tag lookups
 *
 * Responsibilities:
 * - Store and manage task lifecycle
 * - Apply filters (narrows current view)
 * - Apply sorting (reorders current view)
 * - Compute aggregations and statistics
 *
 * @note View operations (filter/sort) do not mutate the canonical task store.
 * @note Not thread-safe; caller must synchronize access if needed.
 */
class Database {
private:
  /// Canonical store (unchanged by filters/sorts), indexed by task ID
  std::map<int, Task> tasks_;

  /// Current view: non-owning pointers to tasks in the canonical store
  std::vector<const Task *> view_;

  /// Secondary index: status -> tasks with that status
  std::unordered_map<std::string, std::vector<const Task *>> status_index_;

  /// Secondary index: tag -> tasks containing that tag
  std::unordered_map<std::string, std::vector<const Task *>> tag_index_;

public:
  // ==========================================================================
  // Data Loading
  // ==========================================================================

  /**
   * @brief Load tasks into the database, replacing any existing data.
   *
   * @pre `tasks` contains valid Task objects (caller-validated).
   * @post `tasks_` contains all tasks indexed by ID.
   * @post `view_` contains pointers to all tasks in insertion order.
   * @post Secondary indices are rebuilt.
   * @post Previous filters/sorts are cleared.
   * @throws none (noexcept move semantics).
   *
   * @param tasks Vector of tasks to load (moved into storage).
   */
  void load(std::vector<Task> tasks);

  // ==========================================================================
  // View Management
  // ==========================================================================

  /**
   * @brief Reset the current view to include all loaded tasks.
   *
   * @pre Database may be empty or contain tasks.
   * @post `view_` contains pointers to all tasks in `tasks_`, ordered by ID.
   * @post Clears any active filters.
   * @throws none (noexcept).
   */
  void reset_view() noexcept;

  /**
   * @brief Apply a filter to narrow the current view.
   *
   * Filtering is cumulative: each filter further narrows the existing view.
   * To start fresh, call `reset_view()` first.
   *
   * @pre `filter` is a valid FilterSpec with recognized field and operator.
   * @pre Current view is non-empty (or filter has no effect).
   * @post `view_` contains only tasks matching the filter predicate.
   * @post Order within view is preserved.
   * @throws none (invalid filters are logged and ignored).
   *
   * @param filter The filter specification to apply.
   */
  void apply_filter(const FilterSpec &filter);

  /**
   * @brief Apply a sort to reorder the current view.
   *
   * @pre `sort` specifies a valid sortable field and direction.
   * @pre Current view is non-empty (or sort has no effect).
   * @post `view_` is reordered according to the sort specification.
   * @post Sorting is stable (preserves relative order of equal elements).
   * @throws none (invalid sorts are logged and ignored).
   *
   * @param sort The sort specification to apply.
   */
  void apply_sort(const SortSpec &sort);

  /**
   * @brief Filter current view to tasks containing a specific tag.
   *
   * @pre `tag` is a non-empty tag string.
   * @post `view_` contains only tasks whose `tags` vector includes `tag`.
   * @throws none (noexcept).
   *
   * @param tag The tag to filter by.
   */
  void filter_by_tag(std::string_view tag);

  /**
   * @brief Filter current view to tasks with no tags.
   *
   * @pre none
   * @post `view_` contains only tasks with empty `tags` vector.
   * @throws none (noexcept).
   */
  void filter_no_tags() noexcept;

  /**
   * @brief Filter current view by text search in title and description.
   *
   * @pre `text` is the search string (case-insensitive substring match).
   * @post `view_` contains only tasks where `text` appears in title or description.
   * @throws none (noexcept).
   *
   * @param text The text to search for.
   */
  void search_text(std::string_view text);

  /**
   * @brief Replay a sequence of view actions to reconstruct a saved view state.
   *
   * @pre Database contains loaded tasks.
   * @pre `actions` is a vector of valid ViewAction objects.
   * @post View is modified according to the action sequence.
   * @post Invalid actions are logged and skipped.
   * @throws none (noexcept - errors are logged).
   *
   * @param actions Vector of actions to replay in order.
   */
  void replay_history(const std::vector<ViewAction> &actions) noexcept;

  // ==========================================================================
  // Data Access
  // ==========================================================================

  /**
   * @brief Get the current filtered/sorted view.
   *
   * @pre none
   * @post Returns a const reference to the current view.
   * @throws none (noexcept).
   * @note Returned pointers remain valid until next `load()` call.
   *
   * @return Const reference to vector of task pointers.
   */
  const std::vector<const Task *> &current_view() const noexcept { return view_; }

  /**
   * @brief Get a task by ID.
   *
   * @pre `id` may or may not exist in the database.
   * @post Returns pointer to task if found, nullptr otherwise.
   * @throws none (noexcept).
   * @note Returned pointer remains valid until next `load()` call.
   *
   * @param id The task ID to look up.
   * @return Pointer to task or nullptr if not found.
   */
  const Task *get_task_by_id(int id) const noexcept;

  /**
   * @brief Get total number of tasks loaded in canonical storage.
   *
   * @pre none
   * @post Returns count of all tasks, ignoring filters.
   * @throws none (noexcept).
   *
   * @return Total task count.
   */
  size_t total_task_count() const noexcept { return tasks_.size(); }

  /**
   * @brief Get number of tasks in current view.
   *
   * @pre none
   * @post Returns count of tasks in filtered/sorted view.
   * @throws none (noexcept).
   *
   * @return Current view task count.
   */
  size_t view_task_count() const noexcept { return view_.size(); }

  /**
   * @brief Check if database is empty.
   *
   * @pre none
   * @post Returns true if no tasks are loaded.
   * @throws none (noexcept).
   *
   * @return True if empty, false otherwise.
   */
  bool empty() const noexcept { return tasks_.empty(); }

  // ==========================================================================
  // Aggregations and Statistics
  // ==========================================================================

  /**
   * @brief Compute status distribution for current view.
   *
   * @pre none
   * @post Returns StatusStats with counts for each status category.
   * @throws none (noexcept).
   *
   * @return Status statistics for current view.
   */
  StatusStats status_stats() const noexcept;

  /**
   * @brief Compute average priority of tasks in current view.
   *
   * @pre none
   * @post Returns average priority (1.0 to 5.0), or 0.0 if view is empty.
   * @throws none (noexcept).
   *
   * @return Average priority value.
   */
  double average_priority() const noexcept;

  /**
   * @brief Count overdue tasks in current view.
   *
   * A task is overdue if:
   * - It has a `due_date` value
   * - `due_date < today_iso`
   * - `status != "done"`
   *
   * @pre `today_iso` is a valid ISO 8601 date string (YYYY-MM-DD).
   * @post Returns count of overdue tasks.
   * @throws none (noexcept, invalid dates are skipped).
   *
   * @param today_iso Current date in ISO 8601 format for comparison.
   * @return Count of overdue tasks.
   */
  size_t overdue_count(std::string_view today_iso) const noexcept;

private:
  // ==========================================================================
  // Internal Helpers
  // ==========================================================================

  /// Rebuild secondary indices after loading tasks
  void rebuild_indices() noexcept;

  /// Helper to create a comparator function for the given SortSpec
  std::function<bool(const Task *, const Task *)> make_comparator(const SortSpec &sort) const;

  /// Helper to create a predicate function for the given FilterSpec
  std::function<bool(const Task *)> make_predicate(const FilterSpec &filter) const;
};
