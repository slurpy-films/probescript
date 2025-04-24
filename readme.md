# ProbeScript

## ProbeScript is a programming language supposed to be very modular, through its core feature probes. There will be a probes.md file which will explain more about probes, but here is a simple explaination:

## Probes
- Can be called as functions
- Can inherit like a class
- When you run a .probe file with the CLI, you need a probe Main in that file unless you specify another probe with the -P flag
- A probe that is to be ran **needs** a run function. In ProbeScript functions are declared like this: fn fnname(param1, param2) { // Function body }
