# Coding conventions: Pre/Post conditions and small-function contracts

This file documents the project's lightweight convention for describing function
preconditions and postconditions. These conventions are intentionally simple —
they're meant to be used for small helper functions and public APIs so callers
clearly know what to expect.

Why
- Makes intent explicit for callers and reviewers.
- Improves testability: tests can assert postconditions given valid preconditions.
- Helps maintainers avoid surprising behavior during refactors.

Style
- Put the canonical contract in the header above the declaration using Doxygen
  tags: `@pre`, `@post`, `@throws`.
- Keep each `@pre` / `@post` short and focused (one line each).
- Use `@note` for extra semantics (ownership, thread-safety).
- For internal helpers, a one-line comment in the implementation is fine.

Template
- Use this template for public functions:

  /**
   * @brief Short summary of the function.
   * @pre <condition caller must satisfy before calling>
   * @post <guarantee after successful return>
   * @throws <exceptions that may be thrown>
   * @note <ownership/lifetime/threading notes>
   */

Examples

1) DataManager::loadFromFile

/**
 * @brief Load tasks from `filepath` and replace manager's tasks on success.
 * @pre `filepath` is a path to a readable file and a matching reader exists.
 * @post On success: `tasks_` contains the loaded tasks and `current_filepath_` == `filepath`.
 * @post On failure: `tasks_` remains unchanged.
 * @throws std::exception for I/O/parse errors (caller can catch).
 */

2) Small helper: split_tags

// Pre: `tags_field` is the raw tag string (possibly empty).
// Post: returns vector<string> with zero or more tag tokens.

Practical tips
- If a function cannot guarantee leaving the object unchanged on failure, explicitly document that.
- Prefer throwing exceptions for I/O/parse errors (document them), or return error codes consistently.
- If a function returns raw pointers, document ownership (who must not delete the pointer).
- Use `@note` to call out concurrency expectations (e.g., "not thread-safe").

Applying the convention
- Add `@pre`/`@post` to public APIs in headers.
- Keep implementations free of long prose; implementation comments are for clarifying complex logic only.
