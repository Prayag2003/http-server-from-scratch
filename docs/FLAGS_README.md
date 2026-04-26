# GCC / Clang Flags

## Build Profiles

```bash
# Dev
gcc -std=c17 -O0 -g -Wall -Wextra -Werror -fsanitize=address,undefined main.c -o out

# Debug (GDB)
gcc -std=c17 -Og -g3 -ggdb -fno-omit-frame-pointer main.c -o out

# Production
gcc -std=c17 -O2 -flto -DNDEBUG -fstack-protector-strong -D_FORTIFY_SOURCE=2 -fPIE -pie -Wl,-z,relro,-z,now main.c -o out
```

---

## Pipeline

| Flag | Effect |
|------|--------|
| `-o <name>` | Name the output (`a.out` by default) |
| `-c` | Compile to `.o`, skip linking |
| `-E` | Dump preprocessed source |
| `-S` | Dump assembly |
| `--save-temps` | Keep `.i` `.s` `.o` intermediates |

---

## Warnings

| Flag | Catches |
|------|---------|
| `-Wall` | Common issues — always use |
| `-Wextra` | Extra checks `-Wall` misses — always use |
| `-Werror` | Warnings become errors |
| `-Wpedantic` | Strict ISO C conformance |
| `-Wshadow` | Variable shadowing outer scope |
| `-Wformat=2` | Stricter `printf`/`scanf` checks |
| `-Wconversion` | Implicit lossy type conversions |
| `-Wundef` | Undefined macro used in `#if` |

---

## Optimization

| Flag | Use when |
|------|----------|
| `-O0` | Developing — default, no optimization |
| `-Og` | Debugging — light optimization, readable |
| `-O2` | Production — best safe optimization ★ |
| `-O3` | Compute-heavy — benchmark before using |
| `-Os` | Size-constrained / embedded |
| `-Ofast` | `-O3` + breaks IEEE floats — dangerous |
| `-flto` | Cross-file inlining at link time (+5–15%) |

> `-O3` can be slower than `-O2` due to binary size / cache pressure. Always measure.

---

## Debug

| Flag | Effect |
|------|--------|
| `-g` | Embed DWARF debug info |
| `-g3` | Include macro definitions too |
| `-ggdb` | Tuned for GDB specifically |
| `-fno-omit-frame-pointer` | Readable stack traces in gdb/perf |

---

## Standards

| Flag | Use when |
|------|----------|
| `-std=c17` | Default choice — current stable ★ |
| `-std=c11` | Need threads / atomics |
| `-std=c99` | Legacy or maximum portability |
| `-std=gnu17` | GCC default — has non-portable extensions |

---

## Linker & Includes

| Flag | Effect |
|------|--------|
| `-lm` `-lpthread` `-ldl` | Link math / threads / dynamic loader |
| `-L<path>` | Add library search directory |
| `-I<path>` | Add header search directory |
| `-static` | Fully static binary |
| `-shared` + `-fPIC` | Build a `.so` shared library |

---

## Sanitizers *(dev only — never ship)*

| Flag | Catches | Overhead |
|------|---------|----------|
| `-fsanitize=address` | Overflow, use-after-free, heap corruption | ~2× |
| `-fsanitize=undefined` | UB: int overflow, null deref, bad shifts | ~1.5× |
| `-fsanitize=thread` | Data races | ~10× |
| `-fsanitize=memory` | Uninitialized reads *(Clang only)* | ~3× |

```bash
gcc -fsanitize=address,undefined -g -O1 main.c -o out
```

> Use `-O1` — at `-O0` some UB isn't triggered; at `-O2` stack traces get mangled.

---

## Hardening

| Flag | Protects against |
|------|-----------------|
| `-fstack-protector-strong` | Stack buffer overflows (canaries) |
| `-D_FORTIFY_SOURCE=2` | Unsafe `memcpy`/`strcpy` at runtime (needs `-O1+`) |
| `-fPIE -pie` | Enables ASLR |
| `-Wl,-z,relro,-z,now` | GOT overwrites / lazy-binding hijacks |

---

## CPU Tuning

| Flag | Effect | Portable? |
|------|--------|-----------|
| `-march=native` | All instructions your CPU supports | ✗ |
| `-march=x86-64` | Baseline SSE2 — runs everywhere | ✓ |
| `-mtune=native` | Scheduling only — no new instructions | ✓ |

---

## Misc

| Flag | Effect |
|------|--------|
| `-DFOO` / `-DFOO=1` | Define a macro across all files |
| `-DNDEBUG` | Disables `assert()` — use in production |
| `-MMD -MP` | Auto-generate header dependency rules for Make |
| `-pg` | Instrument for `gprof` profiling |
| `--coverage` | Instrument for `gcov` coverage |
| `-v` | Show all internal compiler commands |