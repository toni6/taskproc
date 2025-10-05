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

  DataManager data_manager;

  // Dispatch to appropriate command handler
  switch (parsed.command) {
  case Command::Load: {
    std::cout << "Loading tasks from: " << parsed.args[0] << "\n";
    bool result = data_manager.load_from_file(parsed.args[0]);
    if (!result) {
      std::cerr << "Failed to load tasks from file: " << parsed.args[0] << "\n";
      return 1;
    }
    std::cout << "Tasks loaded successfully\n";
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
    // TODO: Implement status command handler
    break;
  case Command::Clear:
    std::cout << "Clearing current view\n";
    data_manager.reset_view();
    break;
  case Command::List: {
    const auto &view = data_manager.current_view();
    std::cout << "Current view:\n";

    if (view.empty()) {
      std::cout << "No tasks in current view\n";
      break;
    }

    std::cout << "Current tasks (" << view.size() << "):\n";
    std::cout << "-------------------------\n";

    for (const auto &task : view) {
      std::cout << *task << "\n";
    }

    break;
  }
  case Command::Sort: {
    std::cout << "Sorting current view\n";
    std::string sort_expr;

    if (parsed.args.empty()) {
      sort_expr = "id asc";
    } else {
      sort_expr = parsed.args[0];
      for (size_t i = 1; i < parsed.args.size(); ++i) {
        sort_expr += " " + parsed.args[i];
      }
    }
    std::cout << "Sorting tasks by: " << sort_expr << "\n";
    bool result = data_manager.apply_sort(sort_expr);
    if (!result) {
      std::cerr << "Failed to sort tasks\n";
      return 1;
    } else {
      std::cout << "Tasks sorted successfully\n";
    }
    break;
  }
  case Command::Filter: {
    std::cout << "Filtering current view\n";
    bool result = data_manager.apply_filter(parsed.args[0]);
    if (!result) {
      std::cerr << "Failed to filter tasks\n";
      return 1;
    } else {
      std::cout << "Tasks filtered successfully\n";
    }
    break;
  }

  default:
    // Invalid command
    break;
  }

  return 0;
}
