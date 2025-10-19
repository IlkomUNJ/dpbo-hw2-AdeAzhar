// File: BankAccount.h

#ifndef BANKACCOUNT_H
#define BANKACCOUNT_H

#include <string>
#include <vector>
#include <numeric>
#include "Transaction.h"

class BankAccount {
private:
    std::string accountId;
    std::string ownerId;
    double balance;
    std::vector<Transaction> cashFlow; // List cash flow (credit/debit) 

public:
    BankAccount(const std::string& accId, const std::string& ownId) 
        : accountId(accId), ownerId(ownId), balance(0.0) {}

    // Getter
    std::string getId() const { return accountId; }
    std::string getOwnerId() const { return ownerId; }
    double getBalance() const { return balance; }
    const std::vector<Transaction>& getCashFlow() const { return cashFlow; }

    // Metode Utama
    bool topup(double amount, const std::string& tId) { // Topup [cite: 29]
        if (amount > 0) {
            balance += amount;
            // Catat sebagai transaksi Bank: TOPUP
            cashFlow.emplace_back(tId, ownerId, amount, TransactionType::TOPUP); 
            return true;
        }
        return false;
    }

    bool withdraw(double amount, const std::string& tId) { // Withdraw [cite: 30]
        // Cek batasan saldo: "Limited by balance" [cite: 37]
        if (amount > 0 && balance >= amount) {
            balance -= amount;
            // Catat sebagai transaksi Bank: WITHDRAW
            cashFlow.emplace_back(tId, ownerId, -amount, TransactionType::WITHDRAW); // -amount untuk debit
            return true;
        }
        return false;
    }

    // Metode untuk memproses pembayaran (Debet)
    bool debit(double amount, const std::string& tId) {
        if (amount > 0 && balance >= amount) {
            balance -= amount;
            // Transaksi pembelian akan dicatat terpisah di Store, ini hanya pergerakan uang
            cashFlow.emplace_back(tId, ownerId, -amount, TransactionType::PURCHASE);
            return true;
        }
        return false;
    }

    // Metode untuk menerima pembayaran (Kredit)
    bool credit(double amount, const std::string& tId) {
        if (amount > 0) {
            balance += amount;
            // Transaksi penjualan akan dicatat terpisah di Store
            cashFlow.emplace_back(tId, ownerId, amount, TransactionType::PURCHASE);
            return true;
        }
        return false;
    }

    // Filter cash flow berdasarkan hari terakhir (credit/debit) 
    std::vector<Transaction> getCashFlowSince(time_t threshold) const {
        std::vector<Transaction> filtered;
        for (const auto& t : cashFlow) {
            if (t.getDate() >= threshold) {
                filtered.push_back(t);
            }
        }
        return filtered;
    }

    // Representasi untuk serialisasi (Id, OwnerId, Balance)
    std::string toString() const {
        return accountId + "," + ownerId + "," + std::to_string(balance);
    }
    
    // Metode Sederhana untuk cek Dormancy (tidak ada transaksi dalam sebulan) [cite: 24]
    bool isDormant() const {
        if (cashFlow.empty()) return true; // Tidak pernah ada transaksi

        time_t oneMonthAgo = DateUtility::getPastMonth();
        // Cek apakah transaksi terakhir lebih lama dari sebulan
        return cashFlow.back().getDate() < oneMonthAgo; 
    }
};

#endif // BANKACCOUNT_H