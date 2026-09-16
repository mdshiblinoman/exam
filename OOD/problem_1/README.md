# Problem 1: Abstract Factory Design Pattern

This solution models Global Motors with separate factories for USA and Asia.

- `CarFactory` is the abstract factory.
- `UsaCarFactory` and `AsiaCarFactory` are concrete factories.
- Each concrete factory creates region-specific `Small`, `Sedan`, and `Luxury` cars.
- `uml.puml` contains the UML class diagram.

## Run

```bash
javac Main.java
java Main
```
