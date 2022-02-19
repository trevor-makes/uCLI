// Copyright (c) 2021 Trevor Makes

#include "uCLI.hpp"
#include "uANSI.hpp"

#include <Arduino.h>

namespace uCLI {

bool Cursor::try_left() {
  if (cursor_ > 0) {
    --cursor_;
    return true;
  } else {
    return false;
  }
}

bool Cursor::try_right() {
  if (cursor_ < length_) {
    ++cursor_;
    return true;
  } else {
    return false;
  }
}

uint8_t Cursor::jump_home() {
  uint8_t spaces = cursor_;
  cursor_ = 0;
  return spaces;
}

uint8_t Cursor::jump_end() {
  uint8_t spaces = length_ - cursor_;
  cursor_ = length_;
  return spaces;
}

bool Cursor::try_insert(char input) {
  if (length_ >= limit_) {
    return false;
  }

  // Shift following characters right
  for (uint8_t i = length_; i > cursor_; --i) {
    buffer_[i] = buffer_[i - 1];
  }

  // Insert and advance cursor
  buffer_[cursor_] = input;
  ++cursor_;
  ++length_;
  buffer_[length_] = '\0';
  return true;
}

bool Cursor::try_delete() {
  if (cursor_ == 0) {
    return false;
  }

  // Shift following characters left
  for (uint8_t i = cursor_; i < length_; ++i) {
    buffer_[i - 1] = buffer_[i];
  }

  // Backspace cursor
  buffer_[length_] = '\0';
  --cursor_;
  --length_;
  return true;
}

void read_command(StreamEx& stream, Cursor& cursor, IdleFn idle_fn) {
  for (;;) {
    // Call user idle function once per loop
    if (idle_fn) {
      idle_fn();
    }

    // Get next key press
    int input = stream.read();
    if (input == -1) {
      continue;
    }

    switch (input) {
    case uANSI::KEY_LEFT:
      if (cursor.try_left()) {
        stream.cursor_left();
      }
      continue;
    case uANSI::KEY_RIGHT:
      if (cursor.try_right()) {
        stream.cursor_right();
      }
      continue;
    case uANSI::KEY_HOME:
      // Move cursor far left
      stream.cursor_left(cursor.jump_home());
      continue;
    case uANSI::KEY_END:
      // Move cursor far right
      stream.cursor_right(cursor.jump_end());
      continue;
    case '\x08': // ASCII backspace
    case '\x7F': // ASCII delete (not ANSI delete \e[3~)
      if (cursor.try_delete()) {
        stream.cursor_left();
        stream.delete_char();
      }
      continue;
    case '\n': // NOTE uANSI transforms \r and \r\n to \n
      if (cursor.length() > 0) {
        // Exit loop and execute command if line is not empty
        return;
      }
      continue;
    default:
      // Ignore other non-printable ASCII chars and uANSI input sequences
      // NOTE UTF-8 multi-byte encodings in [0x80, 0xFF] should be ok
      if (input < 0x20 || input > 0xFF) {
        continue;
      }

      // Echo inserted character to stream
      if (cursor.try_insert(input)) {
        stream.insert_char();
        stream.write(input);
      }
    }
  }
}

char* trim_space(char* input) {
  while (*input == ' ') {
    ++input;
  }
  return input;
}

char* split_at_separator(char* input, char separator) {
  // Scan until end of string
  while (*input != '\0') {
    // Replace separator with null and return
    if (*input == separator) {
      *input++ = '\0';
      return trim_space(input);
    }
    ++input;
  }
  return input;
}

void parse_command(StreamEx& stream, Args args, const Command commands[], uint8_t length) {
  const char* command = args.next();

  // Look for match in command list
  for (uint8_t i = 0; i < length; ++i) {
    if (strcmp(command, commands[i].command) == 0) {
      commands[i].callback(args);
      return;
    }
  }

  // Invalid command; print command list
  stream.write("Commands: ");
  for (uint8_t i = 0; i < length; ++i) {
    if (i > 0) {
      stream.write(", ");
    }
    stream.write(commands[i].command);
  }
  stream.write('\n');
}

const char* Args::next() {
  char* next = next_;
  if (*next == '\"') {
    next_ = split_at_separator(++next, '\"');
  } else if (*next == '\'') {
    next_ = split_at_separator(++next, '\'');
  } else {
    next_ = split_at_separator(next, ' ');
  }
  return next;
}

} // namespace uCLI
