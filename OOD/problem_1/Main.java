interface Car {
    String getType();

    String getEngineVolume();

    String getColor();

    String getFuelType();

    void displayDetails();
}

abstract class AbstractCar implements Car {
    private final String type;
    private final String engineVolume;
    private final String color;
    private final String fuelType;

    protected AbstractCar(String type, String engineVolume, String color, String fuelType) {
        this.type = type;
        this.engineVolume = engineVolume;
        this.color = color;
        this.fuelType = fuelType;
    }

    @Override
    public String getType() {
        return type;
    }

    @Override
    public String getEngineVolume() {
        return engineVolume;
    }

    @Override
    public String getColor() {
        return color;
    }

    @Override
    public String getFuelType() {
        return fuelType;
    }

    @Override
    public void displayDetails() {
        System.out.println(type + " Car");
        System.out.println("Engine Volume: " + engineVolume);
        System.out.println("Color: " + color);
        System.out.println("Fuel Type: " + fuelType);
        System.out.println();
    }
}

class UsaSmallCar extends AbstractCar {
    public UsaSmallCar() {
        super("USA Small", "1.0L", "Red", "Petrol");
    }
}

class UsaSedanCar extends AbstractCar {
    public UsaSedanCar() {
        super("USA Sedan", "2.0L", "Blue", "Petrol");
    }
}

class UsaLuxuryCar extends AbstractCar {
    public UsaLuxuryCar() {
        super("USA Luxury", "3.5L", "Black", "Petrol");
    }
}

class AsiaSmallCar extends AbstractCar {
    public AsiaSmallCar() {
        super("Asia Small", "0.8L", "White", "Electric");
    }
}

class AsiaSedanCar extends AbstractCar {
    public AsiaSedanCar() {
        super("Asia Sedan", "1.5L", "Silver", "Hybrid");
    }
}

class AsiaLuxuryCar extends AbstractCar {
    public AsiaLuxuryCar() {
        super("Asia Luxury", "2.5L", "Yellow", "Hybrid");
    }
}

interface CarFactory {
    Car createSmallCar();

    Car createSedanCar();

    Car createLuxuryCar();
}

class UsaCarFactory implements CarFactory {
    @Override
    public Car createSmallCar() {
        return new UsaSmallCar();
    }

    @Override
    public Car createSedanCar() {
        return new UsaSedanCar();
    }

    @Override
    public Car createLuxuryCar() {
        return new UsaLuxuryCar();
    }
}

class AsiaCarFactory implements CarFactory {
    @Override
    public Car createSmallCar() {
        return new AsiaSmallCar();
    }

    @Override
    public Car createSedanCar() {
        return new AsiaSedanCar();
    }

    @Override
    public Car createLuxuryCar() {
        return new AsiaLuxuryCar();
    }
}

class GlobalMotors {
    public void showAvailableCars(CarFactory carFactory) {
        carFactory.createSmallCar().displayDetails();
        carFactory.createSedanCar().displayDetails();
        carFactory.createLuxuryCar().displayDetails();
    }
}

public class Main {
    public static void main(String[] args) {
        GlobalMotors globalMotors = new GlobalMotors();

        System.out.println("USA Cars");
        System.out.println("--------");
        globalMotors.showAvailableCars(new UsaCarFactory());

        System.out.println("Asia Cars");
        System.out.println("---------");
        globalMotors.showAvailableCars(new AsiaCarFactory());
    }
}
