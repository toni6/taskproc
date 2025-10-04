#include "core/data_manager.hpp"
#include "core/task.hpp"
#include "io/view_storage.hpp"
#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <utility>

// ============================================================================
// Test Helpers
// ============================================================================

struct TempFile {
  std::filesystem::path path;
  explicit TempFile(std::filesystem::path p) : path(std::move(p)) {}
  ~TempFile() {
    std::error_code ec;
    std::filesystem::remove(path, ec);
  }
};

// Create a minimal valid CSV file for testing
static void create_test_csv(const std::filesystem::path &path) {
  std::ofstream ofs(path);
  ofs << "id,title,status,priority,description,assignee,due_date,created_date,tags\n";
  ofs << "1,TestTask,todo,1,desc,user,2025-01-01,2024-01-01,tag1\n";
  ofs << "2,AnotherTask,done,2,desc2,user2,2025-01-02,2024-01-02,tag2\n";
}

// ============================================================================
// Task Ownership Tests
// ============================================================================

TEST_CASE("Task value semantics", "[ownership][task]") {
  SECTION("Task is copyable") {
    Task t1(1, "Original", "todo", 5);
    Task t2 = t1; // Copy

    REQUIRE(t2.id == t1.id);
    REQUIRE(t2.title == t1.title);
    REQUIRE(t2.status == t1.status);
    REQUIRE(t2.priority == t1.priority);

    // Modify copy - original unchanged
    t2.title = "Modified";
    REQUIRE(t1.title == "Original");
    REQUIRE(t2.title == "Modified");
  }

  SECTION("Task is movable") {
    Task t1(1,
            "Original",
            "todo",
            5,
            "2024-01-01",
            "Long description string",
            "assignee@example.com",
            "2025-12-31",
            {"tag1", "tag2", "tag3"});

    std::string original_title = t1.title;
    Task t2 = std::move(t1); // Move

    REQUIRE(t2.title == original_title);
    REQUIRE(t2.id == 1);
    REQUIRE(t2.tags.size() == 3);

    // t1 is in moved-from state (valid but unspecified)
    // We can still access it, but values are unspecified
    (void)t1.id; // This is safe, just reading moved-from object
  }

  SECTION("Task can be stored in STL containers") {
    std::vector<Task> tasks;
    tasks.emplace_back(1, "Task1", "todo", 1);
    tasks.emplace_back(2, "Task2", "done", 2);

    REQUIRE(tasks.size() == 2);
    REQUIRE(tasks[0].id == 1);
    REQUIRE(tasks[1].id == 2);

    // Container owns the tasks
    std::vector<Task> tasks2 = std::move(tasks);
    REQUIRE(tasks2.size() == 2);
    REQUIRE(tasks.empty()); // tasks moved-from (guaranteed empty for vector)
  }
}

// ============================================================================
// DataManager Ownership Tests
// ============================================================================

TEST_CASE("DataManager ownership", "[ownership][data_manager]") {
  SECTION("DataManager is movable") {
    auto tmp_path = std::filesystem::temp_directory_path() / "ownership_test_move.csv";
    TempFile tf(tmp_path);
    create_test_csv(tmp_path);

    DataManager dm1;
    REQUIRE(dm1.load_from_file(tmp_path.string()));
    REQUIRE(dm1.task_count() == 2);

    // Move dm1 into dm2
    DataManager dm2 = std::move(dm1);

    // dm2 now owns everything
    REQUIRE(dm2.task_count() == 2);
    REQUIRE(dm2.current_file_path() == tmp_path.string());

    // dm1 is in moved-from state (valid but unspecified)
    // Safe to access, but values are unspecified
    (void)dm1.task_count(); // This is safe
  }
}

TEST_CASE("DataManager task ownership", "[ownership][data_manager]") {
  SECTION("DataManager owns tasks by value") {
    auto tmp_path = std::filesystem::temp_directory_path() / "ownership_test_tasks.csv";
    TempFile tf(tmp_path);
    create_test_csv(tmp_path);

    DataManager dm;
    REQUIRE(dm.load_from_file(tmp_path.string()));
    REQUIRE(dm.task_count() == 2);

    // Tasks are owned by dm's internal map
    // They are copied/moved as needed, no shared ownership
  }

  SECTION("Loading new tasks replaces old ones atomically") {
    auto tmp_path1 = std::filesystem::temp_directory_path() / "ownership_test1.csv";
    auto tmp_path2 = std::filesystem::temp_directory_path() / "ownership_test2.csv";
    TempFile tf1(tmp_path1);
    TempFile tf2(tmp_path2);

    // Create first file with 2 tasks
    create_test_csv(tmp_path1);

    // Create second file with 1 task
    {
      std::ofstream ofs(tmp_path2);
      ofs << "id,title,status,priority,description,assignee,due_date,created_date,tags\n";
      ofs << "100,NewTask,todo,1,desc,user,2025-01-01,2024-01-01,tag1\n";
    }

    DataManager dm;
    REQUIRE(dm.load_from_file(tmp_path1.string()));
    REQUIRE(dm.task_count() == 2);

    // Load second file - old tasks replaced
    REQUIRE(dm.load_from_file(tmp_path2.string()));
    REQUIRE(dm.task_count() == 1);

    // Old tasks automatically cleaned up (no manual delete needed)
  }

  SECTION("reset_storage clears all owned data") {
    auto tmp_path = std::filesystem::temp_directory_path() / "ownership_test_reset.csv";
    TempFile tf(tmp_path);
    create_test_csv(tmp_path);

    DataManager dm;
    REQUIRE(dm.load_from_file(tmp_path.string()));
    REQUIRE(dm.task_count() == 2);
    REQUIRE(!dm.current_file_path().empty());

    // Reset clears everything
    dm.reset_storage();
    REQUIRE(dm.task_count() == 0);
    REQUIRE(dm.current_file_path().empty());

    // All resources automatically cleaned up
  }
}

// ============================================================================
// ViewStorage Ownership Tests
// ============================================================================

TEST_CASE("ViewStorage value ownership", "[ownership][view_storage]") {
  SECTION("ViewStorage owns data by value") {
    ViewStorage vs;

    vs.set_filepath("/path/to/file.csv");
    vs.push_action(ViewAction{ViewOpType::Filter, "priority<=3"});
    vs.push_action(ViewAction{ViewOpType::Sort, "due_date desc"});

    // ViewStorage owns the path and history
    REQUIRE(vs.filepath().has_value());
    REQUIRE(vs.history().size() == 2);

    // When vs goes out of scope, everything cleaned up automatically
  }

  SECTION("ViewStorage is movable") {
    ViewStorage vs1;
    vs1.set_filepath("/path/to/file.csv");
    vs1.push_action(ViewAction{ViewOpType::Filter, "test"});

    // Move vs1 into vs2
    ViewStorage vs2 = std::move(vs1);

    // vs2 now owns the data
    REQUIRE(vs2.filepath().has_value());
    REQUIRE(vs2.history().size() == 1);

    // vs1 is in moved-from state (valid but unspecified)
  }

  SECTION("history() returns const reference for observation") {
    ViewStorage vs;
    vs.push_action(ViewAction{ViewOpType::Filter, "test"});

    // history() returns const reference - no ownership transfer
    const std::vector<ViewAction> &hist = vs.history();
    REQUIRE(hist.size() == 1);

    // hist is a reference, not a copy - vs still owns the data
    // hist becomes invalid when vs is destroyed
  }

  SECTION("ViewAction has value semantics") {
    ViewAction action1{ViewOpType::Filter, "priority<=3"};
    ViewAction action2 = action1; // Copy

    REQUIRE(action1.type == action2.type);
    REQUIRE(action1.payload == action2.payload);

    // Modify copy
    action2.payload = "modified";
    REQUIRE(action1.payload == "priority<=3");
    REQUIRE(action2.payload == "modified");
  }
}

// ============================================================================
// Move Semantics Verification
// ============================================================================

TEST_CASE("Move semantics preserve correctness", "[ownership][move]") {
  SECTION("Moving Task preserves data") {
    Task original(1,
                  "Original Title",
                  "todo",
                  5,
                  "2024-01-01",
                  "Long description that would be expensive to copy",
                  "user@example.com",
                  "2025-12-31",
                  {"tag1", "tag2", "tag3", "tag4"});

    std::string expected_title = original.title;
    int expected_tag_count = static_cast<int>(original.tags.size());

    Task moved = std::move(original);

    // Moved-to object has the data
    REQUIRE(moved.title == expected_title);
    REQUIRE(static_cast<int>(moved.tags.size()) == expected_tag_count);

    // original is in moved-from state (valid but unspecified)
    // We don't test its state - it's implementation-defined
  }

  SECTION("Moving ViewStorage preserves data") {
    ViewStorage original;
    original.set_filepath("/test/path.csv");
    original.push_action(ViewAction{ViewOpType::Filter, "test_filter"});
    original.push_action(ViewAction{ViewOpType::Sort, "test_sort"});

    ViewStorage moved = std::move(original);

    // Moved-to object has the data
    REQUIRE(moved.filepath().has_value());
    REQUIRE(moved.filepath()->string() == "/test/path.csv");
    REQUIRE(moved.history().size() == 2);
    REQUIRE(moved.history()[0].payload == "test_filter");
  }
}
