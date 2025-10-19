// File: Buyer.h

#ifndef BUYER_H
#define BUYER_H

#include "User.h"

class Buyer : public User {
private:
    std::vector<std::string> orderIds; // Hanya ID order untuk membatasi referensi objek

public:
    Buyer(const std::string& id, const std::string& user, const std::string& pass)
        : User(id, user, pass) {}
        
    void addOrderId(const std::string& orderId) {
        orderIds.push_back(orderId);
    }
    
    const std::vector<std::string>& getOrderIds() const {
        return orderIds;
    }

    // Check spending the last k days [cite: 40] (Fitur ini akan diimplementasikan oleh Store)
    
    // Implementasi toString untuk serialisasi
    std::string toString() const override {
        // Format: BUYER,ID,Username,Password,Order1_ID|Order2_ID|...
        std::string orderList;
        for (const auto& id : orderIds) {
            orderList += id + "|";
        }
        if (!orderList.empty()) {
            orderList.pop_back(); // Hapus "|" terakhir
        }
        return "BUYER," + userId + "," + username + "," + password + "," + orderList;
    }
};

#endif // BUYER_H