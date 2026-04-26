# Memory Profiling — Valgrind

> Always build with `-g -O0` before running any Valgrind tool. Optimized builds reorder code and corrupt reports.

```bash
gcc -g -O0 -std=c17 main.c -o out
```

## Tools

| Tool | Command | Finds |
|------|---------|-------|
| `memcheck` | `valgrind ./out` | Leaks, bad reads/writes, uninitialized memory |
| `callgrind` | `valgrind --tool=callgrind ./out` | CPU hotspots, instruction counts, call graph |
| `massif` | `valgrind --tool=massif ./out` | Heap growth over time, peak usage |
| `helgrind` | `valgrind --tool=helgrind ./out` | Data races between threads |

---

## Memcheck

```bash
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1 ./out
```

| Option | Effect |
|--------|--------|
| `--leak-check=full` | Every leaked block with stack trace |
| `--show-leak-kinds=all` | All categories, not just definitely-lost |
| `--track-origins=yes` | Where uninitialized values came from (slower) |
| `--error-exitcode=1` | Non-zero exit on errors — useful in CI |

**Leak categories:**

```
definitely lost   ← no pointer to it exists — fix these
indirectly lost   ← lost because its parent block leaked
possibly lost     ← pointer into middle of block — usually a bug
still reachable   ← freed at exit — usually fine
```

---

## Callgrind

Counts instructions per function — use to find *where* time is spent.

```bash
valgrind --tool=callgrind --cache-sim=yes --branch-sim=yes ./out
# produces callgrind.out.<pid>

callgrind_annotate callgrind.out.<pid>   # CLI flat report
kcachegrind callgrind.out.<pid>          # GUI (install kcachegrind)
```

| Option | Effect |
|--------|--------|
| `--cache-sim=yes` | L1/L2 cache miss counts per function |
| `--branch-sim=yes` | Branch misprediction counts |
| `--toggle-collect=<fn>` | Profile only inside one function |

---

## Massif

```bash
valgrind --tool=massif ./out
ms_print massif.out.<pid>    # ASCII heap usage graph
```

Use when you suspect heap growth over time rather than a one-shot leak.