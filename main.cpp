// File: main.cpp

#include <iostream>
#include <limits>
#include "Store.h"
#include "DataPersistence.h"

// --- Global Pointers ---
UserPtr current_user = nullptr;

// --- Helper Functions ---

void clear_input()
{
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int get_int_input(const std::string &prompt)
{
    int value;
    std::cout << prompt;
    while (!(std::cin >> value) || value <= 0)
    {
        std::cout << "Input tidak valid. Masukkan angka positif: ";
        std::cin.clear();
        clear_input();
    }
    clear_input();
    return value;
}

double get_double_input(const std::string &prompt)
{
    double value;
    std::cout << prompt;
    while (!(std::cin >> value) || value <= 0.0)
    {
        std::cout << "Input tidak valid. Masukkan jumlah positif: ";
        std::cin.clear();
        clear_input();
    }
    clear_input();
    return value;
}

// --- Menu Functions ---

void menu_buyer()
{
    BuyerPtr buyer = std::dynamic_pointer_cast<Buyer>(current_user);
    if (!buyer)
        return;

    int choice;
    do
    {
        std::cout << "\n--- Buyer Menu (" << buyer->getUsername() << ") ---" << std::endl;
        std::cout << "1. Topup Akun Bank" << std::endl;
        std::cout << "2. Withdraw Akun Bank" << std::endl;
        std::cout << "3. List Cash Flow (Today/Month)" << std::endl;
        std::cout << "4. Purchase Item" << std::endl;
        std::cout << "5. List All Orders" << std::endl;
        std::cout << "6. Check Spending (k Days)" << std::endl;
        std::cout << "7. Logout" << std::endl;
        std::cout << "Pilih Opsi: ";

        if (!(std::cin >> choice))
        {
            choice = 0; // Handle non-numeric input
            std::cin.clear();
        }
        clear_input();

        switch (choice)
        {
        case 1:
        { // Topup
            double amount = get_double_input("Masukkan jumlah Topup: ");
            if (Bank::getInstance().processBankTransaction(buyer->getId(), amount, TransactionType::TOPUP))
            {
                std::cout << "Topup berhasil. Saldo baru: " << buyer->getAccount()->getBalance() << std::endl;
            }
            else
            {
                std::cout << "Topup gagal." << std::endl;
            }
            break;
        }
        case 2:
        { // Withdraw
            double amount = get_double_input("Masukkan jumlah Withdraw: ");
            if (Bank::getInstance().processBankTransaction(buyer->getId(), amount, TransactionType::WITHDRAW))
            {
                std::cout << "Withdraw berhasil. Saldo baru: " << buyer->getAccount()->getBalance() << std::endl;
            }
            else
            {
                std::cout << "Withdraw gagal (Saldo tidak cukup/jumlah tidak valid)." << std::endl;
            }
            break;
        }
        case 3:
        { // List Cash Flow
            int days;
            std::cout << "Pilih rentang waktu (1: Hari Ini, 30: Sebulan): ";
            if (!(std::cin >> days))
            {
                days = 1;
                std::cin.clear();
            }
            clear_input();
            buyer->displayCashFlow(days);
            break;
        }
        case 4:
        { // Purchase Item
            std::string itemId;
            int qty;
            std::cout << "Masukkan Item ID yang akan dibeli: ";
            std::getline(std::cin, itemId);
            qty = get_int_input("Masukkan kuantitas: ");
            Store::getInstance().purchaseItem(buyer, itemId, qty);
            break;
        }
        case 5:
        { // List All Orders
            int f;
            std::cout << "Filter Status (1: PAID, 2: COMPLETED, 3: CANCELLED): ";
            if (!(std::cin >> f))
            {
                f = 1;
                std::cin.clear();
            }
            clear_input();
            TransactionStatus filter = TransactionStatus::PAID;
            if (f == 2)
                filter = TransactionStatus::COMPLETED;
            if (f == 3)
                filter = TransactionStatus::CANCELLED;
            Store::getInstance().listOrders(buyer->getOrderIds(), filter);
            break;
        }
        case 6:
        { // Check Spending
            int k = get_int_input("Cek pengeluaran (k hari terakhir): ");
            Store::getInstance().checkSpending(buyer, k);
            break;
        }
        case 7:
            current_user = nullptr;
            std::cout << "Anda telah logout." << std::endl;
            break;
        default:
            std::cout << "Pilihan tidak valid." << std::endl;
        }
    } while (choice != 7);
}

void menu_seller()
{
    SellerPtr seller = std::dynamic_pointer_cast<Seller>(current_user);
    if (!seller)
    {
        menu_buyer();
        return;
    } // Jika gagal cast, kembali ke buyer menu

    int choice;
    do
    {
        std::cout << "\n--- Seller Menu (" << seller->getUsername() << ") ---" << std::endl;
        std::cout << "1. Akses Fitur Buyer" << std::endl;
        std::cout << "2. Manage Items (Register/Replenish/Discard)" << std::endl;
        std::cout << "3. Discover Top K Popular Items (Per Month)" << std::endl;
        std::cout << "4. Discover Loyal Customer (Per Month)" << std::endl;
        std::cout << "5. Logout" << std::endl;
        std::cout << "Pilih Opsi: ";

        if (!(std::cin >> choice))
        {
            choice = 0;
            std::cin.clear();
        }
        clear_input();

        switch (choice)
        {
        case 1:
            menu_buyer();
            break;
        case 2:
        { // Manage Items
            int subChoice;
            std::cout << "1. Register New Item | 2. Replenish Stock | 3. Discard Stock: ";
            if (!(std::cin >> subChoice))
            {
                subChoice = 0;
                std::cin.clear();
            }
            clear_input();

            if (subChoice == 1)
            {
                std::string itemId, name;
                double price;
                int stock;
                std::cout << "Item ID: ";
                std::getline(std::cin, itemId);
                std::cout << "Nama Item: ";
                std::getline(std::cin, name);
                price = get_double_input("Harga per item: ");
                stock = get_int_input("Stok awal: ");
                seller->registerNewItem(itemId, name, price, stock);
                std::cout << "Item '" << name << "' berhasil didaftarkan." << std::endl;
            }
            else if (subChoice == 2)
            {
                std::string itemId;
                int qty;
                std::cout << "Item ID yang akan ditambah: ";
                std::getline(std::cin, itemId);
                qty = get_int_input("Jumlah stok yang akan ditambahkan: ");
                if (seller->replenishStock(itemId, qty))
                {
                    std::cout << "Stok item berhasil diperbarui." << std::endl;
                }
                else
                {
                    std::cout << "Gagal memperbarui stok." << std::endl;
                }
            }
            else if (subChoice == 3)
            {
                std::string itemId;
                int qty;
                std::cout << "Item ID yang akan dibuang: ";
                std::getline(std::cin, itemId);
                qty = get_int_input("Jumlah stok yang akan dibuang: ");
                if (seller->discardStock(itemId, qty))
                {
                    std::cout << "Stok item berhasil dibuang." << std::endl;
                }
                else
                {
                    std::cout << "Gagal membuang stok (stok tidak cukup)." << std::endl;
                }
            }
            break;
        }
        case 3:
        { // Discover Top K Popular Items
            int k = get_int_input("Jumlah item populer yang ingin dilihat: ");
            Store::getInstance().discoverPopularItems(seller, k);
            break;
        }
        case 4:
        { // Discover Loyal Customer
            Store::getInstance().discoverLoyalCustomer(seller);
            break;
        }
        case 5:
            current_user = nullptr;
            std::cout << "Anda telah logout." << std::endl;
            break;
        default:
            std::cout << "Pilihan tidak valid." << std::endl;
        }
    } while (choice != 5);
}

void menu_store_bank_management()
{
    int choice;
    do
    {
        std::cout << "\n--- Management Menu (Store & Bank) ---" << std::endl;
        std::cout << "1. List Store Transactions (k Days)" << std::endl;
        std::cout << "2. List Paid but Uncompleted Transactions" << std::endl;
        std::cout << "3. List Most Frequent Items" << std::endl;
        std::cout << "4. List Most Active Buyers" << std::endl;
        std::cout << "5. List Most Active Sellers" << std::endl;
        std::cout << "6. Bank: List Transactions Within a Week" << std::endl;
        std::cout << "7. Bank: List All Customers" << std::endl;
        std::cout << "8. Bank: List Dormant Accounts" << std::endl;
        std::cout << "9. Bank: List Top N Users Today" << std::endl;
        std::cout << "10. Kembali ke Main Menu" << std::endl;
        std::cout << "Pilih Opsi: ";

        if (!(std::cin >> choice))
        {
            choice = 0;
            std::cin.clear();
        }
        clear_input();

        switch (choice)
        {
        case 1:
        { // List Store Transactions (k Days)
            int k = get_int_input("Masukkan k hari terakhir: ");
            Store::getInstance().listTransactionsLastKDays(k);
            break;
        }
        case 2: // List Paid but Uncompleted Transactions
            Store::getInstance().listPaidUncompletedTransactions();
            break;
        case 3:
        { // List Most Frequent Items
            int m = get_int_input("Masukkan m (jumlah top item): ");
            Store::getInstance().listMostFrequentItems(m);
            break;
        }
        case 4:
        { // List Most Active Buyers
            int m = get_int_input("Masukkan m (jumlah top buyer): ");
            Store::getInstance().listMostActiveBuyers(m);
            break;
        }
        case 5:
        { // List Most Active Sellers
            int m = get_int_input("Masukkan m (jumlah top seller): ");
            Store::getInstance().listMostActiveSellers(m);
            break;
        }
        case 6: // Bank: List Transactions Within a Week
            Bank::getInstance().listTransactionsWithinAWeek();
            break;
        case 7: // Bank: List All Customers
            Bank::getInstance().listAllCustomers();
            break;
        case 8: // Bank: List Dormant Accounts
            Bank::getInstance().listDormantAccounts();
            break;
        case 9:
        { // Bank: List Top N Users Today
            int n = get_int_input("Masukkan n (jumlah top user): ");
            Bank::getInstance().listTopNUsersToday(n);
            break;
        }
        case 10:
            return;
        default:
            std::cout << "Pilihan tidak valid." << std::endl;
        }
    } while (choice != 10);
}

void menu_main()
{
    int choice;
    do
    {
        std::cout << "\n=== SIMULASI BUYER-SELLER ===" << std::endl;
        if (current_user)
        {
            std::cout << "Logged in as: " << current_user->getUsername() << " ("
                      << (std::dynamic_pointer_cast<Seller>(current_user) ? "Seller" : "Buyer") << ")" << std::endl;
        }
        else
        {
            std::cout << "1. Register Buyer" << std::endl;
            std::cout << "2. Register Seller" << std::endl;
            std::cout << "3. Login" << std::endl;
            std::cout << "4. Akses Menu Management (Store & Bank)" << std::endl;
            std::cout << "5. Keluar dan Simpan Data" << std::endl;
        }
        std::cout << "Pilih Opsi: ";

        if (!(std::cin >> choice))
        {
            choice = 0;
            std::cin.clear();
        }
        clear_input();

        if (current_user)
        {
            if (std::dynamic_pointer_cast<Seller>(current_user))
            {
                menu_seller();
            }
            else
            {
                menu_buyer();
            }
        }
        else
        {
            switch (choice)
            {
            case 1:
            { // Register Buyer
                std::string user, pass;
                std::cout << "Username: ";
                std::getline(std::cin, user);
                std::cout << "Password: ";
                std::getline(std::cin, pass);
                Store::getInstance().registerUser(user, pass, false);
                break;
            }
            case 2:
            { // Register Seller
                std::string user, pass;
                std::cout << "Username: ";
                std::getline(std::cin, user);
                std::cout << "Password: ";
                std::getline(std::cin, pass);
                Store::getInstance().registerUser(user, pass, true);
                break;
            }
            case 3:
            { // Login
                std::string user, pass;
                std::cout << "Username: ";
                std::getline(std::cin, user);
                std::cout << "Password: ";
                std::getline(std::cin, pass);
                current_user = Store::getInstance().login(user, pass);
                break;
            }
            case 4: // Management Menu
                menu_store_bank_management();
                break;
            case 5:
                std::cout << "Menyimpan data dan Keluar..." << std::endl;
                DataPersistence::saveData();
                break;
            default:
                std::cout << "Pilihan tidak valid." << std::endl;
            }
        }
    } while (choice != 5 || current_user); // Lanjutkan loop selama belum memilih keluar atau masih login
}

int main()
{
    // Memuat data saat aplikasi dimulai (Simulasi Data Persistence)
    DataPersistence::loadData();

    // Jalankan menu utama
    menu_main();

    return 0;
}