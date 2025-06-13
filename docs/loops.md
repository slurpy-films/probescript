# Loops
## There are two main types of loops in probescript: for-loops and while-loops.

## For-Loops

Probescript uses a C-like syntax for `for` loops. You use **semicolons (`;`)** to separate the three parts of the loop: **initialization**, **condition**, and **update**.

### Example with one of each:

```probe
for (var i = 0; i < 10; i++) {
    console.println(i); // Outputs: 0, 1, 2, ..., 9
}
```

### Example with multiple declarations and updates:

```probe
for (var i = 0, j = 10; i < j; i++, j--) {
    console.println(i, j); // Outputs both i and j each iteration
}
```