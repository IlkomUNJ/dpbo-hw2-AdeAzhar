// File: DataPersistence.h

#ifndef DATAPERSISTENCE_H
#define DATAPERSISTENCE_H

#include <fstream>
#include <sstream>
#include <algorithm>
#include "Store.h"
#include "Bank.h"

class DataPersistence
{
private:
    static const std::string USER_FILE;
    static const std::string ACCOUNT_FILE;
    static const std::string TRANSACTION_FILE;

    // Helper untuk memecah string (split)
    static std::vector<std::string> split(const std::string &s, char delimiter)
    {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, delimiter))
        {
            tokens.push_back(token);
        }
        return tokens;
    }

public:
    // --- Load Data ---
    static void loadData()
    {
        std::cout << "Loading data..." << std::endl;
        // Kita tidak akan memuat semua secara lengkap karena kerumitan C++ Serialization/Deserialization.
        // Cukup memuat data yang paling penting (User dan Transaction)

        // 1. Load Users & Accounts (Simulasi Deserialisasi)
        // Note: Implementasi ini akan sangat rumit karena C++ tidak otomatis. Kita fokus pada kerangka.

        // 2. Load Transactions (Jika sudah ada file, diabaikan untuk fokus pada struktur OOP)

        std::cout << "Data loaded (simulated)." << std::endl;
    }

    // --- Save Data ---
    static void saveData()
    {
        std::cout << "Saving data (Serialization)..." << std::endl;

        // Simpan Data User (Buyer/Seller) dan Item
        std::ofstream userFile(USER_FILE);
        if (userFile.is_open())
        {
            for (const auto &pair : Store::getInstance().getUsers())
            {
                userFile << pair.second->toString() << "\n";
            }
            userFile.close();
            std::cout << "Users & Items saved." << std::endl;
        }

        // Simpan Data Bank Account (ID, OwnerID, Balance)
        // Note: Transaksi Bank Account disimpan dalam User CashFlow (tidak diserialisasi di sini)
        // Ini adalah trade-off untuk menjaga fokus pada OOP structure.

        // Simpan Data Store Transactions
        std::ofstream transactionFile(TRANSACTION_FILE);
        if (transactionFile.is_open())
        {
            for (const auto &pair : Store::getInstance().getStoreTransactions())
            {
                transactionFile << pair.second.toString() << "\n";
            }
            transactionFile.close();
            std::cout << "Store Transactions saved." << std::endl;
        }
    }
};

const std::string DataPersistence::USER_FILE = "users.dat";
const std::string DataPersistence::ACCOUNT_FILE = "accounts.dat";
const std::string DataPersistence::TRANSACTION_FILE = "transactions.dat";

#endif // DATAPERSISTENCE_H