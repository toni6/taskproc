#include "core/database.hpp"
#include "core/task.hpp"
#include <catch2/catch_test_macros.hpp>

// ============================================================================
// Database Basic Operations Tests
// ============================================================================

TEST_CASE("Database load and access", "[core][database]") {
  Database db;

  SECTION("Empty database") {
    REQUIRE(db.empty());
    REQUIRE(db.total_task_count() == 0);
    REQUIRE(db.view_task_count() == 0);
    REQUIRE(db.current_view().empty());
    REQUIRE(db.get_task_by_id(1) == nullptr);
  }

  SECTION("Load tasks") {
    std::vector<Task> tasks;
    tasks.emplace_back(1, "Task One", "todo", 3, "2024-01-01");
    tasks.emplace_back(2, "Task Two", "done", 5, "2024-01-02");
    tasks.emplace_back(3, "Task Three", "in-progress", 2, "2024-01-03");

    db.load(std::move(tasks));

    REQUIRE(!db.empty());
    REQUIRE(db.total_task_count() == 3);
    REQUIRE(db.view_task_count() == 3);

    const Task *task1 = db.get_task_by_id(1);
    REQUIRE(task1 != nullptr);
    REQUIRE(task1->id == 1);
    REQUIRE(task1->title == "Task One");
    REQUIRE(task1->status == "todo");
    REQUIRE(task1->priority == 3);

    const Task *task_missing = db.get_task_by_id(999);
    REQUIRE(task_missing == nullptr);
  }

  SECTION("Load replaces existing tasks") {
    std::vector<Task> tasks1;
    tasks1.emplace_back(1, "First", "todo", 1);

    db.load(std::move(tasks1));
    REQUIRE(db.total_task_count() == 1);

    std::vector<Task> tasks2;
    tasks2.emplace_back(2, "Second", "done", 2);
    tasks2.emplace_back(3, "Third", "todo", 3);

    db.load(std::move(tasks2));
    REQUIRE(db.total_task_count() == 2);
    REQUIRE(db.get_task_by_id(1) == nullptr);
    REQUIRE(db.get_task_by_id(2) != nullptr);
  }
}

// ============================================================================
// Filter Tests
// ============================================================================

TEST_CASE("Database filtering", "[core][database]") {
  Database db;

  std::vector<Task> tasks;
  tasks.emplace_back(1, "Low priority", "todo", 1, "2024-01-01");
  tasks.emplace_back(2, "High priority", "todo", 5, "2024-01-02");
  tasks.emplace_back(3, "Medium priority", "done", 3, "2024-01-03");
  tasks.emplace_back(4, "Another high", "in-progress", 5, "2024-01-04");

  db.load(std::move(tasks));

  SECTION("Filter by priority equal") {
    FilterSpec filter{FilterField::Priority, FilterOp::Equal, "5"};
    db.apply_filter(filter);

    REQUIRE(db.view_task_count() == 2);
    auto view = db.current_view();
    REQUIRE(view[0]->priority == 5);
    REQUIRE(view[1]->priority == 5);
  }

  SECTION("Filter by priority greater than or equal") {
    FilterSpec filter{FilterField::Priority, FilterOp::GreaterThanOrEqual, "3"};
    db.apply_filter(filter);

    REQUIRE(db.view_task_count() == 3);
    for (const auto *task : db.current_view()) {
      REQUIRE(task->priority >= 3);
    }
  }

  SECTION("Filter by status") {
    FilterSpec filter{FilterField::Status, FilterOp::Equal, "todo"};
    db.apply_filter(filter);

    REQUIRE(db.view_task_count() == 2);
    for (const auto *task : db.current_view()) {
      REQUIRE(task->status == "todo");
    }
  }

  SECTION("Cumulative filters") {
    FilterSpec filter1{FilterField::Priority, FilterOp::GreaterThanOrEqual, "3"};
    FilterSpec filter2{FilterField::Status, FilterOp::Equal, "todo"};

    db.apply_filter(filter1);
    REQUIRE(db.view_task_count() == 3);

    db.apply_filter(filter2);
    REQUIRE(db.view_task_count() == 1);
    REQUIRE(db.current_view()[0]->title == "High priority");
  }

  SECTION("Reset view clears filters") {
    FilterSpec filter{FilterField::Status, FilterOp::Equal, "todo"};
    db.apply_filter(filter);
    REQUIRE(db.view_task_count() == 2);

    db.reset_view();
    REQUIRE(db.view_task_count() == 4);
  }
}

// ============================================================================
// Sort Tests
// ============================================================================

TEST_CASE("Database sorting", "[core][database]") {
  Database db;

  std::vector<Task> tasks;
  tasks.emplace_back(1, "Charlie", "todo", 3, "2024-01-03");
  tasks.emplace_back(2, "Alice", "done", 1, "2024-01-01");
  tasks.emplace_back(3, "Bob", "in-progress", 5, "2024-01-02");

  db.load(std::move(tasks));

  SECTION("Sort by priority ascending") {
    SortSpec sort{SortField::Priority, SortDirection::Ascending};
    db.apply_sort(sort);

    auto view = db.current_view();
    REQUIRE(view.size() == 3);
    REQUIRE(view[0]->priority == 1);
    REQUIRE(view[1]->priority == 3);
    REQUIRE(view[2]->priority == 5);
  }

  SECTION("Sort by priority descending") {
    SortSpec sort{SortField::Priority, SortDirection::Descending};
    db.apply_sort(sort);

    auto view = db.current_view();
    REQUIRE(view.size() == 3);
    REQUIRE(view[0]->priority == 5);
    REQUIRE(view[1]->priority == 3);
    REQUIRE(view[2]->priority == 1);
  }

  SECTION("Sort by title ascending") {
    SortSpec sort{SortField::Title, SortDirection::Ascending};
    db.apply_sort(sort);

    auto view = db.current_view();
    REQUIRE(view.size() == 3);
    REQUIRE(view[0]->title == "Alice");
    REQUIRE(view[1]->title == "Bob");
    REQUIRE(view[2]->title == "Charlie");
  }

  SECTION("Sort by id descending") {
    SortSpec sort{SortField::Id, SortDirection::Descending};
    db.apply_sort(sort);

    auto view = db.current_view();
    REQUIRE(view.size() == 3);
    REQUIRE(view[0]->id == 3);
    REQUIRE(view[1]->id == 2);
    REQUIRE(view[2]->id == 1);
  }
}

// ============================================================================
// Statistics Tests
// ============================================================================

TEST_CASE("Database statistics", "[core][database]") {
  Database db;

  std::vector<Task> tasks;
  tasks.emplace_back(1, "Task 1", "todo", 2, "2024-01-01");
  tasks.emplace_back(2, "Task 2", "done", 4, "2024-01-02");
  tasks.emplace_back(3, "Task 3", "in-progress", 3, "2024-01-03");
  tasks.emplace_back(4, "Task 4", "todo", 5, "2024-01-04");
  tasks.emplace_back(5, "Task 5", "done", 1, "2024-01-05");

  db.load(std::move(tasks));

  SECTION("Status stats") {
    auto stats = db.status_stats();
    REQUIRE(stats.todo_count == 2);
    REQUIRE(stats.done_count == 2);
    REQUIRE(stats.in_progress_count == 1);
    REQUIRE(stats.other_count == 0);
    REQUIRE(stats.total() == 5);
  }

  SECTION("Status stats after filter") {
    FilterSpec filter{FilterField::Status, FilterOp::Equal, "todo"};
    db.apply_filter(filter);

    auto stats = db.status_stats();
    REQUIRE(stats.todo_count == 2);
    REQUIRE(stats.done_count == 0);
    REQUIRE(stats.in_progress_count == 0);
    REQUIRE(stats.total() == 2);
  }

  SECTION("Average priority") {
    double avg = db.average_priority();
    REQUIRE(avg == 3.0); // (2+4+3+5+1) / 5 = 15/5 = 3.0
  }

  SECTION("Average priority of empty view") {
    FilterSpec filter{FilterField::Status, FilterOp::Equal, "nonexistent"};
    db.apply_filter(filter);

    double avg = db.average_priority();
    REQUIRE(avg == 0.0);
  }
}

// ============================================================================
// Tag Filter Tests
// ============================================================================

TEST_CASE("Database tag filtering", "[core][database]") {
  Database db;

  std::vector<Task> tasks;
  tasks.emplace_back(1,
                     "Task 1",
                     "todo",
                     1,
                     "2024-01-01",
                     std::nullopt,
                     std::nullopt,
                     std::nullopt,
                     std::vector<std::string>{"urgent", "bug"});
  tasks.emplace_back(2,
                     "Task 2",
                     "done",
                     2,
                     "2024-01-02",
                     std::nullopt,
                     std::nullopt,
                     std::nullopt,
                     std::vector<std::string>{"feature"});
  tasks.emplace_back(
      3, "Task 3", "todo", 3, "2024-01-03", std::nullopt, std::nullopt, std::nullopt, std::vector<std::string>{});
  tasks.emplace_back(4,
                     "Task 4",
                     "todo",
                     4,
                     "2024-01-04",
                     std::nullopt,
                     std::nullopt,
                     std::nullopt,
                     std::vector<std::string>{"urgent", "feature"});

  db.load(std::move(tasks));

  // TODO: uncomment filter_by_tag tests
  // SECTION("Filter by tag 'urgent'") {
  //   db.filter_by_tag("urgent");
  //   REQUIRE(db.view_task_count() == 2);
  //   for (const auto *task : db.current_view()) {
  //     bool has_urgent = false;
  //     for (const auto &tag : task->tags) {
  //       if (tag == "urgent") {
  //         has_urgent = true;
  //         break;
  //       }
  //     }
  //     REQUIRE(has_urgent);
  //   }
  // }

  // SECTION("Filter by tag 'feature'") {
  //   db.filter_by_tag("feature");
  //   REQUIRE(db.view_task_count() == 2);
  // }

  // SECTION("Filter no tags") {
  //   db.filter_no_tags();
  //   REQUIRE(db.view_task_count() == 1);
  //   REQUIRE(db.current_view()[0]->tags.empty());
  // }
}
