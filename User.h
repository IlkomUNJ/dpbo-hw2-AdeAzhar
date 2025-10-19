// File: User.h

#ifndef USER_H
#define USER_H

#include <string>
#include <memory>
#include "BankAccount.h"

class User
{
protected:
    std::string userId;
    std::string username;
    std::string password;
    std::string bankAccountId;
    std::shared_ptr<BankAccount> account; // Smart pointer untuk kepemilikan Akun Bank

public:
    User(const std::string &id, const std::string &user, const std::string &pass)
        : userId(id), username(user), password(pass)
    {
        // Akun Bank dibuat terpisah/diinject, di sini hanya inisialisasi ID
        bankAccountId = "ACC_" + id;
        account = nullptr; // Akan di set setelah didaftarkan ke Bank
    }

    // Getter
    std::string getId() const { return userId; }
    std::string getUsername() const { return username; }
    std::string getBankAccountId() const { return bankAccountId; }
    std::shared_ptr<BankAccount> getAccount() const { return account; }

    // Setter (Digunakan oleh Bank untuk menetapkan akun yang terdaftar)
    void setAccount(std::shared_ptr<BankAccount> acc) { account = acc; }

    // Fitur Bank yang dimiliki Buyer/Seller [cite: 28]
    bool topup(double amount, const std::string &tId)
    { // Topup [cite: 29]
        if (account)
            return account->topup(amount, tId);
        return false;
    }

    bool withdraw(double amount, const std::string &tId)
    { // Withdraw [cite: 30]
        if (account)
            return account->withdraw(amount, tId);
        return false;
    }

    // List cash flow (credit/debit)
    void displayCashFlow(int days) const
    {
        if (!account)
        {
            std::cout << "Akun bank belum terdaftar." << std::endl;
            return;
        }

        time_t threshold = DateUtility::getPastDays(days);
        std::vector<Transaction> filtered = account->getCashFlowSince(threshold);

        std::cout << "\n--- Cash Flow " << (days == 30 ? "Sebulan" : "Hari Ini") << " ---" << std::endl;
        for (const auto &t : filtered)
        {
            std::cout << DateUtility::timeToString(t.getDate())
                      << " | Tipe: " << (t.getType() == TransactionType::TOPUP ? "TOPUP" : t.getType() == TransactionType::WITHDRAW ? "WITHDRAW"
                                                                                                                                    : "PURCHASE")
                      << " | Jumlah: " << (t.getAmount() > 0 ? "+" : "") << t.getAmount() << std::endl;
        }
        std::cout << "Saldo Saat Ini: " << account->getBalance() << std::endl;
    }

    // Fungsi verifikasi login
    bool verifyPassword(const std::string &p) const
    {
        return password == p;
    }

    // Metode virtual untuk serialisasi (wajib diimplementasikan di subkelas)
    virtual std::string toString() const = 0; // Pure virtual

    // Destructor virtual
    virtual ~User() = default;
};

#endif // USER_H