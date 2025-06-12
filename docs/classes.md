# Classes
## Classes are created with the **class** keyword. When you create a method in a class, it has access to the **this** object. When you modify that, you modify the object that is created with the class. To set the class constructor, create a `new` method in the class body.

## Syntax
In probescript, you use the `class` keyword to create a class. Example:
```prb
class Animal {
    age = 0;
    species = undefined;

    increaseAge(amount) {
        age += amount;
    }

    new(age, species) {
        this.age = age;
        this.species = species;
    }
};
```

Class inheritance is done with the **extends** keyword. When you have a class that inherits from another class, you can call the **super()** function to call the parent's constructor. It is not required, but it is best practice to call super() before using the **this** object. Example:
```probe
class FarmAnimal extends Animal {
    name = undefined;

    newYear() {
        this.increaseAge(1); // Will call the increaseAge method made in the "Animal" class that this class inherits from
    }

    new(name, age, species) {
        super(age, species); // Call the Animal class constructor
        this.name = name;
    }
}

class Cow extends FarmAnimal {
    new(name, age) {
        super(name, age, "Cow");
    }
}
```

To instantiate a class, use the **new** keyword.
```probe
var bob = new Cow("bob", 43); // { name: "bob", age: 43, species: Cow, new: [function new], newYear: [function constructor], increaseAge: [function increaseAge] }
bob.newYear();
bob.age // 44
```