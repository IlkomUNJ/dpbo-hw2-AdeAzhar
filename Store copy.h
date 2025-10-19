// File: Store.h

#ifndef STORE_H
#define STORE_H

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <numeric>
#include <memory>
#include "Buyer.h"
#include "Seller.h"
#include "Bank.h"

// Gunakan User dalam bentuk shared_ptr
using UserPtr = std::shared_ptr<User>;
using BuyerPtr = std::shared_ptr<Buyer>;
using SellerPtr = std::shared_ptr<Seller>;

class Store
{
private:
    std::map<std::string, UserPtr> users;                    // Map: UserId -> User (Buyer/Seller)
    std::map<std::string, Transaction> allStoreTransactions; // Map: TId -> Transaction (Transaksi Pembelian)

    // Konsep Singleton
    Store() = default;
    Store(const Store &) = delete;
    Store &operator=(const Store &) = delete;

    // Helper untuk mencari penjual berdasarkan Item ID
    SellerPtr findSellerByItemId(const std::string &itemId)
    {
        for (auto const &pair : users)
        {
            if (auto seller = std::dynamic_pointer_cast<Seller>(pair.second))
            {
                if (seller->getItem(itemId) != nullptr)
                {
                    return seller;
                }
            }
        }
        return nullptr;
    }

public:

    // Getter untuk Serialisasi
    const std::map<std::string, UserPtr>& getUsers() const {
        return users;
    }

    const std::map<std::string, Transaction>& getStoreTransactions() const {
        return allStoreTransactions;
    }
    // Metode akses Singleton
    static Store &getInstance()
    {
        static Store instance;
        return instance;
    }

    // --- Manajemen Pengguna (Register & Login) ---

    // Register User (Buyer/Seller)
    bool registerUser(const std::string &username, const std::string &password, bool isSeller)
    {
        // Cek duplikasi username
        for (const auto &pair : users)
        {
            if (pair.second->getUsername() == username)
            {
                std::cout << "Error: Username sudah digunakan." << std::endl;
                return false;
            }
        }

        std::string newId = "U" + std::to_string(users.size() + 1);
        UserPtr newUser;

        if (isSeller)
        {
            newUser = std::make_shared<Seller>(newId, username, password);
        }
        else
        {
            newUser = std::make_shared<Buyer>(newId, username, password);
        }

        // 1. Daftarkan di Store
        users[newId] = newUser;

        // 2. Buat Akun Bank dan hubungkan ke User
        auto bankAccount = Bank::getInstance().createAccount(newId);
        newUser->setAccount(bankAccount);

        std::cout << "Registrasi " << (isSeller ? "Seller" : "Buyer") << " berhasil. User ID: " << newId << std::endl;
        return true;
    }

    // Login (Mengembalikan UserPtr jika berhasil)
    UserPtr login(const std::string &username, const std::string &password)
    {
        for (const auto &pair : users)
        {
            if (pair.second->getUsername() == username && pair.second->verifyPassword(password))
            {
                std::cout << "Login berhasil! Selamat datang, " << username << "." << std::endl;
                return pair.second;
            }
        }
        std::cout << "Login gagal: Username atau password salah." << std::endl;
        return nullptr;
    }

    // --- Fungsionalitas Toko (Pembelian) ---

    // Purchase item
    bool purchaseItem(UserPtr buyer, const std::string &itemId, int quantity)
    {
        if (!buyer)
            return false;

        SellerPtr seller = findSellerByItemId(itemId);
        if (!seller)
        {
            std::cout << "Pembelian gagal: Item tidak ditemukan." << std::endl;
            return false;
        }

        Item *item = seller->getItem(itemId);
        if (item->getStock() < quantity)
        {
            std::cout << "Pembelian gagal: Stok item (" << item->getName() << ") tidak cukup. Sisa: " << item->getStock() << std::endl;
            return false;
        }

        double totalAmount = item->getPrice() * quantity;
        std::string tId = "S" + std::to_string(allStoreTransactions.size() + 1);

        // 1. Cek Saldo dan Transfer Dana (rely on banking)
        if (!Bank::getInstance().transfer(buyer->getId(), seller->getId(), totalAmount, tId))
        {
            std::cout << "Pembelian gagal: Saldo tidak cukup di akun buyer." << std::endl;
            return false;
        }

        // 2. Kurangi Stok Penjual
        seller->discardStock(itemId, quantity);

        // 3. Catat Transaksi Toko (default status: PAID, karena sudah dibayar)
        Transaction newTransaction(tId, itemId, buyer->getId(), seller->getId(), totalAmount, quantity);
        allStoreTransactions.emplace(tId, std::move(newTransaction));

        // 4. Tambahkan ID Order ke Buyer
        if (auto buyerPtr = std::dynamic_pointer_cast<Buyer>(buyer))
        {
            buyerPtr->addOrderId(tId);
        }
        else
        {
            // Ini seharusnya tidak terjadi jika 'buyer' adalah Buyer atau Seller
            std::cerr << "Error: Gagal melakukan downcast user ke Buyer." << std::endl;
        }

        std::cout << "Pembelian item '" << item->getName() << "' berhasil. Total: " << totalAmount << std::endl;
        return true;
    }

    // List all orders (filter by paid/canceled/completed) - Untuk Buyer/Seller
    void listOrders(const std::vector<std::string> &orderIds, TransactionStatus filter) const
    {
        std::cout << "\n--- Daftar Pesanan (" << (filter == TransactionStatus::PAID ? "PAID" : filter == TransactionStatus::COMPLETED ? "COMPLETED"
                                                                                                                                      : "CANCELLED")
                  << ") ---" << std::endl;
        int count = 0;
        for (const std::string &tId : orderIds)
        {
            if (allStoreTransactions.count(tId) && allStoreTransactions.at(tId).getStatus() == filter)
            {
                const auto &t = allStoreTransactions.at(tId);
                std::cout << "TID: " << t.getId()
                          << " | Item: " << t.getItemId()
                          << " | Qty: " << t.getQuantity()
                          << " | Total: " << t.getAmount()
                          << " | Date: " << DateUtility::timeToString(t.getDate()) << std::endl;
                count++;
            }
        }
        if (count == 0)
        {
            std::cout << "Tidak ada pesanan dengan status ini." << std::endl;
        }
    }

    // File: Store.h (Koreksi)

    // ... (kode sebelumnya)

    // 2. Check spending the last k days (Fitur Buyer)
    void checkSpending(UserPtr buyer, int k) const
    {
        if (!buyer)
            return;
        // Downcast UserPtr ke BuyerPtr
        BuyerPtr buyerPtr = std::dynamic_pointer_cast<Buyer>(buyer);
        if (!buyerPtr)
        {
            // Objek bukan Buyer atau Seller
            std::cout << "Error: User ini tidak memiliki fitur Buyer (Tidak dapat mengecek pengeluaran)." << std::endl;
            return;
        }

        // Gunakan buyerPtr yang sudah di-cast untuk mengakses getOrderIds()
        const std::vector<std::string> &orderIds = buyerPtr->getOrderIds();

        time_t kDaysAgo = DateUtility::getPastDays(k);
        double totalSpending = 0.0;

        // Ganti buyer->getOrderIds() dengan orderIds yang sudah di-cast
        for (const std::string &tId : orderIds)
        {
            if (allStoreTransactions.count(tId))
            {
                const auto &t = allStoreTransactions.at(tId);
                // Hanya hitung transaksi yang sudah dibayar dan dalam rentang k hari
                if (t.getDate() >= kDaysAgo && t.getStatus() != TransactionStatus::CANCELLED)
                {
                    totalSpending += t.getAmount();
                }
            }
        }
        std::cout << "\n--- Total Pengeluaran Buyer " << buyer->getUsername() << " dalam " << k << " hari terakhir: " << totalSpending << " ---" << std::endl;
    }

    // ... (kode selanjutnya)

    // --- Fungsionalitas Listing Toko ---

    // 1. List all transactions of the latest k days
    void listTransactionsLastKDays(int k) const
    {
        time_t kDaysAgo = DateUtility::getPastDays(k);
        std::cout << "\n--- Transaksi Toko " << k << " Hari Terakhir ---" << std::endl;

        for (const auto &pair : allStoreTransactions)
        {
            const auto &t = pair.second;
            if (t.getDate() >= kDaysAgo)
            {
                std::cout << "TID: " << t.getId() << " | Item: " << t.getItemId()
                          << " | Buyer: " << t.getBuyerId()
                          << " | Amount: " << t.getAmount()
                          << " | Status: " << (t.getStatus() == TransactionStatus::PAID ? "PAID" : t.getStatus() == TransactionStatus::COMPLETED ? "COMPLETED"
                                                                                                                                                 : "CANCELLED")
                          << " | Date: " << DateUtility::timeToString(t.getDate()) << std::endl;
            }
        }
    }

    // 2. List all paid transaction but yet to be completed
    void listPaidUncompletedTransactions() const
    {
        std::cout << "\n--- Transaksi Dibayar Tetapi Belum Selesai ---" << std::endl;
        for (const auto &pair : allStoreTransactions)
        {
            const auto &t = pair.second;
            if (t.getStatus() == TransactionStatus::PAID)
            {
                std::cout << "TID: " << t.getId() << " | Item: " << t.getItemId()
                          << " | Buyer: " << t.getBuyerId()
                          << " | Seller: " << t.getSellerId()
                          << " | Amount: " << t.getAmount() << std::endl;
            }
        }
    }

    // 3. List all most m frequent item transactions
    void listMostFrequentItems(int m) const
    {
        std::map<std::string, int> itemFrequency;
        for (const auto &pair : allStoreTransactions)
        {
            if (pair.second.getStatus() != TransactionStatus::CANCELLED)
            {
                itemFrequency[pair.second.getItemId()]++;
            }
        }

        // Konversi ke vektor pasangan (frequency, itemId) untuk sorting
        std::vector<std::pair<int, std::string>> sortedItems;
        for (const auto &pair : itemFrequency)
        {
            sortedItems.push_back({pair.second, pair.first});
        }
        std::sort(sortedItems.rbegin(), sortedItems.rend());

        std::cout << "\n--- Top " << m << " Item Transaksi Paling Sering ---" << std::endl;
        for (int i = 0; i < std::min((int)sortedItems.size(), m); ++i)
        {
            std::cout << (i + 1) << ". Item ID: " << sortedItems[i].second
                      << " | Frekuensi: " << sortedItems[i].first << std::endl;
        }
    }

    // 4. List all most active buyer counted by number of transactions per day
    void listMostActiveBuyers(int m) const
    {
        // Logika kompleks 'per hari' akan kita sederhanakan menjadi total transaksi untuk efisiensi di terminal
        // *Atau* kita hitung transaksi per hari, yang berarti perlu normalisasi terhadap jumlah hari simulasi.
        // Kita akan menggunakan total transaksi untuk simulasi sederhana.
        std::map<std::string, int> buyerTransactions;
        for (const auto &pair : allStoreTransactions)
        {
            if (pair.second.getStatus() != TransactionStatus::CANCELLED)
            {
                buyerTransactions[pair.second.getBuyerId()]++;
            }
        }

        std::vector<std::pair<int, std::string>> sortedBuyers;
        for (const auto &pair : buyerTransactions)
        {
            sortedBuyers.push_back({pair.second, pair.first});
        }
        std::sort(sortedBuyers.rbegin(), sortedBuyers.rend());

        std::cout << "\n--- Top " << m << " Buyer Paling Aktif (Total Transaksi) ---" << std::endl;
        for (int i = 0; i < std::min((int)sortedBuyers.size(), m); ++i)
        {
            std::cout << (i + 1) << ". Buyer ID: " << sortedBuyers[i].second
                      << " | Transaksi: " << sortedBuyers[i].first << std::endl;
        }
    }

    // 5. List all most active sellers counted by number of transactions per day
    void listMostActiveSellers(int m) const
    {
        std::map<std::string, int> sellerTransactions;
        for (const auto &pair : allStoreTransactions)
        {
            if (pair.second.getStatus() != TransactionStatus::CANCELLED)
            {
                sellerTransactions[pair.second.getSellerId()]++;
            }
        }

        std::vector<std::pair<int, std::string>> sortedSellers;
        for (const auto &pair : sellerTransactions)
        {
            sortedSellers.push_back({pair.second, pair.first});
        }
        std::sort(sortedSellers.rbegin(), sortedSellers.rend());

        std::cout << "\n--- Top " << m << " Seller Paling Aktif (Total Transaksi) ---" << std::endl;
        for (int i = 0; i < std::min((int)sortedSellers.size(), m); ++i)
        {
            std::cout << (i + 1) << ". Seller ID: " << sortedSellers[i].second
                      << " | Transaksi: " << sortedSellers[i].first << std::endl;
        }
    }

    // Fitur Seller: Discover top k most popular items per month
    void discoverPopularItems(SellerPtr seller, int k) const
    {
        if (!seller)
            return;
        time_t oneMonthAgo = DateUtility::getPastMonth();
        std::map<std::string, int> itemSalesCount;

        for (const auto &pair : allStoreTransactions)
        {
            const auto &t = pair.second;
            // Filter: Transaksi milik seller ini, terjadi dalam sebulan terakhir, dan tidak dibatalkan
            if (t.getSellerId() == seller->getId() &&
                t.getDate() >= oneMonthAgo &&
                t.getStatus() != TransactionStatus::CANCELLED)
            {
                itemSalesCount[t.getItemId()] += t.getQuantity();
            }
        }

        std::vector<std::pair<int, std::string>> sortedItems;
        for (const auto &pair : itemSalesCount)
        {
            sortedItems.push_back({pair.second, pair.first});
        }
        std::sort(sortedItems.rbegin(), sortedItems.rend());

        std::cout << "\n--- Top " << k << " Item Populer Milik Anda Sebulan Terakhir ---" << std::endl;
        for (int i = 0; i < std::min((int)sortedItems.size(), k); ++i)
        {
            std::cout << (i + 1) << ". Item ID: " << sortedItems[i].second
                      << " | Jumlah Terjual: " << sortedItems[i].first << std::endl;
        }
    }

    // Fitur Seller: Discover loyal customer per month
    void discoverLoyalCustomer(SellerPtr seller) const
    {
        if (!seller)
            return;
        time_t oneMonthAgo = DateUtility::getPastMonth();
        std::map<std::string, double> buyerSpending; // Buyer ID -> Total Spending

        for (const auto &pair : allStoreTransactions)
        {
            const auto &t = pair.second;
            // Filter: Transaksi milik seller ini, terjadi dalam sebulan terakhir, dan tidak dibatalkan
            if (t.getSellerId() == seller->getId() &&
                t.getDate() >= oneMonthAgo &&
                t.getStatus() != TransactionStatus::CANCELLED)
            {
                buyerSpending[t.getBuyerId()] += t.getAmount();
            }
        }

        // Cari pembeli dengan pengeluaran tertinggi
        std::string loyalBuyerId = "N/A";
        double maxSpending = -1.0;
        for (const auto &pair : buyerSpending)
        {
            if (pair.second > maxSpending)
            {
                maxSpending = pair.second;
                loyalBuyerId = pair.first;
            }
        }

        std::cout << "\n--- Pelanggan Paling Loyal Anda Bulan Ini ---" << std::endl;
        if (maxSpending > 0)
        {
            std::cout << "Buyer ID: " << loyalBuyerId << " | Total Belanja: " << maxSpending << std::endl;
        }
        else
        {
            std::cout << "Belum ada transaksi bulan ini." << std::endl;
        }
    }
};

#endif // STORE_H