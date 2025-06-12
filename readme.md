# Probescript
![Logo](https://slurpy-films.github.io/probescript/probescript.png)

![C++ CI](https://github.com/slurpy-films/probescript/actions/workflows/build.yml/badge.svg)

## Probescript is a programming language designed to be very modular, through its core feature probes.

## Code Example

```probe
probe Main {
    Main() {
        console.println("Hello, World!");
    }
}
```

A probe in probescript is a modular unit similar to a class or function, and is the core building block of the language.

## Intallation
1. Go to the [latest release](https://github.com/slurpy-films/probescript/releases).
2. Download and unzip the ZIP file that fits your operating system.
3. Follow the instructions found in the `installation.txt` file.

## Building
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Basic Syntax
probescript uses curly brace syntax and optional semicolons. It uses var for variable declarations, fn for function declarations, class for class declarations, and probe for probe declarations. If and while statements are done in the C-like way like this:
```probe
if (condition) {
    // body
}

while (condition) {
    // body
}
```

## Probes
When you make a new probescript project, you should have a main file that has your Main probe. That probe will be run when you start your program. A probe is a hybrid between a function and a class, because it can be called as a function, but it can also inherit like a class. When a probe is called, either as a function or as the Main probe, it needs a run function. The run function will be called with the args that the probe is called with, or none if it is the Main probe. A function with the same name as the probe will be the run function. Example:

```probe
probe Main {
    Main() {
        // This code will run when your program starts
        HelloWorld(); // This will call the run function in the HelloWorld probe
    }
}

probe HelloWorld {
    HelloWorld() {
        print("Hello, World"); // This will call the print function created below
    }

    print(message) {
        console.println(message);
    }
}

```

## Functions
In probescript, functions are created with the **fn** keyword. Example:
```probe
fn add(a, b) {
    return a + b;
}
```

## Import Syntax
Imports are done with the **import** keyword. If you want to import a user made module, that file needs a **module** declaration at the very top of the file, like this: 
```probe
module MyModule;
```
For probescript to properly index your modules, make a project.json file at the root of your project. If you run a directory with the **run** command, it will fall back to the **main** property of your project.json. The directory this file, and all children directories, will be indexed.

When you want to export something from a module, use the export keyword, like this:
```probe
module MyModule;

export fn add(a, b) {
    return a + b;
}
```

When you import a module, use the import keyword and then the name of the module you want to import, like this:
```probe
import MyModule;
// You now have access to an object called MyModule, contanining all the things exported by the module.
MyModule.add(1, 3); // 4
```
If you want to directly import something the module exports, you can do this:
```probe
import MyModule.add;
add(1, 3) // 4
```
If you need a custom identifier for the imported value, use the **as** keyword:
```probe
import MyModule as YourModule;
YourModule.add(1, 3); // 4
```
This also works when you directly import: 
```probe
import MyModule.add as addTwoNumbers;

addTwoNumbers(1, 3); // 4
```

Further documentation is found in the `docs` folder.