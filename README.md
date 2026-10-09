# Light Monitor

Console program that connects to the Josh.ai Lights Simulator, prints the state of all lights as JSON, then prints changes, new lights, and removed lights as they happen.

## Running

A pre-built Windows x64 binary is included at `bin/light_monitor.exe`. It is statically linked and has no dependencies beyond Windows 10 or later.

1. Download and start the [Josh.ai Lights Simulator](https://github.com/jstarllc/JoshCodingChallenge/releases/latest).
2. Run the monitor:

```
.\bin\light_monitor.exe
.\bin\light_monitor.exe --host 192.168.1.10 --port 8080
```

| Option | Default | Description |
|---|---|---|
| `-n`, `--host` | `localhost` | Hostname or IP address of the simulator |
| `-p`, `--port` | `8080` | Port of the simulator |
| `-h`, `--help` | | Show usage |

Press `Ctrl+C` to exit. Light output is written to stdout, and errors are written to stderr.

## Building

Requires Windows with MinGW-w64 (`g++` with C++14 support) and `mingw32-make`. All libraries are header-only and included in `libraries/`.

```
mingw32-make          # builds bin/light_monitor.exe
mingw32-make clean
```

## Behavior and Error Handling

- **Polling:** each polling cycle starts one second after the previous one completes.
- **Startup:** the initial state is printed only once every light has been fetched successfully, and it is retried until that succeeds.
- **Brightness:** converted from 0-255 to a 0-100 percentage, rounded to nearest. Changes are compared on the percentage, so raw changes that round to the same value are not reported.
- **Changes:** each changed property (`name`, `room`, `on`, `brightness`) is printed as a separate object, as in the challenge examples.
- **Failed requests:** connection failures, timeouts, non-200 responses, and invalid JSON are logged and skipped. They don't modify cached state or produce false change or removal events, and monitoring resumes once valid responses return.
- **Validation:** responses are checked for required fields and types, brightness in range, unique non empty IDs, and a returned ID that matches the requested one.

## Project Structure

```
src/
  main.cpp           Argument parsing and polling loop
  LightClient.*      HTTP requests and response validation
  LightMonitor.*     State tracking and output
  Light.h            Light state
libraries/           Third-party header-only libraries
```

## Libraries

| Library | License |
|---|---|
| [cpp-httplib](https://github.com/yhirose/cpp-httplib) | MIT |
| [JSON for Modern C++](https://github.com/nlohmann/json) | MIT |
| [CmdParser](https://github.com/FlorianRappl/CmdParser) | MIT |

License files are included alongside each library.
