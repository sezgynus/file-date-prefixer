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
    localtime_s(&localTime, &in_time_t); // G�venli s�r�m kullan�ld�

    std::ostringstream ss;
    ss << (1900 + localTime.tm_year)    // Y�l (YYYY)
        << std::setfill('0') << std::setw(2) << (1 + localTime.tm_mon) // Ay (MM)
        << std::setfill('0') << std::setw(2) << localTime.tm_mday   // G�n (DD)
        << std::setfill('0') << std::setw(2) << localTime.tm_hour   // Saat (HH)
        << std::setfill('0') << std::setw(2) << localTime.tm_min;   // Dakika (MM)
    return ss.str();
}

// Ba�lang��taki tarihi kontrol et ve yeni isim olu�tur
std::string processFileName(const std::string& fileName) {
    std::regex dateRegex(R"(^\d{8}\d{4}\s)"); // YYYYMMDDHHMM format�nda tarih kontrol� (bo�luk dahil)
    std::string currentDate = getCurrentDate();

    if (std::regex_search(fileName, dateRegex)) {
        // Tarih varsa g�ncelle
        return std::regex_replace(fileName, dateRegex, currentDate + " ");
    }
    else {
        // Tarih yoksa ba��na ekle
        return currentDate + " " + fileName;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Bir dosya s�r�kleyip b�rakman�z gerekiyor.\n";
        return 1;
    }

    try {
        fs::path filePath = argv[1]; // S�r�klenen dosyan�n yolu
        if (!fs::exists(filePath)) {
            std::cerr << "Dosya bulunamad�: " << filePath << "\n";
            return 1;
        }

        // Dosya ad�n� i�le
        std::string newFileName = processFileName(filePath.filename().string());
        fs::path parentDir = filePath.parent_path();
        fs::path newFilePath = parentDir / newFileName;

        fs::rename(filePath, newFilePath);

        std::cout << "Dosya ba�ar�yla yeniden adland�r�ld�: " << newFilePath << "\n";
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "Bir hata olu�tu: " << e.what() << "\n";
        return 1;
    }

    return 0;
}