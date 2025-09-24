#include "core/data_manager.hpp"
#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

// RAII helper to ensure temp files are removed even if the test aborts.
struct TempFile {
  std::filesystem::path path;
  explicit TempFile(std::filesystem::path p) : path(std::move(p)) {}
  ~TempFile() {
    std::error_code ec;
    std::filesystem::remove(path, ec);
  }
};

// Verify DataManager load/reload/select behavior
TEST_CASE("DataManager load and reload", "[core][data_manager]") {
  DataManager dm;

  // reload without load should fail
  REQUIRE(!dm.reload_tasks());

  SECTION("load and reload on same instance succeeds") {
    // Create a minimal CSV file the CSVReader can parse
    auto tmp_csv = std::filesystem::temp_directory_path() / "taskproc_dm_test.csv";
    TempFile tf(tmp_csv);

    {
      std::ofstream ofs(tmp_csv);
      REQUIRE(ofs.is_open());
      ofs << "id,title,status,priority,description,assignee,due_date,created_date,tags\n";
      ofs << "1,One,todo,1,desc,me,2025-01-01,2024-01-01,tag1\n";
    }

    // load_from_file should succeed (CSV reader exists)
    REQUIRE(dm.load_from_file(tmp_csv.string()));
    REQUIRE(dm.task_count() > 0);
    REQUIRE(dm.current_file_path() == tmp_csv.string());

    // reload_tasks should succeed now (uses stored current_filepath_)
    REQUIRE(dm.reload_tasks());
    REQUIRE(dm.task_count() > 0);
    REQUIRE(dm.current_file_path() == tmp_csv.string());
  }

  SECTION("unsupported extension returns false") {
    // This is a separate concern: verify that no reader is found for unknown extension
    auto tmp_unknown = std::filesystem::temp_directory_path() / "taskproc_dm_test.unknown";
    TempFile tf2(tmp_unknown);

    {
      std::ofstream ofs(tmp_unknown);
      REQUIRE(ofs.is_open());
      ofs << "garbage\n";
    }

    REQUIRE(!dm.load_from_file(tmp_unknown.string()));
  }

  SECTION("DataManager reload across instances (simulates separate processes)") {
    // Create a minimal CSV file the CSVReader can parse
    auto tmp_csv = std::filesystem::temp_directory_path() / "taskproc_dm_test.csv";
    TempFile tf(tmp_csv);

    {
      // First DataManager instance loads the file and is then destroyed at scope end
      DataManager dm_first;
      {
        std::ofstream ofs(tmp_csv);
        REQUIRE(ofs.is_open());
        ofs << "id,title,status,priority,description,assignee,due_date,created_date,tags\n";
        ofs << "1,One,todo,1,desc,me,2025-01-01,2024-01-01,tag1\n";
      }

      REQUIRE(dm_first.load_from_file(tmp_csv.string()));
      REQUIRE(dm_first.reload_tasks());
    } // dm_first is destroyed here

    // New DataManager instance must obtain filepath from storage
    DataManager dm_new;
    REQUIRE(dm_new.reload_tasks());
  }
}

// Verify DataManager accessors reflect empty state before any load
TEST_CASE("DataManager accessors when empty", "[core][data_manager]") {
  DataManager dm_empty;
  REQUIRE(dm_empty.task_count() == 0);
  REQUIRE(dm_empty.current_file_path().empty());
}
