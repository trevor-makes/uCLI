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

Use the `run_command()` function to display a '>' prompt and wait for input. When the enter key is pressed, the first token in the input will be matched with a `Command` key-value pair and the associated function pointer will be called. Any remaining tokens following the command will be provided as an `Args` parameter. Each call to `run_command()` handles one command and returns, so it should normally be called within a loop.

The `Args` parameter can be used as an iterator by calling `next()` until an empty string is returned. This will return one token at a time, with leading and trailing spaces trimmed away. Alternatively, the `remainder()` function will return all remaining text without splitting or trimming spaces.

```
void do_add(uCLI::Args args) {
  int a = atoi(args.next()); // get first token following command
  int b = atoi(args.next()); // get second token following command
  Serial.print(a);
  Serial.print(" + ");
  Serial.print(b);
  Serial.print(" = ");
  Serial.println(a + b);
}

void do_echo(uCLI::Args args) {
  Serial.println(args.remainder()); // print entire argument string
}

void loop() {
  // command list can be global or static local
  static const uCLI::Command commands[] = {
    { "add", do_add }, // call do_add when "add" is entered
    { "echo", do_echo }, // call do_echo when "echo" is entered
  };

  uCLI::run_command(Serial, commands);
}
```

```
>help
Commands: add, echo
>add 2 3
2 + 3 = 5
>echo All the args
All the args
```

## Dependencies

uCLI uses the [uANSI](https://github.com/trevor-makes/uANSI) library for text input

## Contributors

[Trevor Makes](mailto:the.trevor.makes@gmail.com)
