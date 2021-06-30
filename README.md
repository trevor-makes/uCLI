# uCLI

uCLI is a simple callback-based command line interface for Arduino projects.

uCLI is designed for Arduino and supports serial connections that implement the Stream interface, like Serial, SoftwareSerial, and SerialUSB. This package is intended to be used with [PlatformIO](https://platformio.org/), but the source files can be manually copied into a project when using the Arduino IDE.

To add uCLI to an existing PlatformIO project, modify the `platformio.ini` configuration file as follows:

```
lib_deps =
    https://github.com/trevor-makes/uCLI.git
```

uCLI is distributed under the [MIT license](LICENSE.txt)

## Usage

Use the `run_command` function to display a '>' prompt and wait for input. When the enter key is pressed, text up to the first space will be matched with a Command key-value pair and the associated function will be called. Any text following the command will be provided as a parameter. Each call to `run_command` handles one command and returns, so it should normally be called within a loop.

```
void do_foo(const char* args) {
    Serial.write("Doing foo with: ");
    int value = atoi(args);
    Serial.println(value);
}

void do_bar(const char* args) {
    Serial.write("Doing bar with: ");
    Serial.println(args);
}

void loop() {
  // command list can be global or static local
  static const uCLI::Command commands[] = {
    { "foo", do_foo }, // run do_foo when "foo" is entered
    { "bar", do_bar }, // run do_bar when "bar" is entered
  };

  uCLI::run_command(Serial, commands);
}
```

```
>help
Commands: foo, bar
>foo 123
Doing foo with: 123
>bar abc
Doing bar with: abc
```

## Dependencies

uCLI uses the [uANSI](https://github.com/trevor-makes/uANSI) library for text input

## Contributors

[Trevor Makes](mailto:the.trevor.makes@gmail.com)
