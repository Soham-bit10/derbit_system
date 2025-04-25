# Deribit Trading System (C++)

A high-performance order execution and management system for Deribit Testnet, using C++ and libcurl.

## Features

- ✅ REST API integration (OAuth2)
- ✅ Order placement, cancellation, and orderbook fetching
- ✅ WebSocket server (stubbed, ready for uWebSockets)
- ✅ Multithreaded execution

## Build Instructions

```bash
# Clone repository
git clone <your-repo-url>
cd deribit_trading_system

# Create build directory
mkdir build && cd build

# Configure project
cmake ..

# Build
cmake --build .
