# Task Processor CLI - MVP Specification

## Project Overview
A command-line tool for managing and processing task data from structured files. The tool reads task data, performs queries and transformations, maintains an in-memory database, and exports results.

## Core Features (MVP v0.1)

### 1. Data Input
- **CSV Support**: Read task data from CSV files
  - Required columns: id, title, status, priority, created_date
  - Optional columns: description, assignee, due_date, tags
- **JSON Support**: Read task data from JSON files (simple flat structure)
- **Error Handling**: Validate file format, handle missing files, malformed data

### 2. In-Memory Database
- **Storage**: Use `std::map<int, Task>` for primary storage (indexed by ID)
- **Indexing**: Additional `std::unordered_set` for status filtering
- **Data Model**:
  ```cpp
  struct Task {
      int id;
      std::string title;
      std::string status;        // "todo", "in-progress", "done"
      int priority;              // 1-5 (5 = highest)
      std::string created_date;  // ISO 8601 format
      std::optional<std::string> description;
      std::optional<std::string> assignee;
      std::optional<std::string> due_date;
      std::vector<std::string> tags;
  };
  ```

### 3. Command-Line Interface

**Stateful Subcommand-Based Design**:

```bash
# Data Loading and Management
taskproc load <file>              # Load tasks from CSV/JSON file into memory
taskproc reload                   # Reload from last loaded file
taskproc clear                    # Clear current dataset from memory
taskproc status                   # Show current dataset info (file, count, filters)

# Data Viewing and Querying
taskproc list                     # List all tasks in current dataset (formatted table)
taskproc show <id>                # Show detailed view of specific task
taskproc count                    # Show total task count in current view
taskproc stats                    # Show statistics (count by status, priority distribution)

# Filtering and Searching (modifies current view)
taskproc filter <criteria>        # Filter tasks by criteria (narrows current view)
taskproc search <text>            # Search in title and description
taskproc find-by-tag <tag>        # Find tasks with specific tag
taskproc reset-filters            # Clear all filters, show all loaded tasks

# Sorting (modifies current view order)
taskproc sort <field> [asc|desc]  # Sort current view by field (default: asc)

# Data Export
taskproc export <file>            # Export current filtered/sorted view
taskproc export-all <file>        # Export all loaded tasks (ignore current filters)

# Combined Operations (pipeline style - execute in sequence)
taskproc load tasks.csv filter status=todo sort priority desc list
taskproc load sprint.json find-by-tag urgent export urgent_tasks.csv
```

**State Management**: The tool maintains loaded data and current filters/sorting in memory between commands until `clear` is called or a new `load` command is issued.

### 4. Query & Filter Operations
- **Status Filter**: `status=todo`, `status=in-progress`, `status=done`
- **Priority Filter**: `priority>=3`, `priority=5`
- **Date Filter**: `created_after=2024-01-01`, `due_before=2024-12-31`
- **Text Search**: `title_contains=bug`, `assignee=john`
- **Tag Filter**: `has_tag=urgent`, `no_tags`

### 5. Data Transformations
- **Sorting**: By any field (id, title, priority, created_date, due_date)
- **Aggregation**: Count by status, average priority, overdue tasks
- **Formatting**: Table output, CSV export, JSON export

### 6. Output & Export
- **Console Output**: Formatted table view with column alignment
- **CSV Export**: Standard CSV format with all fields
- **JSON Export**: Pretty-printed JSON
- **Statistics**: Summary reports (count, percentages, etc.)

## Example Usage Scenarios

### Scenario 1: Daily Task Review
```bash
# Load today's tasks and explore interactively
taskproc load daily_tasks.csv
taskproc filter priority>=4
taskproc sort due_date
taskproc list

# Check statistics
taskproc stats
# Output: Current view: 12 tasks (5 todo, 3 in-progress, 4 done). Average priority: 4.2
```

### Scenario 2: Sprint Planning
```bash
# Find all urgent tasks and export for team
taskproc load sprint_tasks.json
taskproc find-by-tag urgent
taskproc export urgent_tasks.csv

# Get overdue tasks in priority order
taskproc reset-filters
taskproc filter due_before=2024-01-15
taskproc sort priority desc
taskproc list
```

### Scenario 3: Data Migration and Processing
```bash
# Convert JSON to CSV with filtering
taskproc load legacy_data.json
taskproc filter status!=cancelled
taskproc export clean_data.csv

# One-line processing pipeline
taskproc load raw_data.csv filter status=active sort created_date export processed.json
```

### Scenario 4: Interactive Exploration
```bash
# Load and explore dataset step by step
taskproc load project_tasks.csv
taskproc status                    # Show: "Loaded 1,245 tasks from project_tasks.csv"
taskproc stats                     # Overview of all data
taskproc filter status=todo        # Narrow down
taskproc count                     # Show: "156 tasks in current view"
taskproc sort priority desc        # Reorder
taskproc list                      # Display top priority todos
```

## Test Cases (MVP)

### Unit Tests
1. **Task Creation**: Valid/invalid task data
2. **File Parsing**: CSV/JSON parsing with edge cases
3. **Filtering**: All filter operations with boundary conditions
4. **Sorting**: All sortable fields with empty datasets
5. **Export**: Round-trip CSV→Memory→CSV integrity

### Integration Tests
1. **End-to-End**: Load file → filter → sort → export
2. **Large Dataset**: Performance with 10k+ tasks
3. **Error Handling**: Malformed files, invalid commands
4. **Memory Management**: No leaks under various operations

### Acceptance Tests
1. **CLI Interface**: All command combinations work as documented
2. **Data Integrity**: No data loss during transformations
3. **Performance**: Reasonable response time for typical datasets (<1000 tasks)

## Non-Goals (Future Versions)
- Database persistence (file-only for MVP)
- Web interface or API
- Advanced query language (SQL-like)
- Task modification commands (add/edit/delete)
- Multi-user support or permissions
- Real-time updates or notifications

## Success Criteria
- Handles CSV and JSON files up to 1MB
- Processes 1000 tasks in <100ms
- Zero memory leaks under sanitizers
- 90%+ test coverage for core functionality
- Builds cleanly on Linux, macOS, Windows
- Professional documentation and examples

## Architecture Overview

### Core Components
1. **CLI Parser**
   - Argument parsing and validation
   - Command dispatch
   - Help and usage generation

2. **File I/O**
   - CSV reader/writer
   - JSON reader/writer
   - Error handling and validation

3. **Task Management**
   - Task class definition
   - In-memory database
   - Query and filter engine

4. **Export System**
   - Template-based exporters
   - Format-specific implementations
   - Statistics generation

### Dependencies (Minimal)
- No external dependencies for core functionality
- Testing framework (Catch2)
- CMake for build system

## File Structure
```
taskproc/
├── source/
│   ├── CMakeLists.txt
│   ├── main.cpp
│   ├── cli/
│   │   ├── parser.hpp
│   │   └── parser.cpp
│   ├── core/
│   │   ├── data_manager.hpp (File loading, state management & reader coordination)
│   │   ├── data_manager.cpp
│   │   ├── task.hpp
│   │   ├── task.cpp
│   │   ├── database.hpp (Query/filter/sort operations)
│   │   └── database.cpp
│   ├── io/
│   │   ├── reader.hpp (ITaskReader interface)
│   │   ├── csv_reader.hpp
│   │   ├── csv_reader.cpp
│   │   ├── json_reader.hpp
│   │   └── json_reader.cpp
│   └── export/
│       ├── exporter.hpp
│       └── table_formatter.cpp
├── tests/
│   ├── test_main.cpp
│   └── ...
├── examples/
│   ├── sample_tasks.csv
│   ├── sample_tasks.json
│   └── ...
├── .clang
├── .gitignore
├── README.md
└── MVP.md (this file)
```
