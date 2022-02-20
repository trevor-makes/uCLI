# uCLI

uCLI is a simple callback-based command line interface for Arduino projects.

uCLI is designed for Arduino and supports serial connections that implement the Stream interface, like Serial, SoftwareSerial, and SerialUSB. This library is intended to be used with [PlatformIO](https://platformio.org/), but the source files can be manually copied into a project when using the Arduino IDE.

To add uCLI to an existing PlatformIO project, modify the `platformio.ini` configuration file as follows:

```
lib_deps =
    https://github.com/trevor-makes/uCLI.git
monitor_filters = direct
```

Modifying `monitor_filters` is optional, but will configure the PlatformIO serial monitor to display ANSI escape sequences correctly.

uCLI is distributed under the [MIT license](LICENSE.txt)

## Usage

Use the `run_command()` function to display a '>' prompt and block while waiting for input. When the enter key is pressed, the first token in the input will be matched with a `Command` key-value pair and the associated function pointer will be called. If the input does not match a command, the list of commands will be printed instead. Any remaining tokens following a matched command will be provided as an `Args` parameter to that function. Each call to `run_command()` handles one command and returns, so it should normally be called within a loop.

```
uCLI::StreamEx serial_ex{Serial};

void loop() {
  // command list can be global or static local
  static const uCLI::Command commands[] = {
    { "iter", test_iter }, // call test_iter when "iter" is entered
    { "get", test_get }, // call test_get when "get" is entered
  };

  uCLI::run_command(Serial, commands);
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
