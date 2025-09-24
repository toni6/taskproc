#include "cli/parser.hpp"
#include "core/data_manager.hpp"
#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {
  // Parse command line arguments
  auto parsed = CommandParser::parse(argc, argv);

  // Handle parsing error
  if (!parsed.is_valid()) {
    if (!parsed.error_message.empty()) {
      cerr << "Error: " << parsed.error_message << "\n";
    }
    CommandParser::print_usage(argv[0]);
    return 1;
  }

  // Handle help command
  if (parsed.command == Command::Help) {
    CommandParser::print_help(argv[0]);
    return 0;
  }

  DataManager data_manager = DataManager();

  // Dispatch to appropriate command handler
  switch (parsed.command) {
  case Command::Load: {
    cout << "Loading tasks from: " << parsed.args[0] << "\n";
    bool result = data_manager.loadFromFile(parsed.args[0]);
    if (!result) {
      cerr << "Failed to load tasks from file: " << parsed.args[0] << "\n";
      return 1;
    } else {
      cout << "Tasks loaded successfully\n";
    }
    break;
  }
  case Command::Reload: {
    cout << "Reloading from last file\n";
    bool result = data_manager.reloadTasks();
    if (!result) {
      cerr << "Failed to reload tasks\n";
      return 1;
    } else {
      cout << "Tasks reloaded successfully\n";
    }
    break;
  }
  case Command::Status:
    cout << "Current dataset status:\n";
    // TOOD: Implement status command handler
    break;
  case Command::Clear:
    cout << "Clearing current dataset\n";
    // TODO: Implement clear command handler
    break;
  default:
    // Invalid command
    break;
  }

  return 0;
}
