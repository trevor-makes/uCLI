// Copyright (c) 2021 Trevor Makes

#pragma once

#include "uANSI.hpp"

#include <stdint.h>

namespace uCLI {

using uANSI::StreamEx;

class Args {
private:
  char* next_;

public:
  Args(char* args): next_{args} {}

  // Casting const literal "" to char* is a necessary evil for now so that we
  // can avoid dealing with nullptr; the empty string will NOT be mutated
  Args(): next_{const_cast<char*>("")} {}

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
};

class Cursor {
  char* buffer_;
  uint8_t limit_;
  uint8_t cursor_ = 0;
  uint8_t length_ = 0;

public:
  template <uint8_t N>
  Cursor(char (&buffer)[N]): Cursor(buffer, N) {}
  Cursor(char* buffer, uint8_t size): buffer_{buffer}, limit_{uint8_t(size - 1)} { buffer_[0] = '\0'; }

  // Prevent copying
  Cursor(const Cursor&) = delete;
  Cursor& operator=(const Cursor&) = delete;

  uint8_t length() const { return length_; }

  bool try_insert(char input);
  bool try_delete();

  bool try_left();
  bool try_right();
  uint8_t jump_home();
  uint8_t jump_end();
};

using CommandFn = void (*)(Args);
using IdleFn = void (*)();

// Function pointer to be called when command string is entered
struct Command {
  const char* command;
  CommandFn callback;
};

// Read string from stream into buffer
void read_command(StreamEx& stream, Cursor& cursor, IdleFn idle_fn = nullptr);

// Attempt to match input to list of commands
void parse_command(StreamEx& stream, Args args, const Command* commands, uint8_t length);

// Attempt to match input to list of commands
template <uint8_t CMD_LEN>
void parse_command(StreamEx& stream, Args args, const Command (&commands)[CMD_LEN]) {
  parse_command(stream, args, commands, CMD_LEN);
}

// Display prompt and execute command from stream
template <uint8_t BUF_LEN = 80, uint8_t CMD_LEN>
void run_command(StreamEx& stream, const Command (&commands)[CMD_LEN], IdleFn idle_fn = nullptr) {
  static char buffer[BUF_LEN];
  stream.write('>');
  Cursor cursor{buffer};
  read_command(stream, cursor, idle_fn);
  stream.write('\n');
  Args args{buffer};
  parse_command(stream, args, commands, CMD_LEN);
}

} // namespace uCLI
