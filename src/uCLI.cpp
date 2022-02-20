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

uint8_t Cursor::seek_home() {
  uint8_t spaces = cursor_;
  cursor_ = 0;
  return spaces;
}

uint8_t Cursor::seek_end() {
  uint8_t spaces = length_ - cursor_;
  cursor_ = length_;
  return spaces;
}

uint8_t Cursor::try_insert(const char* input, uint8_t size) {
  // Limit size to space available in Cursor
  size = min(size, limit_ - length_);

  // Limit size to null terminator in input
  for (uint8_t i = 0; i < size; ++i) {
    if (input[i] == '\0') {
      size = i;
    }
  }

  if (size > 0) {
    // Move what follows the cursor and copy input into hole
    memmove(buffer_ + cursor_ + size, buffer_ + cursor_, length_ - cursor_);
    memcpy(buffer_, input, size);
    cursor_ += size;
    length_ += size;
    buffer_[length_] = '\0';
  }

  return size;
}

bool Cursor::try_insert(char input) {
  // Reject if buffer is full or input is null
  if (length_ >= limit_ || input == '\0') {
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

void History::push_from(const Cursor& cursor) {
  if (size_ == 0) {
    return;
  }

  // Limit entry size to absolute size of history buffer (excluding prefix)
  uint8_t size = min(cursor.length(), size_ - 1);
  uint8_t available = size_ - (size + 1);

  // Determine how many old entries will be overwritten
  uint8_t old_size = 0;
  for (uint8_t entry = 0; entry < entries_; ++entry) {
    uint8_t entry_size = 1 + buffer_[old_size]; // prefix byte plus size of entry
    if (old_size + entry_size > available) {
      // Drop this and any remaining entries
      entries_ = entry;
      break;
    }
    old_size += entry_size;
  }

  // Shift old entries back and copy new entry at beginning
  memmove(buffer_ + size + 1, buffer_, old_size);
  memcpy(buffer_ + 1, cursor.contents(), size);
  buffer_[0] = size;
  ++entries_;
}

void History::copy_to(uint8_t entry, Cursor& cursor) {
  if (entry >= entries_) {
    return;
  }

  // Skip forward `entry` list entries
  uint8_t index = 0;
  for (; entry > 0; --entry) {
    index += 1 + buffer_[index]; // skip prefix byte plus size of entry
  }

  // Copy entry into cursor
  cursor.clear();
  uint8_t size = buffer_[index];
  const char* offset = buffer_ + index + 1;
  cursor.try_insert(offset, size);
}

// Move cursor far left and delete line
inline void clear_line(StreamEx& stream, Cursor& cursor) {
  stream.cursor_left(cursor.seek_home());
  stream.delete_char(cursor.length());
  cursor.clear();
}

void read_command(StreamEx& stream, Cursor& cursor, History& history, IdleFn idle_fn) {
  uint8_t hist_index = 0;

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
      stream.cursor_left(cursor.seek_home());
      continue;
    case uANSI::KEY_END:
      // Move cursor far right
      stream.cursor_right(cursor.seek_end());
      continue;
    case uANSI::KEY_UP:
      if (hist_index < history.entries()) {
        clear_line(stream, cursor);
        // Copy history entry into buffer
        history.copy_to(hist_index++, cursor);
        stream.print(cursor.contents());
      }
      continue;
    case uANSI::KEY_DOWN:
      clear_line(stream, cursor);
      hist_index = hist_index > 0 ? hist_index - 1 : 0;
      if (hist_index > 0) {
        // Copy history entry into buffer
        history.copy_to(hist_index - 1, cursor);
        stream.print(cursor.contents());
      }
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
        // Reset history index on edit
        hist_index = 0;
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
