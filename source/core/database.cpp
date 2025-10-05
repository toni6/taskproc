#include "database.hpp"
#include "core/expr_parser.hpp"
#include <algorithm>
#include <iostream>
#include <numeric>

// ============================================================================
// Data Loading
// ============================================================================

void Database::load(std::vector<Task> tasks) {
  // Clear existing data
  tasks_.clear();
  view_.clear();
  status_index_.clear();
  tag_index_.clear();

  // Move tasks into the map
  for (auto &task : tasks) {
    auto id = task.id;
    tasks_.insert_or_assign(id, std::move(task));
  }

  // Rebuild view and indices
  reset_view();
  rebuild_indices();
}

// ============================================================================
// View Management
// ============================================================================

void Database::reset_view() noexcept {
  // Clear the current view
  view_.clear();

  // Reserve space (optimization)
  view_.reserve(tasks_.size());

  // Populate view with pointers to all tasks
  for (const auto &[id, task] : tasks_) {
    view_.push_back(&task);
  }
}

void Database::apply_filter(const FilterSpec &filter) {
  // Create predicate function
  auto predicate = make_predicate(filter);

  // Remove elements that DON'T match
  view_.erase(std::remove_if(view_.begin(), view_.end(), [&predicate](const Task *t) { return !predicate(t); }),
              view_.end());
}

void Database::apply_sort(const SortSpec &sort) {
  // Create comparator function
  auto comp = make_comparator(sort);

  // Sort the view
  std::stable_sort(view_.begin(), view_.end(), comp);
}

void Database::filter_by_tag(std::string_view tag) {
  // Step 1: Use erase-remove with lambda that checks tags
  //   view_.erase(
  //     std::remove_if(view_.begin(), view_.end(),
  //       [tag](const Task* t) {
  //         // Check if tag exists in t->tags vector
  //         return std::find(t->tags.begin(), t->tags.end(), tag) == t->tags.end();
  //       }),
  //     view_.end()
  //   );
  //
  // C++ CONCEPTS:
  // - std::find searches vector, returns iterator
  // - If not found, returns .end()
  // - Lambda captures tag by value (string_view is cheap to copy)
  // - Remove tasks where tag is NOT found
  //
  // OPTIMIZATION (optional):
  // - Could use tag_index_ for O(1) lookup instead of O(n) iteration
  //
  // EXAMPLE STRUCTURE:
  //   view_.erase(
  //     std::remove_if(view_.begin(), view_.end(),
  //       [tag](const Task* t) {
  //         return std::find(t->tags.begin(), t->tags.end(), tag) == t->tags.end();
  //       }),
  //     view_.end()
  //   );
}

void Database::filter_no_tags() noexcept {
  // Step 1: Remove tasks that HAVE tags
  //   view_.erase(
  //     std::remove_if(view_.begin(), view_.end(),
  //       [](const Task* t) { return !t->tags.empty(); }),
  //     view_.end()
  //   )  //
  // C++ CONCEPTS:
  // - Comparator: return true if a should come BEFORE b in sorted order
  // - Ascending: a < b means a before b (smallest first)
  // - Descending: a > b means a before b (largest first)
  // - Lambda return type is deduced automatically
  // - std::function can hold any callable with matching signature
  //;
  //
  // C++ CONCEPTS:
  // - .empty() returns true if container has no elements
  // - Lambda with no captures: []
  // - Remove if tags vector is NOT empty
  //
  // EXAMPLE STRUCTURE:
  //   view_.erase(
  //     std::remove_if(view_.begin(), view_.end(),
  //       [](const Task* t) { return !t->tags.empty(); }),
  //     view_.end()
  //   );
}

void Database::search_text(std::string_view text) {
  // Step 1: Convert search text to lowercase (for comparison)
  //   - Create std::string lower_text
  //   - Use std::transform with ::tolower
  //
  // Step 2: Create helper lambda for case-insensitive contains
  //   auto contains_ci = [](std::string_view haystack, std::string_view needle) {
  //     // Convert both to lowercase and use .find()
  //   };
  //
  // Step 3: Remove tasks that don't match
  //   view_.erase(
  //     std::remove_if(view_.begin(), view_.end(),
  //       [&](const Task* t) {
  //         bool in_title = contains_ci(t->title, lower_text);
  //         bool in_desc = t->description.has_value() &&
  //                        contains_ci(*t->description, lower_text);
  //         return !(in_title || in_desc);
  //       }),
  //     view_.end()
  //   );
  //
  // C++ CONCEPTS:
  // - std::transform(s.begin(), s.end(), s.begin(), ::tolower) lowercases string
  // - std::optional: use .has_value() to check, *opt to access
  // - string::find(substr) returns string::npos if not found
  //
  // EXAMPLE STRUCTURE (helper):
  //   auto to_lower = [](std::string s) {
  //     std::transform(s.begin(), s.end(), s.begin(),
  //       [](unsigned char c) { return std::tolower(c); });
  //     return s;
  //   };
  //
  //   std::string search_lower = to_lower(std::string(text));
  //   view_.erase(
  //     std::remove_if(view_.begin(), view_.end(),
  //       [&](const Task* t) {
  //         std::string title_lower = to_lower(t->title);
  //         bool match = title_lower.find(search_lower) != std::string::npos;
  //         if (t->description) {
  //           std::string desc_lower = to_lower(*t->description);
  //           match = match || (desc_lower.find(search_lower) != std::string::npos);
  //         }
  //         return !match;
  //       }),
  //     view_.end()
  //   );
}

void Database::replay_history(const std::vector<ViewAction> &actions) noexcept {
  reset_view();

  for (const auto &action : actions) {
    try {
      switch (action.type) {
      case ViewOpType::Filter: {
        auto spec = ExpressionParser::parse_filter(action.payload);
        if (spec) {
          apply_filter(*spec);
          std::cerr << "[Replay] Applied filter: " << action.payload << "\n";
        } else {
          std::cerr << "[Replay] Failed to parse filter: " << action.payload << "\n";
        }
        break;
      }

      case ViewOpType::Sort: {
        auto spec = ExpressionParser::parse_sort(action.payload);
        if (spec) {
          apply_sort(*spec);
          std::cerr << "[Replay] Applied sort: " << action.payload << "\n";
        } else {
          std::cerr << "[Replay] Failed to parse sort: " << action.payload << "\n";
        }
        break;
      }

      case ViewOpType::FindByTag: {
        // Payload is the tag string directly
        filter_by_tag(action.payload);
        std::cerr << "[Replay] Applied tag filter: " << action.payload << "\n";
        break;
      }

      case ViewOpType::ResetFilters: {
        reset_view();
        std::cerr << "[Replay] Reset filters\n";
        break;
      }

      case ViewOpType::Load: {
        // Skip - load is handled by DataManager
        break;
      }

      default:
        std::cerr << "[Replay] Warning: Unknown action type\n";
      }
    } catch (const std::exception &e) {
      std::cerr << "Error replaying action: " << e.what() << std::endl;
    }
  }
}

// ============================================================================
// Data Access
// ============================================================================

const Task *Database::get_task_by_id(int id) const noexcept {
  auto it = tasks_.find(id);
  if (it != tasks_.end()) {
    return &it->second;
  } else {
    return nullptr;
  }
}

// ============================================================================
// Aggregations and Statistics
// ============================================================================

StatusStats Database::status_stats() const noexcept {
  StatusStats stats;

  for (const Task *task : view_) {
    // Check task->status and increment appropriate counter
    if (task->status == "todo")
      stats.todo_count++;
    else if (task->status == "in-progress")
      stats.in_progress_count++;
    else if (task->status == "done")
      stats.done_count++;
    else
      stats.other_count++;
  }

  return stats;
}

double Database::average_priority() const noexcept {
  // Handle empty view
  if (view_.empty())
    return 0.0;

  // Sum all priorities
  double sum =
      std::accumulate(view_.begin(), view_.end(), 0.0, [](int acc, const Task *task) { return acc + task->priority; });

  return sum / view_.size();
}

size_t Database::overdue_count(std::string_view today_iso) const noexcept {
  // Step 1: Use std::count_if with complex predicate
  //   return std::count_if(view_.begin(), view_.end(),
  //     [today_iso](const Task* t) {
  //       // Check all three conditions
  //     });
  //
  // Step 2: Implement predicate logic
  //   - Check t->due_date.has_value()
  //   - Check *t->due_date < today_iso (ISO dates compare lexically)
  //   - Check t->status != "done"

  return 0;
  //
  // C++ CONCEPTS:
  // - std::count_if counts elements matching predicate
  // - std::optional: .has_value() check before dereferencing
  // - ISO 8601 dates: "YYYY-MM-DD" format compares correctly as strings
  // - Lambda captures: [today_iso] captures by value
  //
  // EXAMPLE STRUCTURE:
  //   return std::count_if(view_.begin(), view_.end(),
  //     [today_iso](const Task* t) {
  //       if (!t->due_date.has_value()) return false;
  //       if (t->status == "done") return false;
  //       return *t->due_date < today_iso;
  //     });
}

// ============================================================================
// Internal Helpers
// ============================================================================

void Database::rebuild_indices() noexcept {
  // Clear existing indices
  status_index_.clear();
  tag_index_.clear();

  // Iterate through all tasks and populate indices
  for (const auto &[id, task] : tasks_) {
    // Add to status index
    status_index_[task.status].push_back(&task);
    // Add to tag index (for each tag)
    for (const auto &tag : task.tags) {
      tag_index_[tag].push_back(&task);
    }
  }
}

std::function<bool(const Task *, const Task *)> Database::make_comparator(const SortSpec &sort) const {
  switch (sort.field) {
  case SortField::Priority:
    return sort.direction == SortDirection::Ascending
               ? [](const Task *a, const Task *b) { return a->priority < b->priority; }
               : [](const Task *a, const Task *b) { return a->priority > b->priority; };
  case SortField::Title:
    return sort.direction == SortDirection::Ascending
               ? [](const Task *a, const Task *b) { return a->title < b->title; }
               : [](const Task *a, const Task *b) { return a->title > b->title; };
  case SortField::Status:
    return sort.direction == SortDirection::Ascending
               ? [](const Task *a, const Task *b) { return a->status < b->status; }
               : [](const Task *a, const Task *b) { return a->status > b->status; };
  default:
    return sort.direction == SortDirection::Ascending ? [](const Task *a, const Task *b) { return a->id < b->id; }
                                                      : [](const Task *a, const Task *b) { return a->id > b->id; };
  }

  // TODO: Handle optional fields (due_date, assignee)
  //   Strategy: Put tasks with missing values at the end
  //
  //   if (!a->due_date && !b->due_date) return false;  // equal
  //   if (!a->due_date) return false;  // a goes after b
  //   if (!b->due_date) return true;   // b goes after a
  //   return *a->due_date < *b->due_date;  // normal comparison
}

std::function<bool(const Task *)> Database::make_predicate(const FilterSpec &filter) const {
  switch (filter.field) {
  case FilterField::Priority:
    return [filter](const Task *t) {
      int target = std::stoi(filter.value);
      switch (filter.op) {
      case FilterOp::Equal:
        return t->priority == target;
      case FilterOp::NotEqual:
        return t->priority != target;
      case FilterOp::GreaterThanOrEqual:
        return t->priority >= target;
      case FilterOp::LessThanOrEqual:
        return t->priority <= target;
      case FilterOp::GreaterThan:
        return t->priority > target;
      case FilterOp::LessThan:
        return t->priority < target;
      default:
        return false;
      }
    };
  case FilterField::Status:
    return [filter](const Task *t) {
      std::string target = filter.value;
      switch (filter.op) {
      case FilterOp::Equal:
        return t->status == target;
      case FilterOp::NotEqual:
        return t->status != target;
      default:
        return false;
      }
    };
  case FilterField::Title:
    return [filter](const Task *t) {
      std::string target = filter.value;
      switch (filter.op) {
      case FilterOp::Equal:
        return t->title == target;
      case FilterOp::NotEqual:
        return t->title != target;
      default:
        return false;
      }
    };
  case FilterField::CreatedDate:
    return [filter](const Task *t) {
      std::string target = filter.value;
      switch (filter.op) {
      case FilterOp::Equal:
        return t->created_date == target;
      case FilterOp::NotEqual:
        return t->created_date != target;
      default:
        return false;
      }
    };
  default:
    return [](const Task *) { return true; };
  }

  // TODO: Handle optional fields (due_date, assignee, description)
  //
  //   Example for DueDate + LessThan:
  //     std::string date = filter.value;
  //     return [date](const Task* t) {
  //       if (!t->due_date.has_value()) return false;  // No date = doesn't match
  //       return *t->due_date < date;
  //     };
  //
}
