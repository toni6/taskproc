#include "io/json_reader.hpp"
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

// Verify JSONReader::can_handle
TEST_CASE("JSONReader::can_handle checks", "[io][json_reader]") {
  JSONReader reader;
  REQUIRE(reader.can_handle("test.json"));
  REQUIRE(!reader.can_handle("test.txt"));
  REQUIRE(!reader.can_handle("test.csv"));
  REQUIRE(!reader.can_handle("test.json.gz"));
  REQUIRE(!reader.can_handle("test.json.bak"));
}

// Verify JSONReader::read_tasks parses file and tags
TEST_CASE("JSONReader::read_tasks parses file and tags", "[io][json_reader]") {
  using namespace std::string_literals;

  // Create a unique temporary filename in the system temp dir
  auto now = std::chrono::steady_clock::now().time_since_epoch().count();
  std::filesystem::path tmp =
      std::filesystem::temp_directory_path() / ("taskproc_json_test_"s + std::to_string(now) + ".json");

  // Write a small JSON array with several objects (some malformed)
  {
    std::ofstream ofs(tmp);
    REQUIRE(ofs.is_open());
    ofs << R"JSON([
  {
    "id": 1,
    "title": "Fix login page bug",
    "status": "todo",
    "priority": 5,
    "created_date": "2024-01-15",
    "description": "Users cannot log in with special characters in password",
    "assignee": "john.doe",
    "due_date": "2024-01-20",
    "tags": ["bug", "urgent", "frontend"]
  },
  {
    "id": 2,
    "title": "Single tag",
    "status": "done",
    "priority": 1,
    "created_date": "2024-01-10",
    "description": "Create personalized dashboard for user metrics",
    "assignee": "jane.smith",
    "due_date": "2024-01-25",
    "tags": ["tag1"]
  },
  {
    "id": 3,
    "title": "NoPriorityNoTags",
    "status": "done",
    "created_date": "2025-01-10"
  },
  {
    "id": 0,
    "title": "Invalid",
    "status": "todo"
  },
  {
    "id": 4,
    "title": "",
    "status": "todo"
  }
])JSON";
  }

  // Read using JSONReader
  JSONReader reader;
  auto tasks = reader.read_tasks(tmp.string());

  // Only the three valid tasks should be present
  REQUIRE(tasks.size() == 3);

  // First task checks
  REQUIRE(tasks[0].id == 1);
  REQUIRE(tasks[0].title == "Fix login page bug");
  REQUIRE(tasks[0].status == "todo");
  REQUIRE(tasks[0].priority == 5);
  REQUIRE(tasks[0].created_date == "2024-01-15");
  REQUIRE(tasks[0].description.has_value());
  REQUIRE(*tasks[0].description == "Users cannot log in with special characters in password");
  REQUIRE(tasks[0].assignee.has_value());
  REQUIRE(*tasks[0].assignee == "john.doe");
  REQUIRE(tasks[0].due_date.has_value());
  REQUIRE(*tasks[0].due_date == "2024-01-20");
  REQUIRE(tasks[0].tags.size() == 3);
  REQUIRE(tasks[0].tags[0] == "bug");
  REQUIRE(tasks[0].tags[1] == "urgent");
  REQUIRE(tasks[0].tags[2] == "frontend");

  // Second task checks
  REQUIRE(tasks[1].id == 2);
  REQUIRE(tasks[1].title == "Single tag");
  REQUIRE(tasks[1].tags.size() == 1);
  REQUIRE(tasks[1].tags[0] == "tag1");

  // Third task checks (no priority/tags provided -> defaults)
  REQUIRE(tasks[2].id == 3);
  REQUIRE(tasks[2].title == "NoPriorityNoTags");
  REQUIRE(tasks[2].tags.size() == 0);
  REQUIRE(tasks[2].priority == 1);

  // Cleanup
  std::error_code ec;
  std::filesystem::remove(tmp, ec);
  REQUIRE(!ec); // ensure cleanup succeeded
}
