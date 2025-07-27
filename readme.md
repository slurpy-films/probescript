<h1 align="left">
  <img src="img/logo.svg" alt="Probescript Logo" width="40" height="40" style="vertical-align: middle;">
  Probescript
</h1>

![C++ CI](https://github.com/slurpy-films/probescript/actions/workflows/build.yml/badge.svg)

**Probescript** is a statically-typed programming language designed for building complex systems through a unique modular architecture centered around **probes**.

---

## Philosophy

At the heart of Probescript lies the concept of the **probe**. A **probe** is an isolated unit of code, similar to a class, that is fully self-contained. Unlike traditional classes, probes do not expose their internal state or behavior. They **cannot be referenced directly, only executed**.

This strict isolation encourages clear module boundaries, promotes separation of concerns, and makes large codebases more predictable and maintainable.

---

## Code Example

```probescript
probe Main {
    Main() {
        console.println("Hello, World!");
    }
}
```

The `Main` probe is the program entry point. Probes must define a function with the same name to serve as their entry (run) function.

---

## Installation

1. Visit the [latest release](https://github.com/slurpy-films/probescript/releases).
2. Download and unzip the file appropriate for your operating system.
3. Follow the instructions in `installation.txt`.

---

## Building from Source

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

---

## Basic Syntax

Probescript uses C-style curly brace syntax with semicolons.

* `var` for variables
* `fn` for functions
* `class` for classes
* `probe` for probes

### Control Flow:

```probescript
if (condition) {
    // do something
}

while (condition) {
    // loop
}
```

---

## Probes

Probes are the fundamental building blocks of every Probescript program.

* Each probe is **completely isolated**.
* Probes are **only executable**, not referenceable.
* Each probe must define a function with the same name, which acts as its **run function**.
* Probes can call other probes, but cannot access their internals.

### Example:

```probescript
probe Greeter {
    greet(name) {
        console.println("Hello, " + name + "!");
    }

    Greeter() {
        greet("World");
    }
}

probe Main {
    Main() {
        Greeter(); // Calls the Greeter probe's run function
    }
}
```

This isolation makes probes ideal for structuring systems with clear boundaries and minimal side effects.

---

## Functions

Functions are declared using the `fn` keyword:

```probescript
fn add(a, b) {
    return a + b;
}
```

---