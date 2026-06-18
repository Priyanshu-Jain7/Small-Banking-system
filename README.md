# 🏦 Banking Management System

A feature-rich, console-based Banking Management System written in **C++17**. It supports two account types, full transaction history, fund transfers, interest calculation, and persistent file storage — all with no external dependencies.

---

## 📋 Table of Contents

- [Features](#features)
- [Project Structure](#project-structure)
- [Requirements](#requirements)
- [Getting Started](#getting-started)
- [Usage](#usage)
- [Data Storage](#data-storage)
- [Business Rules](#business-rules)
- [Class Design](#class-design)
- [Sample Session](#sample-session)

---

## ✨ Features

| # | Feature | Description |
|---|---------|-------------|
| 1 | **Create Account** | Open a Savings or Current account with holder details |
| 2 | **Deposit** | Add funds with a timestamped transaction record |
| 3 | **Withdraw** | Withdraw funds with minimum-balance enforcement |
| 4 | **Fund Transfer** | Transfer money between any two active accounts |
| 5 | **Balance Inquiry** | Instant balance check by account number |
| 6 | **Mini Statement** | View the last 10 transactions with running balance |
| 7 | **Full Statement** | View complete transaction history |
| 8 | **Apply Interest** | Credit 4% p.a. interest to all Savings accounts |
| 9 | **Update Details** | Edit holder name, email, or phone number |
| 10 | **Close Account** | Soft-delete an account with confirmation prompt |
| 11 | **Search Accounts** | Case-insensitive search by account holder name |
| 12 | **List All Accounts** | Summary table with total deposits across all accounts |

---

## 📁 Project Structure

```
banking-management-system/
├── banking_system.cpp   # Full source code (single file)
├── accounts.dat         # Auto-generated data file (created on first run)
└── README.md
```

---

## ⚙️ Requirements

- A C++ compiler supporting **C++17** or later
  - GCC 7+ (`g++`)
  - Clang 5+
  - MSVC 2017+
- Linux, macOS, or Windows (WSL recommended on Windows)

No third-party libraries required — only the C++ Standard Library.

---

## 🚀 Getting Started

### 1. Clone or download

```bash
git clone https://github.com/your-username/banking-management-system.git
cd banking-management-system
```

Or simply place `banking_system.cpp` in a folder of your choice.

### 2. Compile

```bash
g++ -std=c++17 -O2 -o bank_app banking_system.cpp
```

### 3. Run

```bash
./bank_app
```

> **Note:** On Windows with MinGW, use `bank_app.exe` instead.

---

## 🖥️ Usage

After launching, you are presented with an interactive menu:

```
  ╔═══════════════════════════════════════════╗
  ║      BANKING MANAGEMENT SYSTEM  v1.0      ║
  ║              Powered by C++17             ║
  ╚═══════════════════════════════════════════╝

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
```

Navigate by entering the number corresponding to your desired action and pressing Enter. All inputs are validated — invalid entries prompt you to try again.

---

## 💾 Data Storage

All data is persisted automatically in **`accounts.dat`** (plain-text format) in the same directory as the executable.

- Data is **loaded on startup** and **saved after every write operation** (create, deposit, withdraw, transfer, update, close, interest).
- No database or external dependency is needed.
- **Do not manually edit** `accounts.dat` while the program is running.

### File format (internal)

```
<next_account_number>
ACCOUNT
<account_number>
<holder_name>
<email>
<phone>
S or C         (Savings / Current)
<balance>
1 or 0         (active / closed)
<transaction_count>
<timestamp>|<type>|<amount>|<running_balance>
...
```

---

## 📏 Business Rules

| Rule | Value |
|------|-------|
| Account numbers | Start at `1000000001`, auto-increment |
| Savings minimum balance | Rs 500.00 |
| Current minimum balance | Rs 0.00 |
| Savings interest rate | 4% per annum (applied manually via menu option 8) |
| Closed accounts | Soft-deleted (data retained, marked inactive) |
| Transfer to self | Not allowed |

---

## 🏗️ Class Design

```
Transaction
  ├── timestamp   (string)
  ├── type        (DEPOSIT | WITHDRAWAL | TRANSFER_IN | TRANSFER_OUT | INTEREST | INITIAL_DEPOSIT)
  ├── amount      (double)
  └── balance     (double — running balance after this transaction)

Account
  ├── accountNumber  (long long)
  ├── holderName     (string)
  ├── email          (string)
  ├── phone          (string)
  ├── type           (AccountType::SAVINGS | AccountType::CURRENT)
  ├── balance        (double)
  ├── active         (bool)
  ├── history        (vector<Transaction>)
  ├── deposit()
  ├── withdraw()
  ├── applyInterest()
  ├── printStatement()
  ├── save()         — serialise to file
  └── load()         — deserialise from file

Bank
  ├── accounts       (map<long long, Account>)
  ├── createAccount()
  ├── deposit()
  ├── withdraw()
  ├── transfer()
  ├── checkBalance()
  ├── statement()
  ├── applyInterest()
  ├── updateAccount()
  ├── closeAccount()
  ├── searchAccounts()
  └── listAllAccounts()
```

---

## 🧪 Sample Session

```
  Enter choice: 1

  ── Create New Account ──
  Holder Name   : Priyanshu Jain
  Email         : priyanshu@example.com
  Phone         : 9876543210
  Account Type  : 1) Savings   2) Current
  Choice        : 1
  Minimum initial deposit: Rs 500
  Initial Deposit (Rs): 5000

  [✓] Account created successfully!
  Account Number : 1000000001

  Enter choice: 2

  ── Deposit ──
  Account Number: 1000000001
  Amount (Rs)   : 3000

  [✓] Deposited Rs 3000.00. New Balance: Rs 8000.00

  Enter choice: 6

  ── Mini Statement (last 10) ──
  Account Number: 1000000001

  Account: 1000000001  |  Priyanshu Jain  |  Savings  |  Balance: Rs 8000.00
  --------------------------------------------------------------------------------
  Date/Time             Type              Amount (Rs)   Balance (Rs)
  --------------------------------------------------------------------------------
  2026-06-19 10:22:31   INITIAL_DEPOSIT   5000.00       5000.00
  2026-06-19 10:23:05   DEPOSIT           3000.00       8000.00
  --------------------------------------------------------------------------------
```

---

## 📄 License

This project is open source and free to use for educational purposes.