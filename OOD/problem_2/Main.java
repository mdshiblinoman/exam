import java.util.ArrayList;
import java.util.List;

class Cheque {
    private final String accountHolderName;
    private final double amount;
    private final List<String> approvals;

    public Cheque(String accountHolderName, double amount) {
        this.accountHolderName = accountHolderName;
        this.amount = amount;
        this.approvals = new ArrayList<>();
    }

    public String getAccountHolderName() {
        return accountHolderName;
    }

    public double getAmount() {
        return amount;
    }

    public void addApproval(String roleName) {
        approvals.add(roleName);
    }

    public List<String> getApprovals() {
        return approvals;
    }
}

abstract class Authorizer {
    private Authorizer nextAuthorizer;

    public Authorizer setNextAuthorizer(Authorizer nextAuthorizer) {
        this.nextAuthorizer = nextAuthorizer;
        return nextAuthorizer;
    }

    public final void authorize(Cheque cheque) {
        processCheque(cheque);

        if (nextAuthorizer != null && shouldForward(cheque)) {
            nextAuthorizer.authorize(cheque);
        }
    }

    protected abstract void processCheque(Cheque cheque);

    protected boolean shouldForward(Cheque cheque) {
        return true;
    }
}

class Cashier extends Authorizer {
    private static final double SENIOR_OFFICER_LIMIT = 10000;
    private static final double MANAGER_LIMIT = 1000000;

    @Override
    protected void processCheque(Cheque cheque) {
        if (cheque.getAmount() <= MANAGER_LIMIT) {
            cheque.addApproval("Cashier");
            System.out.println("Cashier authorized the cheque.");
            return;
        }

        System.out.println("Cashier forwarded the cheque without authorization.");
    }

    @Override
    protected boolean shouldForward(Cheque cheque) {
        return cheque.getAmount() >= SENIOR_OFFICER_LIMIT;
    }
}

class SeniorOfficer extends Authorizer {
    private static final double SENIOR_OFFICER_LIMIT = 10000;

    @Override
    protected void processCheque(Cheque cheque) {
        if (cheque.getAmount() >= SENIOR_OFFICER_LIMIT) {
            cheque.addApproval("Senior Officer");
            System.out.println("Senior Officer authorized the cheque.");
        }
    }
}

class Manager extends Authorizer {
    private static final double MANAGER_LIMIT = 1000000;

    @Override
    protected void processCheque(Cheque cheque) {
        if (cheque.getAmount() > MANAGER_LIMIT) {
            cheque.addApproval("Manager");
            System.out.println("Manager authorized the cheque.");
        }
    }

    @Override
    protected boolean shouldForward(Cheque cheque) {
        return false;
    }
}

class Bank {
    private final Authorizer firstAuthorizer;

    public Bank() {
        firstAuthorizer = new Cashier();
        Authorizer seniorOfficer = firstAuthorizer.setNextAuthorizer(new SeniorOfficer());
        seniorOfficer.setNextAuthorizer(new Manager());
    }

    public void withdrawUsingCheque(Cheque cheque) {
        System.out.println("Account Holder: " + cheque.getAccountHolderName());
        System.out.println("Cheque Amount: Tk. " + cheque.getAmount());

        firstAuthorizer.authorize(cheque);

        System.out.println("Final Approvals: " + cheque.getApprovals());
        System.out.println();
    }
}

public class Main {
    public static void main(String[] args) {
        Bank bank = new Bank();

        bank.withdrawUsingCheque(new Cheque("Rahim", 8000));
        bank.withdrawUsingCheque(new Cheque("Karim", 50000));
        bank.withdrawUsingCheque(new Cheque("Nusrat", 1500000));
    }
}
