#include <iostream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <regex>
#include <string>

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

// Baþlangýçtaki tarihi kontrol et ve yeni isim oluþtur
std::string processFileName(const std::string& fileName) {
    std::regex dateRegex(R"(^\d{8}\d{4}\s)"); // YYYYMMDDHHMM formatýnda tarih kontrolü (boþluk dahil)
    std::string currentDate = getCurrentDate();

    if (std::regex_search(fileName, dateRegex)) {
        // Tarih varsa güncelle
        return std::regex_replace(fileName, dateRegex, currentDate + " ");
    }
    else {
        // Tarih yoksa baþýna ekle
        return currentDate + " " + fileName;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Bir dosya sürükleyip býrakmanýz gerekiyor.\n";
        return 1;
    }

    try {
        fs::path filePath = argv[1]; // Sürüklenen dosyanýn yolu
        if (!fs::exists(filePath)) {
            std::cerr << "Dosya bulunamadý: " << filePath << "\n";
            return 1;
        }

        // Dosya adýný iþle
        std::string newFileName = processFileName(filePath.filename().string());
        fs::path parentDir = filePath.parent_path();
        fs::path newFilePath = parentDir / newFileName;

        fs::rename(filePath, newFilePath);

        std::cout << "Dosya baþarýyla yeniden adlandýrýldý: " << newFilePath << "\n";
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "Bir hata oluþtu: " << e.what() << "\n";
        return 1;
    }

    return 0;
}