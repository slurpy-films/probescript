<h1 align="left">
  <img src="img/logo.svg" alt="Probescript Logo" width="40" height="40" style="vertical-align: middle;">
  Probescript
</h1>

![C++ CI](https://github.com/slurpy-films/probescript/actions/workflows/build.yml/badge.svg)

**Probescript** is a statically-typed programming language designed for building complex systems through a modular architecture centered around **probes**.

---

## Philosophy

At the core of probescript's design is the **probe**. A **probe** is an isolated unit of code, similar to a class, that is fully self-contained. Unlike traditional classes, probes do not expose their internal state or behavior. They can only be executed directly, and the only thing you get back is the result of the **run** function of the probe.

This strict isolation encourages clear module boundaries, promotes separation of concerns, and makes large codebases more predictable and maintainable.

---

## Code Example

<div style="background:#1e1e1e;border-radius:6px;padding:16px;overflow-x:auto;font-family:Consolas, Monaco, 'Andale Mono', 'Ubuntu Mono', monospace;font-size:14px;line-height:1.5;">
<code>
<span style="color:#569CD6;">probe</span> <span style="color:#4EC9B0;">Main</span> {<br>
&nbsp;&nbsp;<span style="color:#4EC9B0;">Main</span>() {<br>
&nbsp;&nbsp;&nbsp;&nbsp;console.<span style="color:#DCDCAA;">println</span>(<span style="color:#CE9178;">"Hello, World!"</span>);<br>
&nbsp;&nbsp;}<br>
}
</code>
</div>

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
````

---

## Basic Syntax

Probescript uses C-style curly brace syntax with semicolons.

* `var` for variables
* `fn` for functions
* `class` for classes
* `probe` for probes

### Control Flow:

<div style="background:#1e1e1e;border-radius:6px;padding:16px;overflow-x:auto;font-family:Consolas, Monaco, 'Andale Mono', 'Ubuntu Mono', monospace;font-size:14px;line-height:1.5;">
<code>
<span style="color:#569CD6;">if</span> (condition) {<br>
}<br><br>
<span style="color:#569CD6;">while</span> (condition) {<br>
}
</code>
</div>

---

## Probes

Probes are the fundamental building blocks of every Probescript program.

* Each probe is **completely isolated**.
* Probes are **only executable**, not referenceable.
* Each probe must define a function with the same name, which acts as its **run function**.
* Probes can call other probes, but cannot access their internals.

### Example:

<div style="background:#1e1e1e;border-radius:6px;padding:16px;overflow-x:auto;font-family:Consolas, Monaco, 'Andale Mono', 'Ubuntu Mono', monospace;font-size:14px;line-height:1.5;">
<code>
<span style="color:#569CD6;">probe</span> <span style="color:#4EC9B0;">Greeter</span> {<br>
&nbsp;&nbsp;<span style="color:#DCDCAA;">greet</span>(name) {<br>
&nbsp;&nbsp;&nbsp;&nbsp;console.<span style="color:#DCDCAA;">println</span>(<span style="color:#CE9178;">"Hello, " </span>+ name + <span style="color:#CE9178;">"!"</span>);<br>
&nbsp;&nbsp;}<br><br>
&nbsp;&nbsp;<span style="color:#4EC9B0;">Greeter</span>() {<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span style="color:#DCDCAA;">greet</span>(<span style="color:#CE9178;">"World"</span>);<br>
&nbsp;&nbsp;}<br>
}<br><br>
<span style="color:#569CD6;">probe</span> <span style="color:#4EC9B0;">Main</span> {<br>
&nbsp;&nbsp;<span style="color:#4EC9B0;">Main</span>() {<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span style="color:#4EC9B0;">Greeter</span>();<br>
&nbsp;&nbsp;}<br>
}
</code>
</div>

---

## Functions

Functions are declared using the `fn` keyword:

<div style="background:#1e1e1e;border-radius:6px;padding:16px;overflow-x:auto;font-family:Consolas, Monaco, 'Andale Mono', 'Ubuntu Mono', monospace;font-size:14px;line-height:1.5;">
<code>
<span style="color:#569CD6;">fn</span> <span style="color:#DCDCAA;">add</span>(a, b) {<br>
&nbsp;&nbsp;<span style="color:#569CD6;">return</span> a + b;<br>
}
</code>
</div>

---

## Types

Variable types are inferred from the value, but you can also explictly type a variable like this:

<div style="background:#1e1e1e;border-radius:6px;padding:16px;overflow-x:auto;font-family:Consolas, Monaco, 'Andale Mono', 'Ubuntu Mono', monospace;font-size:14px;line-height:1.5;">
<code>
<span style="color:#569CD6;">var</span> explicit_type: <span style="color:#4EC9B0;">str</span> = <span style="color:#CE9178;">"Hello!"</span>;<br>
<span style="color:#569CD6;">var</span> implicit_type = <span style="color:#CE9178;">"Hello!"</span>;
</code>
</div>

Both variables are strings.

To set a function's return type, add a `:` followed by the return type after the parameters:

<div style="background:#1e1e1e;border-radius:6px;padding:16px;overflow-x:auto;font-family:Consolas, Monaco, 'Andale Mono', 'Ubuntu Mono', monospace;font-size:14px;line-height:1.5;">
<code>
<span style="color:#569CD6;">fn</span> <span style="color:#DCDCAA;">my_function</span>(): <span style="color:#4EC9B0;">str</span> {<br>
&nbsp;&nbsp;<span style="color:#569CD6;">return</span> <span style="color:#CE9178;">"Hello!"</span>;<br>
}
</code>
</div>

---

## Documentation

As of now, there is no documentation, but it is coming soon.

```