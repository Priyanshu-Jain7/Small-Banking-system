/*
 * ============================================================
 *  BANKING MANAGEMENT SYSTEM  —  C++17
 *  Features:
 *   - Create / delete accounts (Savings & Current)
 *   - Deposit & Withdraw with validation
 *   - Fund Transfer between accounts
 *   - Transaction history per account
 *   - Interest calculation for Savings accounts
 *   - Search & list all accounts
 *   - Persistent file storage (accounts.dat)
 * ============================================================
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <ctime>
#include <limits>
#include <stdexcept>

// ─────────────────────────────────────────────
//  UTILITIES
// ─────────────────────────────────────────────

static std::string currentTimestamp()
{
    std::time_t now = std::time(nullptr);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return buf;
}

static void clearInput()
{
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

static double getDouble(const std::string &prompt)
{
    double val;
    while (true) {
        std::cout << prompt;
        if (std::cin >> val && val >= 0) { clearInput(); return val; }
        std::cout << "  [!] Invalid input. Please enter a positive number.\n";
        clearInput();
    }
}

static int getInt(const std::string &prompt, int lo, int hi)
{
    int val;
    while (true) {
        std::cout << prompt;
        if (std::cin >> val && val >= lo && val <= hi) { clearInput(); return val; }
        std::cout << "  [!] Please enter a number between " << lo << " and " << hi << ".\n";
        clearInput();
    }
}

static std::string getString(const std::string &prompt)
{
    std::string s;
    std::cout << prompt;
    std::getline(std::cin, s);
    return s;
}

// ─────────────────────────────────────────────
//  TRANSACTION
// ─────────────────────────────────────────────

struct Transaction {
    std::string timestamp;
    std::string type;      // DEPOSIT | WITHDRAWAL | TRANSFER_IN | TRANSFER_OUT | INTEREST
    double      amount;
    double      balance;   // running balance after this txn

    std::string toCSV() const {
        std::ostringstream oss;
        oss << timestamp << "|" << type << "|"
            << std::fixed << std::setprecision(2) << amount << "|" << balance;
        return oss.str();
    }
    static Transaction fromCSV(const std::string &line) {
        Transaction t;
        std::istringstream ss(line);
        std::string tok;
        std::getline(ss, t.timestamp, '|');
        std::getline(ss, t.type,      '|');
        std::getline(ss, tok,         '|'); t.amount  = std::stod(tok);
        std::getline(ss, tok,         '|'); t.balance = std::stod(tok);
        return t;
    }
};

// ─────────────────────────────────────────────
//  ACCOUNT
// ─────────────────────────────────────────────

enum class AccountType { SAVINGS, CURRENT };

class Account {
public:
    long long           accountNumber;
    std::string         holderName;
    std::string         email;
    std::string         phone;
    AccountType         type;
    double              balance;
    bool                active;
    std::vector<Transaction> history;

    static const double SAVINGS_INTEREST_RATE;   // annual %
    static const double MIN_BALANCE_SAVINGS;
    static const double MIN_BALANCE_CURRENT;

    Account() = default;
    Account(long long no, const std::string &name, const std::string &email,
            const std::string &phone, AccountType t, double initialDeposit)
        : accountNumber(no), holderName(name), email(email), phone(phone),
          type(t), balance(0.0), active(true)
    {
        deposit(initialDeposit, "INITIAL_DEPOSIT");
    }

    double minBalance() const {
        return (type == AccountType::SAVINGS) ? MIN_BALANCE_SAVINGS : MIN_BALANCE_CURRENT;
    }

    std::string typeName() const {
        return (type == AccountType::SAVINGS) ? "Savings" : "Current";
    }

    // ── operations ──

    bool deposit(double amount, const std::string &txType = "DEPOSIT") {
        if (amount <= 0) return false;
        balance += amount;
        history.push_back({currentTimestamp(), txType, amount, balance});
        return true;
    }

    bool withdraw(double amount, const std::string &txType = "WITHDRAWAL") {
        if (amount <= 0) return false;
        if (balance - amount < minBalance()) return false;
        balance -= amount;
        history.push_back({currentTimestamp(), txType, amount, balance});
        return true;
    }

    void applyInterest() {
        if (type != AccountType::SAVINGS) return;
        double interest = balance * (SAVINGS_INTEREST_RATE / 100.0);
        deposit(interest, "INTEREST");
        std::cout << "  Interest of Rs " << std::fixed << std::setprecision(2)
                  << interest << " applied.\n";
    }

    void printStatement(int last = 0) const {
        std::cout << "\n  Account: " << accountNumber
                  << "  |  " << holderName
                  << "  |  " << typeName()
                  << "  |  Balance: Rs " << std::fixed << std::setprecision(2) << balance << "\n";
        std::cout << "  " << std::string(80, '-') << "\n";
        std::cout << "  " << std::left
                  << std::setw(22) << "Date/Time"
                  << std::setw(18) << "Type"
                  << std::setw(14) << "Amount (Rs)"
                  << std::setw(14) << "Balance (Rs)" << "\n";
        std::cout << "  " << std::string(80, '-') << "\n";

        auto start = history.begin();
        if (last > 0 && (int)history.size() > last)
            start = history.end() - last;

        for (auto it = start; it != history.end(); ++it) {
            std::cout << "  " << std::left
                      << std::setw(22) << it->timestamp
                      << std::setw(18) << it->type
                      << std::setw(14) << std::fixed << std::setprecision(2) << it->amount
                      << std::setw(14) << it->balance << "\n";
        }
        std::cout << "  " << std::string(80, '-') << "\n";
    }

    // ── serialisation ──

    void save(std::ofstream &out) const {
        out << "ACCOUNT\n"
            << accountNumber << "\n"
            << holderName    << "\n"
            << email         << "\n"
            << phone         << "\n"
            << (type == AccountType::SAVINGS ? "S" : "C") << "\n"
            << std::fixed << std::setprecision(2) << balance << "\n"
            << (active ? "1" : "0") << "\n"
            << history.size() << "\n";
        for (auto &t : history) out << t.toCSV() << "\n";
    }

    static Account load(std::ifstream &in) {
        Account a;
        std::string line;
        std::getline(in, line); // "ACCOUNT"
        std::getline(in, line); a.accountNumber = std::stoll(line);
        std::getline(in, a.holderName);
        std::getline(in, a.email);
        std::getline(in, a.phone);
        std::getline(in, line); a.type   = (line == "S") ? AccountType::SAVINGS : AccountType::CURRENT;
        std::getline(in, line); a.balance = std::stod(line);
        std::getline(in, line); a.active  = (line == "1");
        std::getline(in, line); int n = std::stoi(line);
        for (int i = 0; i < n; ++i) {
            std::getline(in, line);
            a.history.push_back(Transaction::fromCSV(line));
        }
        return a;
    }
};

const double Account::SAVINGS_INTEREST_RATE = 4.0;   // 4 % p.a.
const double Account::MIN_BALANCE_SAVINGS   = 500.0;
const double Account::MIN_BALANCE_CURRENT   = 0.0;

// ─────────────────────────────────────────────
//  BANK (controller)
// ─────────────────────────────────────────────

class Bank {
    std::map<long long, Account> accounts;
    long long nextAccNum = 1000000001LL;
    const std::string DATA_FILE = "accounts.dat";

    void loadData() {
        std::ifstream in(DATA_FILE);
        if (!in) return;
        std::string line;
        // read next account number
        std::getline(in, line);
        if (!line.empty()) nextAccNum = std::stoll(line);
        while (in.peek() != EOF) {
            try {
                Account a = Account::load(in);
                accounts[a.accountNumber] = a;
            } catch (...) { break; }
        }
    }

    void saveData() const {
        std::ofstream out(DATA_FILE);
        out << nextAccNum << "\n";
        for (auto &[no, acc] : accounts) acc.save(out);
    }

    Account* find(long long no) {
        auto it = accounts.find(no);
        return (it != accounts.end() && it->second.active) ? &it->second : nullptr;
    }

public:
    Bank()  { loadData(); }
    ~Bank() { saveData(); }

    // ── 1. Create Account ──

    void createAccount() {
        std::cout << "\n  ── Create New Account ──\n";
        clearInput();
        std::string name  = getString("  Holder Name   : ");
        std::string email = getString("  Email         : ");
        std::string phone = getString("  Phone         : ");

        std::cout << "  Account Type  : 1) Savings   2) Current\n";
        int choice = getInt("  Choice        : ", 1, 2);
        AccountType type = (choice == 1) ? AccountType::SAVINGS : AccountType::CURRENT;

        double minDep = (type == AccountType::SAVINGS) ? Account::MIN_BALANCE_SAVINGS : 0.0;
        std::cout << "  Minimum initial deposit: Rs " << minDep << "\n";
        double dep = getDouble("  Initial Deposit (Rs): ");

        if (dep < minDep) {
            std::cout << "  [!] Initial deposit must be at least Rs " << minDep << ". Aborting.\n";
            return;
        }

        long long no = nextAccNum++;
        accounts[no] = Account(no, name, email, phone, type, dep);
        saveData();

        std::cout << "\n  [✓] Account created successfully!\n";
        std::cout << "  Account Number : " << no << "\n";
    }

    // ── 2. Deposit ──

    void deposit() {
        std::cout << "\n  ── Deposit ──\n";
        long long no = getDouble("  Account Number: ");
        Account *a = find((long long)no);
        if (!a) { std::cout << "  [!] Account not found.\n"; return; }
        double amt = getDouble("  Amount (Rs)   : ");
        if (a->deposit(amt)) {
            saveData();
            std::cout << "  [✓] Deposited Rs " << std::fixed << std::setprecision(2)
                      << amt << ". New Balance: Rs " << a->balance << "\n";
        } else std::cout << "  [!] Invalid amount.\n";
    }

    // ── 3. Withdraw ──

    void withdraw() {
        std::cout << "\n  ── Withdraw ──\n";
        long long no = getDouble("  Account Number: ");
        Account *a = find((long long)no);
        if (!a) { std::cout << "  [!] Account not found.\n"; return; }
        double amt = getDouble("  Amount (Rs)   : ");
        if (a->withdraw(amt)) {
            saveData();
            std::cout << "  [✓] Withdrawn Rs " << std::fixed << std::setprecision(2)
                      << amt << ". New Balance: Rs " << a->balance << "\n";
        } else {
            std::cout << "  [!] Insufficient funds or below minimum balance (Rs "
                      << a->minBalance() << ").\n";
        }
    }

    // ── 4. Transfer ──

    void transfer() {
        std::cout << "\n  ── Fund Transfer ──\n";
        long long from = getDouble("  From Account  : ");
        long long to   = getDouble("  To Account    : ");
        Account *src = find((long long)from);
        Account *dst = find((long long)to);
        if (!src) { std::cout << "  [!] Source account not found.\n"; return; }
        if (!dst) { std::cout << "  [!] Destination account not found.\n"; return; }
        if (from == to) { std::cout << "  [!] Cannot transfer to the same account.\n"; return; }

        double amt = getDouble("  Amount (Rs)   : ");
        if (src->withdraw(amt, "TRANSFER_OUT")) {
            dst->deposit(amt, "TRANSFER_IN");
            saveData();
            std::cout << "  [✓] Transferred Rs " << std::fixed << std::setprecision(2)
                      << amt << " from " << from << " to " << to << "\n";
        } else {
            std::cout << "  [!] Transfer failed — insufficient funds or below minimum balance.\n";
        }
    }

    // ── 5. Balance Inquiry ──

    void checkBalance() {
        std::cout << "\n  ── Balance Inquiry ──\n";
        long long no = getDouble("  Account Number: ");
        Account *a = find((long long)no);
        if (!a) { std::cout << "  [!] Account not found.\n"; return; }
        std::cout << "  Account Holder : " << a->holderName << "\n"
                  << "  Account Type   : " << a->typeName()  << "\n"
                  << "  Balance        : Rs " << std::fixed << std::setprecision(2)
                  << a->balance << "\n";
    }

    // ── 6. Mini Statement / Full Statement ──

    void statement(bool full) {
        std::cout << (full ? "\n  ── Full Statement ──\n" : "\n  ── Mini Statement (last 10) ──\n");
        long long no = getDouble("  Account Number: ");
        Account *a = find((long long)no);
        if (!a) { std::cout << "  [!] Account not found.\n"; return; }
        a->printStatement(full ? 0 : 10);
    }

    // ── 7. Apply Interest (Savings) ──

    void applyInterest() {
        std::cout << "\n  ── Apply Interest to Savings Accounts ──\n";
        int cnt = 0;
        for (auto &[no, acc] : accounts)
            if (acc.active && acc.type == AccountType::SAVINGS)
                { acc.applyInterest(); ++cnt; }
        saveData();
        std::cout << "  [✓] Interest applied to " << cnt << " savings account(s).\n";
    }

    // ── 8. Update Account Details ──

    void updateAccount() {
        std::cout << "\n  ── Update Account Details ──\n";
        long long no = getDouble("  Account Number: ");
        Account *a = find((long long)no);
        if (!a) { std::cout << "  [!] Account not found.\n"; return; }
        std::cout << "  Current Name : " << a->holderName << "\n";
        std::cout << "  Current Email: " << a->email      << "\n";
        std::cout << "  Current Phone: " << a->phone      << "\n";
        clearInput();
        a->holderName = getString("  New Name  (Enter to keep): ");
        a->email      = getString("  New Email (Enter to keep): ");
        a->phone      = getString("  New Phone (Enter to keep): ");
        saveData();
        std::cout << "  [✓] Details updated.\n";
    }

    // ── 9. Close Account ──

    void closeAccount() {
        std::cout << "\n  ── Close Account ──\n";
        long long no = getDouble("  Account Number: ");
        Account *a = find((long long)no);
        if (!a) { std::cout << "  [!] Account not found.\n"; return; }
        std::cout << "  Holder  : " << a->holderName << "\n";
        std::cout << "  Balance : Rs " << std::fixed << std::setprecision(2) << a->balance << "\n";
        std::cout << "  Type YES to confirm closure: ";
        std::string confirm; std::cin >> confirm; clearInput();
        if (confirm != "YES") { std::cout << "  Aborted.\n"; return; }
        a->active = false;
        saveData();
        std::cout << "  [✓] Account " << no << " closed.\n";
    }

    // ── 10. Search Accounts ──

    void searchAccounts() {
        std::cout << "\n  ── Search by Name ──\n";
        clearInput();
        std::string query = getString("  Enter name or keyword: ");
        std::transform(query.begin(), query.end(), query.begin(), ::tolower);
        bool found = false;
        std::cout << "\n  " << std::left
                  << std::setw(14) << "Acc. No"
                  << std::setw(25) << "Holder Name"
                  << std::setw(10) << "Type"
                  << "Balance (Rs)\n";
        std::cout << "  " << std::string(65, '-') << "\n";
        for (auto &[no, acc] : accounts) {
            if (!acc.active) continue;
            std::string lower = acc.holderName;
            std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
            if (lower.find(query) != std::string::npos) {
                std::cout << "  " << std::left
                          << std::setw(14) << no
                          << std::setw(25) << acc.holderName
                          << std::setw(10) << acc.typeName()
                          << std::fixed << std::setprecision(2) << acc.balance << "\n";
                found = true;
            }
        }
        if (!found) std::cout << "  No accounts found for \"" << query << "\".\n";
    }

    // ── 11. List All Accounts ──

    void listAllAccounts() {
        std::cout << "\n  ── All Active Accounts ──\n";
        std::cout << "  " << std::left
                  << std::setw(14) << "Acc. No"
                  << std::setw(25) << "Holder Name"
                  << std::setw(10) << "Type"
                  << "Balance (Rs)\n";
        std::cout << "  " << std::string(65, '-') << "\n";
        double total = 0;
        int count = 0;
        for (auto &[no, acc] : accounts) {
            if (!acc.active) continue;
            std::cout << "  " << std::left
                      << std::setw(14) << no
                      << std::setw(25) << acc.holderName
                      << std::setw(10) << acc.typeName()
                      << std::fixed << std::setprecision(2) << acc.balance << "\n";
            total += acc.balance;
            ++count;
        }
        std::cout << "  " << std::string(65, '-') << "\n";
        std::cout << "  Total accounts: " << count << "   Total deposits: Rs "
                  << std::fixed << std::setprecision(2) << total << "\n";
    }
};

// ─────────────────────────────────────────────
//  MAIN MENU
// ─────────────────────────────────────────────

void printBanner()
{
    std::cout << R"(
  ╔═══════════════════════════════════════════╗
  ║      BANKING MANAGEMENT SYSTEM  v1.0      ║
  ║              Powered by C++17             ║
  ╚═══════════════════════════════════════════╝
)";
}

void printMenu()
{
    std::cout << R"(
  ┌──────────────────────────────────────────┐
  │               MAIN MENU                  │
  ├──────────────────────────────────────────┤
  │  1.  Create Account                      │
  │  2.  Deposit                             │
  │  3.  Withdraw                            │
  │  4.  Fund Transfer                       │
  │  5.  Check Balance                       │
  │  6.  Mini Statement  (last 10)           │
  │  7.  Full Statement                      │
  │  8.  Apply Interest  (Savings)           │
  │  9.  Update Account Details              │
  │ 10.  Close Account                       │
  │ 11.  Search Accounts                     │
  │ 12.  List All Accounts                   │
  │  0.  Exit                                │
  └──────────────────────────────────────────┘
)";
}

int main()
{
    printBanner();
    Bank bank;

    while (true) {
        printMenu();
        int choice = getInt("  Enter choice: ", 0, 12);
        switch (choice) {
            case  1: bank.createAccount();  break;
            case  2: bank.deposit();        break;
            case  3: bank.withdraw();       break;
            case  4: bank.transfer();       break;
            case  5: bank.checkBalance();   break;
            case  6: bank.statement(false); break;
            case  7: bank.statement(true);  break;
            case  8: bank.applyInterest();  break;
            case  9: bank.updateAccount();  break;
            case 10: bank.closeAccount();   break;
            case 11: bank.searchAccounts(); break;
            case 12: bank.listAllAccounts();break;
            case  0:
                std::cout << "\n  Thank you for using the Banking System. Goodbye!\n\n";
                return 0;
        }
    }
}
