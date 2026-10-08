# cpp_orderbook

A limit order book in modern C++20, built as a **measurement-driven performance study**.
Each design is kept side by side (`OrderBookV1` → `V3`), checked against the same
test suite and benchmarked on the same workloads, so every optimization comes with
a number and a trade-off.

- **36× faster** inserts when the workload creates many new price levels (V3 vs V1)
- **12× faster** best-bid lookup at 100k levels, flat O(1) at any depth (V2/V3 vs V1)
- **0 heap allocations** per new price level, down from 3 (≈4.2 KB) in the baseline
- The same V3 is **12× slower** on deep-level traffic: the costs and limits of each
  design are documented below

---

## Results at a glance

Apple M1 Pro, macOS 26.6, Apple Clang 21, `-O3 -DNDEBUG`. Mean of 5 repetitions,
coefficient of variation < 3% unless noted.

**Cost per `add()`** (ns / order, lower is better)

| Workload | V1 `map`+`deque` | V2 `map` (desc. bids) | V3 vector + pool |
|---|---:|---:|---:|
| Realistic: 1M orders, both sides, near the top of the book | 23.7 | 21.6 | **18.4** |
| Few levels: 1M orders over 10 levels | **4.6** | 5.3 | 5.1 |
| Level-heavy: 100k orders, each at a new best price | 218 | 203 | **6.1** |
| Deep levels: 1M orders cycling over 1,000 levels | 14.3 | **11.1** ¹ | 177 |

**Queries** (ns / call)

| Query | V1 | V2 | V3 |
|---|---:|---:|---:|
| `best_bid()`, 10 levels | 3.15 | **0.94** | 1.26 |
| `best_bid()`, 100k levels | 11.4 | **0.94** | 1.26 |
| `order_count()`, 100k levels | 445,859 | same code as V1 | **0.94** |

**Heap allocations per `add()`** (`orderbook_alloc_probe`)

| | New price level | Existing price level |
|---|---|---|
| V1 / V2 | 3 allocations, 4,176 bytes | 0 |
| V3 | 0.003 (amortized vector growth) | 0 |

¹ cv 3.9–5.8% for the deep-level rows.

---

## Design evolution

### V1: baseline, `std::map<Price, std::deque<Order>>`

The textbook design: a sorted tree of price levels, each holding a FIFO queue of orders.

Counting allocations (by replacing the global `operator new`) showed what a single
new price level actually costs on libc++:

```
tree node        88 B   (3 pointers + color, key, deque header)
deque map         8 B
deque block   4,080 B   (170 orders × 24 B, allocated up front)
```

That is **3 allocations and ≈4.2 KB per level**, about 174× the size of one order.
A sparse book with 1M levels would need more than 4 GB. The same code with
libstdc++ (512 B deque blocks) has a very different footprint, so allocator
behavior is a property of the implementation, not of the source code.

Benchmarks also showed that `best_bid()` is **O(log n)** on libc++: `rbegin()` walks
the right spine of the tree, because libc++ caches the leftmost node but not the
rightmost (3.15 ns at 10 levels → 11.4 ns at 100k).

### V2: descending bids, `std::map<Price, ..., std::greater<>>`

Sorting bids in descending order puts the best bid at `begin()`, which is cached:
`best_bid()` becomes **O(1) and flat at 0.94 ns** regardless of depth.

Because the comparator is part of the type, `bids_` and `asks_` no longer share a
type. A generic lambda replaces the shared `? :` expression, and the comparison
still inlines to a single instruction (no `std::function`, no virtual call).

### V3: data-oriented layout, sorted `std::vector` + order pool

```
bids_  (vector<Level>, 24 B each, ascending; best at back)
  [ 97 | qty | head | tail ][ 98 | ... ][ 99 | ... ][ 100 | ... ]  ← top of book
                                                         │ head
orders_ (vector<OrderSlot>, 16 B each, one pool for the whole book)
  [ id | qty | next ] [ id | qty | next ] [ ... ]   FIFO per level via 32-bit indices
```

- **Hot/cold split:** level search reads only `Level` (price, total quantity, list
  ends). Per-order data is touched only when a level is walked.
- **Best price at the back of the vector:** the top of the book, where nearly all
  activity happens, sits at the cheap end, so `push_back`/`pop_back` don't shift anything.
- **One pool, 32-bit indices instead of pointers:** indices survive vector
  reallocation (pointers would dangle) and halve the link size.
- **O(1) `best_*_quantity()` and `order_count()`**, maintained incrementally.

**Trade-off:** a level lookup is a linear scan from the best price, O(distance from
the top of the book). This is why V3 is 36× faster when new levels appear at the top,
and 12× slower when traffic is spread evenly across 1,000 deep levels.

---

## Findings

1. **The workload decides, not the big-O.** V3 removes a 200 ns cost (allocating
   a new level) that the realistic workload pays only ~250 times per million orders.
   The result is a 1.3× gain on realistic traffic, but 36× on a level-heavy one.
   Before optimizing, ask both *how expensive is it* and *how often does it happen*.
2. **Synthetic benchmarks can flatter a design.** With ascending buy-only prices,
   every V3 insert is a `push_back`, its best case. The realistic generator (both
   sides, geometric distance from the mid price, fixed seed, generated outside the
   timed region) exists to avoid that bias.
3. **Allocation order affects traversal speed.** `order_count()` on V1 costs only
   ≈4.5 ns per tree node at 100k levels (8.8 MB, larger than L2), because nodes
   allocated in sequence are adjacent in memory and the prefetcher hides the misses.
4. **At the nanosecond scale, code generation is visible.** V1 and V2 run the same
   insertion algorithm, yet differ by ±10–20% depending on the workload. An isolated
   experiment traced most of the gap to how `add()` is written (ternary on a
   reference vs. a lambda instantiated twice), not to the comparator. This kind of
   claim needs one variable changed at a time.
5. **Benchmark pitfalls hit during the study:** timing a method that wasn't the one
   intended, counter bookkeeping inside the timed loop (85% of a 9 ns measurement),
   calling the measured function twice per iteration, and comparing against a binary
   that hadn't been rebuilt. All of these produced plausible-looking numbers.

---

## Engineering

- **One test suite for every implementation:** GoogleTest typed tests run each
  behavior against V1, V2 and V3, which caught a regression introduced while editing
  side-by-side versions.
- **Debug preset:** AddressSanitizer + UndefinedBehaviorSanitizer.
- **Strict warnings:** `-Wall -Wextra -Wpedantic -Wshadow -Wconversion
  -Wsign-conversion -Werror` on all project targets.
- **Benchmarks:** Google Benchmark, release preset only, results protected with
  `DoNotOptimize`, aggregated over repetitions.
- **Formatting:** clang-format (Google style, 4-space indent).

## Build and run

Requirements: CMake ≥ 3.25 and a C++20 compiler (tested with Apple Clang 21).
GoogleTest and Google Benchmark are fetched automatically.

```bash
# Tests (debug + sanitizers)
cmake --preset debug && cmake --build --preset debug && ctest --preset debug

# Benchmarks (release)
cmake --preset release && cmake --build --preset release
./build/release/orderbook_bench --benchmark_repetitions=5 --benchmark_report_aggregates_only=true
./build/release/orderbook_alloc_probe
```

## Layout

```
include/orderbook/   order.h, order_book_v1.h, order_book_v2.h, order_book_v3.h
src/                 implementations
tests/               typed tests shared by all versions
bench/               Google Benchmark suite, allocation probe
app/                 small demo
```

## Roadmap

- Matching engine (crossing orders, partial fills) and order cancellation
- Profiling with Instruments / hardware counters to attribute V3's remaining 18 ns per order
- `reserve()` for the pool; binary search, or a hybrid scan for deep levels
- Price-indexed array (tick → level) for bounded price ranges
- Realistic replay including cancels, which dominate real order flow
