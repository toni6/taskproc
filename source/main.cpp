#include "cli/parser.hpp"
#include "core/data_manager.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
  // Parse command line arguments
  auto parsed = CommandParser::parse(argc, argv);

  // Handle parsing error
  if (!parsed.is_valid()) {
    if (!parsed.error_message.empty()) {
      std::cerr << "Error: " << parsed.error_message << "\n";
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
    std::cout << "Loading tasks from: " << parsed.args[0] << "\n";
    bool result = data_manager.load_from_file(parsed.args[0]);
    if (!result) {
      std::cerr << "Failed to load tasks from file: " << parsed.args[0] << "\n";
      return 1;
    } else {
      std::cout << "Tasks loaded successfully\n";
    }
    break;
  }
  case Command::Reload: {
    std::cout << "Reloading from last file\n";
    bool result = data_manager.reload_tasks();
    if (!result) {
      std::cerr << "Failed to reload tasks\n";
      return 1;
    } else {
      std::cout << "Tasks reloaded successfully\n";
    }
    break;
  }
  case Command::Status:
    std::cout << "Current dataset status:\n";
    // TOOD: Implement status command handler
    break;
  case Command::Clear:
    std::cout << "Clearing current dataset\n";
    data_manager.reset_storage();
    break;
  default:
    // Invalid command
    break;
  }

  return 0;
}
