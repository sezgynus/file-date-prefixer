#include <iostream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <regex>
#include <string>
#include "resources.h"

namespace fs = std::filesystem;

// Tarih formatlama fonksiyonu
std::string getCurrentDate() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::tm localTime;
    localtime_s(&localTime, &in_time_t); // Güvenli sürüm kullanýldý

    std::ostringstream ss;
    ss << (1900 + localTime.tm_year)    // Yýl (YYYY)
        << std::setfill('0') << std::setw(2) << (1 + localTime.tm_mon) // Ay (MM)
        << std::setfill('0') << std::setw(2) << localTime.tm_mday   // Gün (DD)
        << std::setfill('0') << std::setw(2) << localTime.tm_hour   // Saat (HH)
        << std::setfill('0') << std::setw(2) << localTime.tm_min;   // Dakika (MM)
    return ss.str();
}

// Dosyanýn son deðiþtirilme tarihini al
std::string getFileModificationDate(const fs::path& filePath) {
    auto ftime = fs::last_write_time(filePath);

    // Dosya saatini sistem saatine dönüþtür
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
    );

    auto in_time_t = std::chrono::system_clock::to_time_t(sctp);

    std::tm localTime;
    localtime_s(&localTime, &in_time_t);

    std::ostringstream ss;
    ss << (1900 + localTime.tm_year)    // Yýl (YYYY)
        << std::setfill('0') << std::setw(2) << (1 + localTime.tm_mon) // Ay (MM)
        << std::setfill('0') << std::setw(2) << localTime.tm_mday   // Gün (DD)
        << std::setfill('0') << std::setw(2) << localTime.tm_hour   // Saat (HH)
        << std::setfill('0') << std::setw(2) << localTime.tm_min;   // Dakika (MM)
    return ss.str();
}

// Baþlangýçtaki tarihi kontrol et ve yeni isim oluþtur
std::string processFileName(const std::string& fileName, const std::string& date) {
    std::regex dateRegex(R"(^\d{8}\d{4}\s)"); // YYYYMMDDHHMM formatýnda tarih kontrolü (boþluk dahil)

    if (std::regex_search(fileName, dateRegex)) {
        // Tarih varsa güncelle
        return std::regex_replace(fileName, dateRegex, date + " ");
    }
    else {
        // Tarih yoksa baþýna ekle
        return date + " " + fileName;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Dosya adý girmelisiniz.\n";
        return 1;
    }

    std::string timeOption = "--use-system-time"; // Varsayýlan olarak sistem zamaný
    if (argc >= 3) {
        timeOption = argv[2]; // Zaman seçimi argümaný (eðer varsa)
    }

    if (timeOption != "--use-system-time" && timeOption != "--use-file-time") {
        std::cerr << "Geçersiz zaman seçimi! --use-system-time veya --use-file-time kullanýn.\n";
        return 1;
    }

    try {
        fs::path filePath = argv[1]; // Sürüklenen dosyanýn yolu
        if (!fs::exists(filePath)) {
            std::cerr << "Dosya bulunamadý: " << filePath << "\n";
            return 1;
        }

        std::string date;
        if (timeOption == "--use-system-time") {
            date = getCurrentDate(); // Sistem zamaný
        }
        else {
            date = getFileModificationDate(filePath); // Dosya deðiþtirilme zamaný
        }
        printf("Date: %s\n", date);
        // Dosya adýný iþle
        std::string newFileName = processFileName(filePath.filename().string(), date);
        fs::path parentDir = filePath.parent_path();
        fs::path newFilePath = parentDir / newFileName;

        fs::rename(filePath, newFilePath);

        std::cout << "Dosya baþarýyla yeniden adlandýrýldý: " << newFilePath << " Date:"<< date<< "\n";
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "Bir hata oluþtu: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
