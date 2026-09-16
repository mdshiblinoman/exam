# Problem 2: Chain of Responsibility Design Pattern

This solution models cheque withdrawal authorization with a chain:

`Cashier -> SeniorOfficer -> Manager`

Business rules implemented:

- Amount less than Tk. 10,000: only the `Cashier` authorizes.
- Amount from Tk. 10,000 up to Tk. 1,000,000: both the `Cashier` and `SeniorOfficer` authorize.
- Amount greater than Tk. 1,000,000: the `Cashier` only forwards; `SeniorOfficer` and `Manager` authorize.
- `uml.puml` contains the UML class diagram.

## Run

```bash
javac Main.java
java Main
```
