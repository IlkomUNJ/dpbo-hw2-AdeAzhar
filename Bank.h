// File: Bank.h

#ifndef BANK_H
#define BANK_H

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <numeric>
#include <memory>
#include "BankAccount.h"
#include "DateUtility.h"

// Gunakan BankAccount dalam bentuk shared_ptr karena Bank memiliki daftar kepemilikan
using BankAccountPtr = std::shared_ptr<BankAccount>;

class Bank
{
private:
    std::map<std::string, BankAccountPtr> accounts; // Map: AccountId -> BankAccountPtr
    std::map<std::string, std::string> customerMap; // Map: UserId -> AccountId
    std::vector<Transaction> allTransactions;       // Semua transaksi bank (topup/withdraw/debit/credit)

    // Konsep Singleton
    Bank() = default;                       // Konstruktor pribadi
    Bank(const Bank &) = delete;            // Non-copyable
    Bank &operator=(const Bank &) = delete; // Non-assignable

public:
    // Metode akses Singleton
    static Bank &getInstance()
    {
        static Bank instance; // Diinisialisasi saat pertama kali diakses
        return instance;
    }

    // --- Fungsionalitas Bank ---

    // 1. Create banking account [cite: 27]
    BankAccountPtr createAccount(const std::string &userId)
    {
        if (customerMap.count(userId))
        {
            std::cout << "Error: User ID " << userId << " sudah memiliki akun bank." << std::endl;
            return accounts.at(customerMap.at(userId));
        }

        std::string accountId = "BA_" + userId;
        auto newAccount = std::make_shared<BankAccount>(accountId, userId);

        accounts[accountId] = newAccount;
        customerMap[userId] = accountId;
        return newAccount;
    }

    // 2. Mendapatkan Akun
    BankAccountPtr getAccount(const std::string &userId)
    {
        if (customerMap.count(userId))
        {
            return accounts.at(customerMap.at(userId));
        }
        return nullptr;
    }

    // 3. Memproses Topup/Withdraw (Transaksi Bank)
    bool processBankTransaction(const std::string &userId, double amount, TransactionType type)
    {
        BankAccountPtr account = getAccount(userId);
        if (!account)
            return false;

        std::string tId = "T" + std::to_string(allTransactions.size() + 1);
        bool success = false;

        if (type == TransactionType::TOPUP)
        {
            success = account->topup(amount, tId);
        }
        else if (type == TransactionType::WITHDRAW)
        {
            success = account->withdraw(amount, tId);
        }

        if (success)
        {
            // Karena cashFlow di BankAccount sudah mencatat transaksi ini,
            // kita bisa mencatatnya di Bank untuk tujuan Bank Listing (List all transaction within a week)
            // Namun, untuk membedakan antara transaksi Toko dan Bank, kita akan ambil dari cashFlow saja.
            // Di sini, kita hanya akan mencatat transaksi Bank inti (Topup/Withdraw)
            allTransactions.emplace_back(tId, userId, amount, type);
        }
        return success;
    }

    // 4. Proses Transfer (Digunakan oleh Store)
    // Transfer dari pembeli (debit) ke penjual (credit)
    bool transfer(const std::string &buyerId, const std::string &sellerId, double amount, const std::string &tId)
    {
        BankAccountPtr buyerAcc = getAccount(buyerId);
        BankAccountPtr sellerAcc = getAccount(sellerId);

        if (!buyerAcc || !sellerAcc)
            return false;

        // 1. Debet dari Pembeli
        if (!buyerAcc->debit(amount, tId))
        {
            return false; // Saldo tidak cukup
        }

        // 2. Kredit ke Penjual
        sellerAcc->credit(amount, tId);

        // Transaksi ini adalah transaksi toko (PURCHASE), jadi kita tidak mencatatnya di allTransactions Bank
        // agar tidak tumpang tindih dengan pencatatan Store.

        return true;
    }

    // --- Fungsionalitas Listing Bank ---

    // List all transaction within a week starting from nowon backwards [cite: 22]
    void listTransactionsWithinAWeek() const
    {
        time_t oneWeekAgo = DateUtility::getPastDays(7);
        std::cout << "\n--- Transaksi Bank (Topup/Withdraw) dalam Seminggu Terakhir ---" << std::endl;

        for (const auto &accPair : accounts)
        {
            const auto &account = accPair.second;
            const auto &cashFlow = account->getCashFlow();

            for (const auto &t : cashFlow)
            {
                // Hanya tampilkan Topup/Withdraw yang terjadi dalam seminggu
                if (t.getDate() >= oneWeekAgo && (t.getType() == TransactionType::TOPUP || t.getType() == TransactionType::WITHDRAW))
                {
                    std::cout << DateUtility::timeToString(t.getDate())
                              << " | Akun: " << account->getId()
                              << " | Tipe: " << (t.getType() == TransactionType::TOPUP ? "TOPUP" : "WITHDRAW")
                              << " | Jumlah: " << (t.getAmount() > 0 ? "+" : "") << t.getAmount() << std::endl;
                }
            }
        }
    }

    // List all bank customers [cite: 23]
    void listAllCustomers() const
    {
        std::cout << "\n--- Daftar Semua Pelanggan Bank ---" << std::endl;
        for (const auto &pair : customerMap)
        {
            std::cout << "User ID: " << pair.first << " | Account ID: " << pair.second << std::endl;
        }
    }

    // List all dormant accounts, no transaction within a month [cite: 24]
    void listDormantAccounts() const
    {
        std::cout << "\n--- Daftar Akun Dormant (Tidak ada transaksi dalam Sebulan) ---" << std::endl;
        time_t oneMonthAgo = DateUtility::getPastMonth();
        int count = 0;

        for (const auto &accPair : accounts)
        {
            const auto &account = accPair.second;
            const auto &cashFlow = account->getCashFlow();

            // Cek transaksi terakhir (kita asumsikan cashFlow selalu terurut berdasarkan tanggal)
            if (cashFlow.empty() || cashFlow.back().getDate() < oneMonthAgo)
            {
                std::cout << "Akun ID: " << account->getId() << " | Pemilik: " << account->getOwnerId() << std::endl;
                count++;
            }
        }
        if (count == 0)
        {
            std::cout << "Tidak ada akun dormant." << std::endl;
        }
    }

    // List n top users that conduct most transaction for today [cite: 25]
    void listTopNUsersToday(int n) const
    {
        // Peta: UserID -> Jumlah Transaksi Hari Ini
        std::map<std::string, int> userTransactionCount;
        time_t startOfToday = DateUtility::getCurrentTime() - (std::time(nullptr) % 86400); // Kira-kira awal hari

        // Iterasi semua cash flow dari semua akun
        for (const auto &accPair : accounts)
        {
            const auto &account = accPair.second;
            for (const auto &t : account->getCashFlow())
            {
                if (t.getDate() >= startOfToday)
                {
                    userTransactionCount[account->getOwnerId()]++;
                }
            }
        }

        // Konversi ke vektor pasangan (count, userId) untuk sorting
        std::vector<std::pair<int, std::string>> sortedUsers;
        for (const auto &pair : userTransactionCount)
        {
            sortedUsers.push_back({pair.second, pair.first});
        }

        // Urutkan (descending)
        std::sort(sortedUsers.rbegin(), sortedUsers.rend());

        std::cout << "\n--- Top " << n << " Pengguna Paling Aktif Hari Ini ---" << std::endl;
        for (int i = 0; i < std::min((int)sortedUsers.size(), n); ++i)
        {
            std::cout << (i + 1) << ". User ID: " << sortedUsers[i].second
                      << " | Jumlah Transaksi: " << sortedUsers[i].first << std::endl;
        }
    }
};

#endif // BANK_H