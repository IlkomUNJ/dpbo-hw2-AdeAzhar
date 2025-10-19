// File: Item.h

#ifndef ITEM_H
#define ITEM_H

#include <string>

class Item
{
private:
    std::string itemId;
    std::string name;
    double price;
    int stock;

public:
    Item(const std::string &id, const std::string &n, double p, int s)
        : itemId(id), name(n), price(p), stock(s) {}

    // Getter
    std::string getId() const { return itemId; }
    std::string getName() const { return name; }
    double getPrice() const { return price; }
    int getStock() const { return stock; }

    // Setter (untuk manajemen stok)
    void setStock(int s) { stock = s; }

    // Metode untuk representasi output (untuk serialisasi, kita akan menggunakan string sederhana)
    std::string toString() const
    {
        return itemId + "," + name + "," + std::to_string(price) + "," + std::to_string(stock);
    }
};

#endif // ITEM_H