# 🚗 OBD2 Monitor

A C++ application for monitoring OBD2 (On-Board Diagnostic) vehicle data in real-time.

## ⚡ Features

- 🔌 Vehicle connection management via Bluetooth
- 📊 PID (Parameter ID) reading and monitoring
- ⚠️ DTC (Diagnostic Trouble Code) retrieval
- 💾 Data logging to CSV format

## ⚙️ Prerequisites

**Hardware Required:**
- 🔧 ELM327 OBD2 Bluetooth Adapter

**Important:**
- 🔑 **Car ignition must be ON** before running this application

**Setup:**
1. Locate the MAC address of your ELM327 adapter
2. Pair the adapter with your system:
```bash
sudo rfcomm bind 0 <MAC_ADDRESS>
```
Replace `<MAC_ADDRESS>` with your adapter's MAC (e.g., `00:1A:7D:DA:71:13`)

3. The adapter will be available at `/dev/rfcomm0` after pairing

## 🛠️ Building

```bash
make
```

## ▶️ Running

```bash
./obd2_monitor
```

## 🧹 Cleaning

Remove object files and logs:
```bash
make clean
```

Remove everything including the executable:
```bash
make fclean
```

## 📁 Project Structure

- `src/main.cpp` - Entry point & user interface
- `src/connection.cpp/h` - Vehicle connection handling (Bluetooth serial)
- `src/pids.cpp/h` - OBD2 PID operations
- `src/dtc.cpp/h` - Diagnostic trouble code handling
- `src/logger.cpp/h` - Data logging to CSV functionality
