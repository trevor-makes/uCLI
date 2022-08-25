# uCLI

**Deprecated! Merged into [core](https://github.com/trevor-makes/core)**

uCLI is a simple callback-based command line interface for Arduino projects.

uCLI is designed for Arduino and supports serial connections that implement the Stream interface, like Serial, SoftwareSerial, and SerialUSB. This library is intended to be used with [PlatformIO](https://platformio.org/), but the source files can be manually copied into a project when using the Arduino IDE.

uCLI works fine with the built-in Arduino serial monitor, but other terminals like PuTTY and the PlatformIO serial monitor support additional keyboard commands via ANSI escape sequences:

- backspace works as expected
- left/right arrows move the cursor and allow insertion
- home/end move the cursor to the beginning/end of the current line
- up/down arrows scroll through recently entered commands

To add uCLI to an existing PlatformIO project, modify the `platformio.ini` configuration file as follows:

```
lib_deps =
    https://github.com/trevor-makes/uCLI.git
monitor_filters = direct
```

Modifying `monitor_filters` is optional, but will configure the PlatformIO serial monitor to display ANSI escape sequences correctly.

uCLI is distributed under the [MIT license](LICENSE.txt)

## Usage

Create a `StreamEx` wrapper around the Arduino stream that will serve the command line, in this case `Serial` for the USB serial on an Arduino Uno. Then create a `CLI<>` instance to drive the command line.

```
#include "uCLI.hpp"

uCLI::StreamEx serial_ex{Serial};
uCLI::CLI<> serial_cli{serial_ex};
```

Use the `prompt()` method of an instance of `CLI` to display a '>' prompt and wait for user input. When the enter key is pressed, the first token in the input will be matched with a `Command` keyword and the paired function pointer will be called. Any remaining tokens will be passed on as an `Args` parameter to that function. If the input does not match a keyword, the list of commands will be printed instead. Each call to `prompt()` handles one command and returns, so it should be called within a loop for an interactive program.

```
void loop() {
  // command list can be global or static local
  static const uCLI::Command commands[] = {
    { "iter", test_iter }, // call test_iter when "iter" is entered
    { "get", test_get }, // call test_get when "get" is entered
  };

  serial_cli.prompt(commands);
}
```

```
>help
Commands: iter, get
>asdf
Commands: iter, get
```

The `Args` parameter can be used as an iterator by calling `next()` until an empty string is returned or `has_next()` returns false. This will return one token at a time, with leading and trailing spaces trimmed away. String arguments may be surrounded with double or single quotes to permit spaces. The `is_string()` method will indicate if the next argument uses quotes.

```
void test_iter(uCLI::Args args) {
  while (args.has_next()) {
    if (args.is_string()) {
      serial_ex.print("was quoted: [");
      serial_ex.print(args.next());
      serial_ex.println("]");
    } else {
      serial_ex.print("not quoted: ");
      serial_ex.println(args.next());
    }
  }
}
```

```
>iter one "two 'three' four"
not quoted: one
was quoted: [two 'three' four]
```

Alternatively, the `get()` method will unpack the first N args into an array of pointers:

```
void test_get(uCLI::Args args) {
  const char* argv[4];
  uint8_t argc = args.get(argv);
  for (uint8_t i = 0; i < argc; ++i) {
    serial_ex.print(i);
    serial_ex.print(": ");
    serial_ex.println(argv[i]);
  }
}
```

```
>get 'first arg' "second arg"
0: first arg
1: second arg
```

## Dependencies

uCLI uses the [uANSI](https://github.com/trevor-makes/uANSI) library for text input

## Contributors

[Trevor Makes](mailto:the.trevor.makes@gmail.com)
