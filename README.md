# High Performance C++ Mock Exchange

This project is a high-performance, single-threaded Limit Order Book (LOB) and Matching Engine written in modern C++20. It demonstrates low-latency programming techniques suitable for quantitative development and high-frequency trading (HFT) environments.

Included is a native **Dear ImGui** application that visually simulates order flow through the engine in real time.

##  Key Features and Architecture

*   **Zero Dynamic Allocations in the Critical Path**: The engine uses a custom, statically pre-allocated `MemoryPool` that issues `Order` objects in $O(1)$ time, entirely avoiding slow system calls like `new` or `malloc` when matching orders.
*   **$O(1)$ Price Level Operations**: The order book utilizes intrusive doubly-linked lists. Order objects hold their own `prev` and `next` pointers, meaning deletions/fills at the head of a price level (Price-Time Priority) happen in constant time.
*   **$O(1)$ Order Cancellations**: The exchange maintains a flat hash map (`std::unordered_map`) of Order IDs to memory locations. Canceled orders are snipped out of the intrusive linked lists instantly without searching through price structures.
*   **Google Benchmark Verified**: Limit order matches execute in **~15 microseconds** and cancellations in **~1 microsecond** in initial un-optimized map-based structures, extensible to sub-microsecond latency by migrating from `std::map` to dense arrays.
*   **Google Test Suite**: Unit tests verifying edge cases like partial-fills and overlapping limits.
*   **Native GUI**: A high-framerate renderer built with `Dear ImGui` and `GLFW` to visualize the engine's internal state.

## Prerequisites

*   A C++20 capable compiler (`clang` or `gcc`)
*   `CMake` (Version 3.14+)
*   `GLFW` (For the GUI. On macOS, install via `brew install glfw`)

## Building and Running

### 1. Build the GUI App
To run the live mock exchange visualizer:

```bash
cd mock_exchange
cmake -B build -S .
cmake --build build -j$(sysctl -n hw.ncpu)
./build/mock_exchange_gui
```

### 2. Run the Benchmark Suite
To measure pure nanosecond limit-order throughput (ensure you compile in `Release` mode!):

```bash
mkdir build-release
cd build-release
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(sysctl -n hw.ncpu)
./tests/benchmark_matching
```

### 3. Run the Unit Tests
```bash
./build/tests/test_matching
```

## Memory Architecture Deep Dive

The standard way to build a limit order book uses standard libraries: `std::map<Price, std::list<Order>>`. 
However, standard libraries inherently call `new` when inserting elements, destroying latency.

This engine bypasses this by allocating a vast block of memory on startup:
1. `MemoryPool` reserves 1,000,000 blank orders in a single underlying `std::vector`.
2. As a client submits a new limit order, the engine queries the `MemoryPool` free-list index, grabs a memory address, and uses **placement-new** to construct the order *there*.
3. The order pointer is then pushed to the `OrderBook`'s intrusive list (`PriceLevel`). 

By enforcing that all active `Order`s are completely contiguous and pre-allocated, it eliminates context-switching latency during market volatility.
