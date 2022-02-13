// Copyright (c) 2021 Trevor Makes

#include "uCLI.hpp"
#include "uANSI.hpp"

#include <Arduino.h>

namespace uCLI {

void read_command(Stream& stream, char* buffer, uint8_t length, IdleFn idle_fn) {
  uint8_t i = 0;
  uint8_t end = 0;

  do {
    // Call user idle function once per loop
    if (idle_fn) {
      idle_fn();
    }

    // Get next key press
    int input = uANSI::read_key(stream);
    if (input == -1) {
      continue;
    }

    // Handle arrow keys (what about up/down, home/end?)
    if (input == uANSI::KEY_LEFT) {
      if (i > 0) {
        // Move cursor left
        stream.write("\e[D");
        --i;
      }
      continue;
    }

    if (input == uANSI::KEY_RIGHT) {
      if (i < end) {
        // Move cursor right
        stream.write("\e[C");
        ++i;
      }
      continue;
    }

    // Ignore escape and other special keys
    // TODO what about non-printable ASCII keys?
    if (input == '\e' || input > 0xFF) {
      continue;
    }

    // Handle backspace and delete
    if (input == '\x08' || input == '\x7F') {
      if (i > 0) {
        // Move cursor left and delete
        stream.write("\e[D\e[P");
        // Shift following characters left
        for (uint8_t j = i; j < end; ++j) {
          buffer[j-1] = buffer[j];
        }
        --i;
        --end;
      }
      continue;
    }

    // Handle enter/return
    if (input == '\n' || input == '\r') {
      if (end > 0) {
        // Exit loop and execute command
        break;
      } else {
        continue;
      }
    }

    if (end > i) {
      // Shift following characters right
      for (uint8_t j = end; j > i; --j) {
        buffer[j] = buffer[j-1];
      }
      // Insert character
      stream.write("\e[@");
    }
    // Echo and record input
    stream.write(input);
    buffer[i] = input;
    i++;
    end++;
  } while (end < length);

  stream.write("\n");
  buffer[end] = '\0';
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

void parse_command(Stream& stream, char* input, const Command commands[], uint8_t length) {
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
  stream.write("\n");
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
