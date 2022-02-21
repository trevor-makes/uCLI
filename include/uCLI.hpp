// Copyright (c) 2021 Trevor Makes

#pragma once

#include "uANSI.hpp"

#include <stdint.h>

namespace uCLI {

using uANSI::StreamEx;

using CommandFn = void (*)(StreamEx&, class Tokens);
using IdleFn = void (*)();

// Function pointer to be called when command string is entered
struct Command {
  const char* keyword;
  CommandFn callback;
};

class Tokens {
private:
  char* next_;

public:
  Tokens(char* args): next_{args} {}

  // Casting const literal "" to char* is a necessary evil for now so that we
  // can avoid dealing with nullptr; the empty string will NOT be mutated
  Tokens(): next_{const_cast<char*>("")} {}

  const char* next();
  bool has_next() { return *next_ != '\0'; }
  bool is_string() { return *next_ == '\"' || *next_ == '\''; }

  template <uint8_t N>
  uint8_t get(const char* (&argv)[N], bool are_strings[] = nullptr) {
    for (uint8_t i = 0; i < N; ++i) {
      if (!has_next()) {
        return i;
      }
      if (are_strings != nullptr) {
        are_strings[i] = is_string();
      }
      argv[i] = next();
    }
    return N;
  }

  // Attempt to match the next argument to a command in the list
  template <uint8_t CMD_LEN>
  void dispatch(StreamEx& stream, const Command (&commands)[CMD_LEN]) {
    const char* input = next();

    // Look for match in command list
    for (const Command& command : commands) {
      if (strcmp(input, command.keyword) == 0) {
        command.callback(stream, *this);
        return;
      }
    }

    // Otherwise, print help message
    stream.println("Commands:");
    for (const Command& command : commands) {
      stream.println(command.keyword);
    }
  }
};

class Cursor {
  char* buffer_;
  uint8_t limit_; // Maximum number of characters, excluding null terminator
  uint8_t cursor_ = 0; // Index where next character will be inserted
  uint8_t length_ = 0; // Number of characters currently in buffer

public:
  template <uint8_t N>
  Cursor(char (&buffer)[N]): Cursor(buffer, N) {}
  Cursor(char* buffer, uint8_t size): buffer_{buffer}, limit_{uint8_t(size - 1)} { buffer_[0] = '\0'; }

  // Prevent copying
  Cursor(const Cursor&) = delete;
  Cursor& operator=(const Cursor&) = delete;

  uint8_t length() const { return length_; }
  const char* contents() const { return buffer_; }
  char* contents() { return buffer_; }
  bool at_eol() const { return cursor_ == length_; }

  void clear() {
    cursor_ = length_ = 0;
    buffer_[0] = '\0';
  }

  // Insert at cursor up to size chars from input, returning count
  uint8_t try_insert(const char* input, uint8_t size = 255);

  // Attempt to insert at cursor, returning false if full
  bool try_insert(char input);

  // Attempt to delete at cursor, returning false if nothing to delete
  bool try_delete();

  // Attempt to move cursor left, returning false if already at margin
  bool try_left();

  // Attempt to move cursor right, returning false if already at margin
  bool try_right();

  // Move cursor to left margin, returning number of spaces moved
  uint8_t seek_home();

  // Move cursor to right margin, returning number of spaces moved
  uint8_t seek_end();
};

class History {
  char* buffer_;
  uint8_t size_;
  uint8_t entries_ = 0;
  uint8_t index_ = 0;

  void copy_entry(uint8_t entry, Cursor& cursor);

public:
  template <uint8_t N>
  History(char (&buffer)[N]): History(buffer, N) {}
  History(char* buffer, uint8_t size): buffer_{buffer}, size_{size} {}
  History(): buffer_{nullptr}, size_{0} {}

  void reset_index() { index_ = 0; }
  bool has_prev() { return index_ < entries_; }
  bool has_next() { return index_ > 0; }

  void push(const Cursor& cursor);
  void copy_prev(Cursor& cursor);
  void copy_next(Cursor& cursor);
};

// Read string from stream into buffer
Tokens read_command(StreamEx& stream, Cursor& cursor, History& history, IdleFn idle_fn = nullptr);

template <uint8_t SIZE>
class CursorOwner : public Cursor {
  char buffer_[SIZE];
public:
  CursorOwner(): Cursor(buffer_) {}
};

template <uint8_t SIZE>
class HistoryOwner : public History {
  char buffer_[SIZE];
public:
  HistoryOwner(): History(buffer_) {}
};

template <uint8_t BUF_LEN = 80, uint8_t HIST_LEN = 80>
class CLI {
  CursorOwner<BUF_LEN> cursor_;
  HistoryOwner<HIST_LEN> history_;
  StreamEx& stream_;

public:
  CLI(StreamEx& stream): stream_{stream} {}

  Tokens read(const char* prefill = nullptr, IdleFn idle_fn = nullptr) {
    cursor_.clear();
    if (prefill != nullptr) {
      // Copy editable text into line buffer
      cursor_.try_insert(prefill);
      stream_.print(cursor_.contents());
    }
    return read_command(stream_, cursor_, history_, idle_fn);
  }

  template <uint8_t CMD_LEN>
  void dispatch(Tokens tokens, const Command (&commands)[CMD_LEN]) {
    tokens.dispatch(stream_, commands);
  }

  // Display prompt and execute command from stream
  template <uint8_t CMD_LEN>
  void prompt(const Command (&commands)[CMD_LEN], IdleFn idle_fn = nullptr) {
    stream_.write('>');
    Tokens tokens = read(nullptr, idle_fn);
    stream_.write('\n');
    dispatch(tokens, commands);
  }
};

} // namespace uCLI
