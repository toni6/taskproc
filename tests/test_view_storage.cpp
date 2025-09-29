#include "io/view_storage.hpp"
#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <string>
#include <system_error>

// Small RAII helper for a temporary directory used as the process CWD.
// On construction it creates a temp directory and switches CWD to it.
// On destruction it restores the previous CWD and removes the temp directory.
struct TempCwd {
  std::filesystem::path dir;
  std::filesystem::path prev;
  TempCwd() {
    prev = std::filesystem::current_path();
    dir = std::filesystem::temp_directory_path() /
          ("taskproc_test_view_storage_" + std::to_string(std::hash<std::string>{}(std::to_string(
                                               std::chrono::steady_clock::now().time_since_epoch().count()))));
    std::filesystem::create_directories(dir);
    std::filesystem::current_path(dir);
  }
  ~TempCwd() {
    std::error_code ec;
    std::filesystem::current_path(prev);
    std::filesystem::remove_all(dir, ec);
    (void)ec;
  }
};

// Simple helper that checks whether the storage file exists in the current CWD.
static bool storage_file_exists(const std::string &filename = ".taskproc.storage") {
  return std::filesystem::exists(std::filesystem::current_path() / filename);
}

TEST_CASE("ViewStorage in-memory operations", "[io][view_storage]") {
  ViewStorage vs;

  REQUIRE(!vs.filepath().has_value());
  REQUIRE(vs.history().empty());

  vs.set_filepath("data/tasks.csv");
  REQUIRE(vs.filepath().has_value());
  REQUIRE(vs.filepath().value() == std::filesystem::path("data/tasks.csv"));

  vs.push_action(ViewAction(ViewOpType::Filter, "priority<=3"));
  vs.push_action(ViewAction(ViewOpType::Sort, "due_date desc"));

  auto hist = vs.history();
  REQUIRE(hist.size() == 2);
  REQUIRE(hist[0].type == ViewOpType::Filter);
  REQUIRE(hist[0].payload == "priority<=3");
  REQUIRE(hist[1].type == ViewOpType::Sort);
  REQUIRE(hist[1].payload == "due_date desc");

  vs.clear_history();
  REQUIRE(vs.history().empty());
}

TEST_CASE("ViewStorage persist and load_from_storage round-trip", "[io][view_storage]") {
  TempCwd tmp; // work inside isolated temp dir so storage file is local and removed after test
  const std::string storage_name = ".taskproc.storage";
  REQUIRE(!storage_file_exists(storage_name));

  // Create and persist state
  {
    ViewStorage writer;
    writer.set_filepath("/absolute/or/relative/tasks.csv");
    writer.push_action(ViewAction(ViewOpType::Filter, "status=todo"));
    writer.push_action(ViewAction(ViewOpType::Sort, "priority desc"));
    writer.persist();

    // storage file should now exist
    REQUIRE(storage_file_exists(storage_name));
  }

  // New instance should be able to load the persisted state
  {
    ViewStorage reader;
    const bool loaded = reader.load_from_storage();
    REQUIRE(loaded);
    REQUIRE(reader.filepath().has_value());
    REQUIRE(reader.filepath().value() == std::filesystem::path("/absolute/or/relative/tasks.csv"));

    auto hist = reader.history();
    REQUIRE(hist.size() == 2);
    REQUIRE(hist[0].type == ViewOpType::Filter);
    REQUIRE(hist[0].payload == "status=todo");
    REQUIRE(hist[1].type == ViewOpType::Sort);
    REQUIRE(hist[1].payload == "priority desc");
  }
}

TEST_CASE("ViewStorage clear removes persisted file and clears memory", "[io][view_storage]") {
  TempCwd tmp;
  const std::string storage_name = ".taskproc.storage";

  // Persist a state first
  {
    ViewStorage writer;
    writer.set_filepath("somefile.csv");
    writer.push_action(ViewAction(ViewOpType::Filter, "priority<=2"));
    writer.persist();
    REQUIRE(storage_file_exists(storage_name));
  }

  // Now clear and ensure file is removed and in-memory state cleared
  {
    ViewStorage w;
    const bool loaded = w.load_from_storage();
    REQUIRE(loaded);
    REQUIRE(w.filepath().has_value());
    w.clear();
    REQUIRE(!w.filepath().has_value());
    REQUIRE(w.history().empty());

    // storage file should be removed by clear()
    REQUIRE(!storage_file_exists(storage_name));
  }
}
