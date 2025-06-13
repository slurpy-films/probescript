# Input/Output in Probescript

## Introduction

This guide explains how to use input and output functions in Probescript to interact with the terminal.

---

## Output

Probescript provides two main functions for printing to the terminal:

* `console.println(...)` – Prints all arguments separated by spaces, followed by a newline.
* `console.print(...)` – Prints all arguments separated by spaces, but **without** adding a newline at the end.

Use `console.println()` for most cases where you want each output on its own line. Use `console.print()` when you want to print multiple things on the same line, followed by a newline later.

Both functions can print any value, including strings, numbers, arrays, maps, and even functions.

**Example:**

```prb
var age: num = 26;
console.println("You are", age, "years old");
```

**Output:**

```
You are 26 years old
```

---

## Input

Probescript has one primary function for reading input from the user:

* `console.prompt(...)` – Displays a prompt with the provided arguments and waits for user input. The input is returned as a string (`str`).

To convert input to a number or another type, use casting functions such as `num()`.

**Example:**

```prb
var name: str = console.prompt("What is your name?");
console.println("Hello", name);
```

**Example with type conversion:**

```prb
var age: num = num(console.prompt("Enter your age:"));
console.println("You are", age, "years old");
```