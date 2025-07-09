# Template Functions

Template functions allow you to write generic code that can still be type-checked at compile time.

## Syntax

In Probescript, a template function is defined like this:

```prb
fn example<T>(): T
{
    // ...
}
```

Here, `T` is a type parameter, and the function returns a value of type `T`.

---

## Usage

To call a template function, you specify the type explicitly:

```prb
example<num>()
```

This tells the compiler that `T` should be `num`, so the function will return a number.

---

## Type Safety

Template functions are type-safe. If the type returned by the function does not match the expected type, a compile-time error will occur. For example:

```prb
var a: str = example<num>() // TYPEERROR: number is not assignable to string
```

In this case, the function returns a number, but the variable `a` is declared as a string, resulting in a type error.