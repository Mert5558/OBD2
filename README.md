# OBD2 Monitor

A C++ application for monitoring OBD2 (On-Board Diagnostic) vehicle data.

## Features

- Vehicle connection management
- PID (Parameter ID) reading and monitoring
- DTC (Diagnostic Trouble Code) retrieval
- Data logging to CSV format

## Building

```bash
make
```

## Running

```bash
make run
```

Or:

```bash
./obd2_monitor
```

## Cleaning

Remove object files and logs:
```bash
make clean
```

Remove everything including the executable:
```bash
make fclean
```

## Project Structure

- `main.cpp` - Entry point
- `connection.cpp/h` - Vehicle connection handling
- `pids.cpp/h` - OBD2 PID operations
- `dtc.cpp/h` - Diagnostic trouble code handling
- `logger.cpp/h` - Data logging functionality
