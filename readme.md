# Probescript
![Logo](https://slurpy-films.github.io/probescript/probescript.png)

![C++ CI](https://github.com/slurpy-films/probescript/actions/workflows/build.yml/badge.svg)

## probescript is a programming language designed to be very modular, through its core feature probes.

## Code Example

```probe
probe Main {
    Main() {
        console.println("Hello, World!");
    }
}
```

A probe in probescript is a modular unit similar to a class or function, and is the core building block of the language.

## Building
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Basic Syntax
probescript uses curly brace syntax and optional semicolons. It uses var for variable declarations, fn for function declarations, class for class declarations, and probe for probe declarations. If and while statments are done in the C-like way like this:
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

## Functions and Classes
In probescript, functions are created with the **fn** keyword. Example:
```probe
fn add(a, b) {
    return a + b;
}
```

Classes are created with the **class** keyword. When you create a method in a class, it has access to the **this** object. When you modify that, you modify the object that is created with the class. To set the class constructor, create a fn constructor in the class body.
```probe
class Animal {
    constructor(age, species) {
        this.age = age;
        this.species = species;
    }

    increaseAge(amount) {
        age += amount;
    }

    age = 0;
    species = undefined;
}
```

Class inheritance is done with the **extends** keyword. When you have a class that inherits from another class, you can call the **super()** function to call the parent's constructor. It is not required, but it is best practice to call super() before using the **this** object. Example:
```probe
class FarmAnimal extends Animal {
    constructor(name, age, species) {
        super(age, species); // Call the Animal class constructor
        this.name = name;
    }

    newYear() {
        this.increaseAge(1); // Will call the increaseAge method made in the "Animal" class that this class inherits from
    }

    name = undefined;
}

class Cow extends FarmAnimal {
    constructor(name, age) {
        super(name, age, "Cow");
    }
}
```

To instantiate a class, use the **new** keyword.
```probe
var bob = new Cow("bob", 43); // { name: "bob", age: 43, species: Cow, constructor: [function constructor], newYear: [function constructor], increaseAge: [function increaseAge] }
bob.newYear();
bob.age // 44
```

## For Loops
The for loop syntax in probescript is unique. You create a for loop with the for keyword. After the for keyword you need parentheses. In the for loop there are 3 sections: declarations, conditions, and updates, similar to any language with C-like syntax, but in probescript, they are divided by commas instead of semicolons, since semicolons are optional in probescript. If you need more than one declaration, condition or update, you can put them in parentheses and divide them with commas. Example:
```probe
// Example with one of each
for (var i = 0, i < 10, i++) {
    console.println(i); // 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
}

// Example using multiple declarations, conditions, and updates:
for ((var i = 0, var j = 10), (i > j, j > 0), (i++, j--)) {
    console.println(i, j);
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

## Operators
These are the operators probescript supports now, as well as a brief explanation.

### Binary Operators
These operators are used to perform operations between two values:

    + (Addition Operator): Adds two operands.

        Example: 5 + 3 results in 8.

    - (Subtraction Operator): Subtracts the right operand from the left operand.

        Example: 5 - 3 results in 2.

    * (Multiplication Operator): Multiplies two operands.

        Example: 5 * 3 results in 15.

    / (Division Operator): Divides the left operand by the right operand.

        Example: 6 / 3 results in 2.

    % (Modulo Operator): Returns the remainder of division of two operands.

        Example: 5 % 3 results in 2.

    < (Less Than Operator): Checks if the left operand is less than the right operand.

        Example: 5 < 10 results in true.

    > (Greater Than Operator): Checks if the left operand is greater than the right operand.

        Example: 5 > 3 results in true.

    <= (Less Than or Equal To Operator): Checks if the left operand is less than or equal to the right operand.

        Example: 5 <= 5 results in true.

    >= (Greater Than or Equal To Operator): Checks if the left operand is greater than or equal to the right operand.

        Example: 5 >= 3 results in true.

    == (Equality Operator): Checks if two operands are equal.

        Example: 5 == 5 results in true.

    != (Inequality Operator): Checks if two operands are not equal.

        Example: 5 != 3 results in true.

### Assignment Operator

This operator is used to assign values to variables:

    = (Assignment Operator): Assigns the value of the right operand to the left operand.

        Example: x = 10 assigns the value 10 to x.

### Logical Operators

These operators are used to perform logical operations:

    && (Logical AND Operator): Returns true if both operands are true.

        Example: true && false results in false.

    || (Logical OR Operator): Returns true if at least one of the operands is true.

        Example: true || false results in true.

### Increment/Decrement Operators

These operators are used to increase or decrease a value:

    ++ (Increment Operator): Increases the value of the operand by 1.

        Example: x++ increases x by 1.

    -- (Decrement Operator): Decreases the value of the operand by 1.

        Example: x-- decreases x by 1.

### Assignment Operators

These operators are used to perform operations and then assign the result to a variable:

    += (Addition Assignment Operator): Adds the right operand to the left operand and assigns the result to the left operand.

        Example: x += 5 is equivalent to x = x + 5.

    -= (Subtraction Assignment Operator): Subtracts the right operand from the left operand and assigns the result to the left operand.

        Example: x -= 5 is equivalent to x = x - 5.

    *= (Multiplication Assignment Operator): Multiplies the left operand by the right operand and assigns the result to the left operand.

        Example: x *= 5 is equivalent to x = x * 5.

    /= (Division Assignment Operator): Divides the left operand by the right operand and assigns the result to the left operand.

        Example: x /= 5 is equivalent to x = x / 5.

### Comparison Operators

These operators are used to compare two values:

    == (Equality Operator): Checks if two operands are equal.

        Example: 5 == 5 results in true.

    != (Inequality Operator): Checks if two operands are not equal.

        Example: 5 != 3 results in true.

    < (Less Than Operator): Checks if the left operand is less than the right operand.

        Example: 5 < 10 results in true.

    > (Greater Than Operator): Checks if the left operand is greater than the right operand.

        Example: 5 > 3 results in true.

    <= (Less Than or Equal To Operator): Checks if the left operand is less than or equal to the right operand.

        Example: 5 <= 5 results in true.

    >= (Greater Than or Equal To Operator): Checks if the left operand is greater than or equal to the right operand.

        Example: 5 >= 3 results in true.