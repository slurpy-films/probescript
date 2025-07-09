# Types

## Introduction

In Probescript, you can annotate variables, function parameters, and function return values with types to catch errors at compile-time and write more predictable code.

## Syntax

To type a variable in Probescript, use a colon (`:`) followed by the type name:

```prb
var example: num = 13;
```

If you try to assign a string to this variable, you'll get a compile-time error:

```prb
example = "test"; // TYPEERROR: Cannot assign string to number.
```

To specify a function's return type, add a colon and the type after the parameter list:

```prb
fn example(): num {
    return 13;
}
```

If the return value doesn't match the declared return type, a compile-time error is raised:

```prb
fn example(): num {
    return "test"; // TYPEERROR: String does not match expected return type, number.
}
```

## Built-in / Primitive Types

Probescript has 6 built-in (primitive) types:

* `str` — for string values
* `num` — for numbers (integers and decimals)
* `bool` — for boolean values (`true` or `false`)
* `array` — for lists of items
* `map` — for key-value pairs (like dictionaries or objects)
* `function` — for functions