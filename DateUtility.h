// File: DateUtility.h

#ifndef DATEUTILITY_H
#define DATEUTILITY_H

#include <ctime>
#include <iostream>
#include <iomanip>
#include <chrono>

class DateUtility
{
public:
    // Mendapatkan waktu saat ini sebagai time_t
    static time_t getCurrentTime()
    {
        return std::time(nullptr);
    }

    // Mengubah time_t menjadi string yang mudah dibaca
    static std::string timeToString(time_t time)
    {
        std::tm *ltm = std::localtime(&time);
        std::stringstream ss;
        ss << std::put_time(ltm, "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    // Mendapatkan waktu (time_t) dari k hari yang lalu
    static time_t getPastDays(int k)
    {
        // 86400 adalah jumlah detik dalam sehari
        return getCurrentTime() - (k * 86400);
    }

    // Mendapatkan waktu (time_t) dari sebulan yang lalu (~30 hari)
    static time_t getPastMonth()
    {
        return getPastDays(30);
    }
};

#endif // DATEUTILITY_H