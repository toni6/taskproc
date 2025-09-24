#include "io/csv_reader.hpp"
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

// Verify CVSReader::canHandle
TEST_CASE("CSVReader::canHandle checks", "[io][csv_reader]") {
  CSVReader reader;
  REQUIRE(reader.canHandle("test.csv"));
  REQUIRE(!reader.canHandle("test.txt"));
  REQUIRE(!reader.canHandle("test.json"));
}

// Verify CSVReader::readTasks parses file and tags
TEST_CASE("CSVReader::readTasks parses file and tags", "[io][csv_reader]") {
  using namespace std::string_literals;

  // Create a unique temporary filename in the system temp dir
  auto now = std::chrono::steady_clock::now().time_since_epoch().count();
  std::filesystem::path tmp =
      std::filesystem::temp_directory_path() / ("taskproc_test_"s + std::to_string(now) + ".csv");

  // Write a small CSV with header and two rows
  {
    std::ofstream ofs(tmp);
    REQUIRE(ofs.is_open());
    ofs << "id,title,status,priority,description,assignee,due_date,created_date,tags\n";
    ofs << "1,\"Fix login\",\"todo\",5,\"desc\",\"john\",\"2024-01-20\",\"2024-01-15\",\"bug,urgent,frontend\"\n";
    ofs << "2,\"Single tag\",\"done\",1,\"desc2\",\"jane\",\"2024-01-22\",\"2024-01-10\",\"tag1\"\n";
    ofs << "3,\"NoPriorityNoTags\",\"done\",,\"desc3\",\"jane\",\"2024-01-23\",\"2025-01-10\",\n";
    ofs << "4,\"Invalid\",,,\"desc4\",\"jane\",\"2024-01-23\",\"2025-01-10\",\n";
    ofs << ",\"Invalid\",1,,\"desc5\",\"jane\",\"2024-01-23\",\"2025-01-10\",\n";
  }

  // Read using CSVReader
  CSVReader reader;
  auto tasks = reader.readTasks(tmp.string());

  // Basic checks
  REQUIRE(tasks.size() == 3);

  // First task checks
  REQUIRE(tasks[0].id == 1);
  REQUIRE(tasks[0].title == "Fix login");
  REQUIRE(tasks[0].status == "todo");
  REQUIRE(tasks[0].tags.size() == 3);
  REQUIRE(tasks[0].tags[0] == "bug");
  REQUIRE(tasks[0].tags[1] == "urgent");
  REQUIRE(tasks[0].tags[2] == "frontend");

  // Second task checks
  REQUIRE(tasks[1].id == 2);
  REQUIRE(tasks[1].title == "Single tag");
  REQUIRE(tasks[1].tags.size() == 1);
  REQUIRE(tasks[1].tags[0] == "tag1");

  // Third task checks
  REQUIRE(tasks[2].id == 3);
  REQUIRE(tasks[2].title == "NoPriorityNoTags");
  REQUIRE(tasks[2].tags.size() == 0);
  REQUIRE(tasks[2].priority == 1);

  // Cleanup
  std::error_code ec;
  std::filesystem::remove(tmp, ec);
  REQUIRE(!ec); // ensure cleanup succeeded
}
