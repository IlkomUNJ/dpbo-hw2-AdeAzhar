// File: Seller.h

#ifndef SELLER_H
#define SELLER_H

#include <map>
#include "Buyer.h"
#include "Item.h"

class Seller : public Buyer {
private:
    std::map<std::string, Item> items; // Seller manage stock items [cite: 7]

public:
    Seller(const std::string& id, const std::string& user, const std::string& pass)
        : Buyer(id, user, pass) {}

    // Manajemen Item [cite: 43]
    void registerNewItem(const std::string& itemId, const std::string& name, double price, int stock) { // Register new item [cite: 44]
        items.emplace(itemId, Item(itemId, name, price, stock));
        // Set price per item [cite: 46] dilakukan saat registrasi
    }

    Item* getItem(const std::string& itemId) {
        if (items.count(itemId)) {
            return &items.at(itemId);
        }
        return nullptr;
    }
    
    // Item can be replenished, discarded [cite: 45]
    bool replenishStock(const std::string& itemId, int quantity) {
        Item* item = getItem(itemId);
        if (item) {
            item->setStock(item->getStock() + quantity);
            return true;
        }
        return false;
    }

    bool discardStock(const std::string& itemId, int quantity) {
        Item* item = getItem(itemId);
        if (item && item->getStock() >= quantity) {
            item->setStock(item->getStock() - quantity);
            if (item->getStock() == 0) {
                // Opsional: Hapus item jika stok nol
                // items.erase(itemId); 
            }
            return true;
        }
        return false;
    }

    // Getter untuk semua item
    const std::map<std::string, Item>& getAllItems() const {
        return items;
    }

    // Implementasi toString untuk serialisasi
    std::string toString() const override {
        // Format: SELLER,ID,Username,Password,Order1_ID|...| , Item1|Item2|...
        std::string base = Buyer::toString();
        
        // Tambahkan item yang dimiliki
        std::string itemList;
        for (const auto& pair : items) {
            // Item format: ID,Name,Price,Stock
            itemList += pair.second.toString() + ";"; 
        }
        if (!itemList.empty()) {
            itemList.pop_back(); // Hapus ";" terakhir
        }

        // Ganti 'BUYER' di base string menjadi 'SELLER'
        base.replace(0, 5, "SELLER");
        return base + "|" + itemList;
    }
};

#endif // SELLER_H