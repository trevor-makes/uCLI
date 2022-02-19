// Copyright (c) 2021 Trevor Makes

#include "uCLI.hpp"
#include "uANSI.hpp"

#include <Arduino.h>

namespace uCLI {

void read_command(StreamEx& stream, char* buffer, uint8_t length, IdleFn idle_fn) {
  uint8_t cur = 0;
  uint8_t end = 0;

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
      if (cur > 0) {
        stream.cursor_left();
        --cur;
      }
      continue;
    case uANSI::KEY_RIGHT:
      if (cur < end) {
        stream.cursor_right();
        ++cur;
      }
      continue;
    case uANSI::KEY_HOME:
      if (cur > 0) {
        // Move cursor far left
        stream.cursor_left(cur);
        cur = 0;
      }
      continue;
    case uANSI::KEY_END:
      if (cur < end) {
        // Move cursor far right
        stream.cursor_right(end - cur);
        cur = end;
      }
      continue;
    case '\x08': // ASCII backspace
    case '\x7F': // ASCII delete (not ANSI delete \e[3~)
      if (cur > 0) {
        stream.cursor_left();
        stream.delete_char();
        // Shift following characters left
        for (uint8_t i = cur; i < end; ++i) {
          buffer[i - 1] = buffer[i];
        }
        --cur;
        --end;
      }
      continue;
    case '\n': // NOTE uANSI transforms \r and \r\n to \n
      if (end > 0) {
        // Exit loop and execute command
        buffer[end] = '\0';
        return;
      } else {
        // Ignore and continue if line is empty
        continue;
      }
    default:
      // Ignore other non-printable ASCII chars and uANSI input sequences
      // NOTE UTF-8 multi-byte encodings in [0x80, 0xFF] should be ok
      if (input < 0x20 || input > 0xFF) {
        continue;
      }

      // Prevent typing further characters when buffer is full
      if (end + 1 == length) {
        continue;
      }

      if (cur < end) {
        // Shift following characters right
        for (uint8_t i = end; i > cur; --i) {
          buffer[i] = buffer[i - 1];
        }
        stream.insert_char();
      }

      // Echo and record input
      stream.write(input);
      buffer[cur] = input;
      cur++;
      end++;
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

void parse_command(StreamEx& stream, char* input, const Command commands[], uint8_t length) {
  input = trim_space(input);
  Args args = split_at_separator(input, ' ');

  // Look for match in command list
  for (uint8_t i = 0; i < length; ++i) {
    if (strcmp(input, commands[i].command) == 0) {
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
