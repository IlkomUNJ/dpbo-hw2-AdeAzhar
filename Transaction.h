// File: Transaction.h

#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <string>
#include "DateUtility.h"

// Enum untuk Status Transaksi (lebih baik daripada string)
enum class TransactionStatus
{
    PAID,      // Dibayar, tetapi belum selesai [cite: 9, 17]
    COMPLETED, // Selesai
    CANCELLED  // Dibatalkan
};

// Enum untuk Tipe Transaksi (untuk cash flow bank)
enum class TransactionType
{
    PURCHASE,
    TOPUP,
    WITHDRAW
};

class Transaction
{
private:
    std::string transactionId;
    std::string itemId;
    std::string buyerId;
    std::string sellerId;
    double amount;
    int quantity;
    time_t date;
    TransactionStatus status;
    TransactionType type; // Digunakan untuk transaksi bank/non-toko

public:
    // Konstruktor untuk transaksi toko (Pembelian)
    Transaction(const std::string &tId, const std::string &iId, const std::string &bId, const std::string &sId,
                double amt, int qty)
        : transactionId(tId), itemId(iId), buyerId(bId), sellerId(sId),
          amount(amt), quantity(qty), date(DateUtility::getCurrentTime()),
          status(TransactionStatus::PAID), type(TransactionType::PURCHASE) {}

    // Konstruktor untuk transaksi Bank (Topup/Withdraw)
    Transaction(const std::string &tId, const std::string &uId, double amt,
                TransactionType t)
        : transactionId(tId), itemId("N/A"), buyerId(uId), sellerId("N/A"),
          amount(amt), quantity(1), date(DateUtility::getCurrentTime()),
          status(TransactionStatus::COMPLETED), type(t) {}

    // Getter
    std::string getId() const { return transactionId; }
    std::string getItemId() const { return itemId; }
    std::string getBuyerId() const { return buyerId; }
    std::string getSellerId() const { return sellerId; }
    double getAmount() const { return amount; }
    time_t getDate() const { return date; }
    TransactionStatus getStatus() const { return status; }
    TransactionType getType() const { return type; }
    int getQuantity() const { return quantity; }

    // Setter
    void setStatus(TransactionStatus s) { status = s; }

    // Metode untuk representasi output
    std::string toString() const
    {
        // String format: ID,ItemID,BuyerID,SellerID,Amount,Quantity,Date(time_t),Status(int),Type(int)
        return transactionId + "," + itemId + "," + buyerId + "," + sellerId + "," +
               std::to_string(amount) + "," + std::to_string(quantity) + "," +
               std::to_string(date) + "," + std::to_string(static_cast<int>(status)) + "," +
               std::to_string(static_cast<int>(type));
    }
};

#endif // TRANSACTION_H