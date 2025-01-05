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
    localtime_s(&localTime, &in_time_t); // G�venli s�r�m kullan�ld�

    std::ostringstream ss;
    ss << (1900 + localTime.tm_year)    // Y�l (YYYY)
        << std::setfill('0') << std::setw(2) << (1 + localTime.tm_mon) // Ay (MM)
        << std::setfill('0') << std::setw(2) << localTime.tm_mday   // G�n (DD)
        << std::setfill('0') << std::setw(2) << localTime.tm_hour   // Saat (HH)
        << std::setfill('0') << std::setw(2) << localTime.tm_min;   // Dakika (MM)
    return ss.str();
}

// Dosyan�n son de�i�tirilme tarihini al
std::string getFileModificationDate(const fs::path& filePath) {
    auto ftime = fs::last_write_time(filePath);

    // Dosya saatini sistem saatine d�n��t�r
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
    );

    auto in_time_t = std::chrono::system_clock::to_time_t(sctp);

    std::tm localTime;
    localtime_s(&localTime, &in_time_t);

    std::ostringstream ss;
    ss << (1900 + localTime.tm_year)    // Y�l (YYYY)
        << std::setfill('0') << std::setw(2) << (1 + localTime.tm_mon) // Ay (MM)
        << std::setfill('0') << std::setw(2) << localTime.tm_mday   // G�n (DD)
        << std::setfill('0') << std::setw(2) << localTime.tm_hour   // Saat (HH)
        << std::setfill('0') << std::setw(2) << localTime.tm_min;   // Dakika (MM)
    return ss.str();
}

// Ba�lang��taki tarihi kontrol et ve yeni isim olu�tur
std::string processFileName(const std::string& fileName, const std::string& date) {
    std::regex dateRegex(R"(^\d{8}\d{4}\s)"); // YYYYMMDDHHMM format�nda tarih kontrol� (bo�luk dahil)

    if (std::regex_search(fileName, dateRegex)) {
        // Tarih varsa g�ncelle
        return std::regex_replace(fileName, dateRegex, date + " ");
    }
    else {
        // Tarih yoksa ba��na ekle
        return date + " " + fileName;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Dosya ad� girmelisiniz.\n";
        return 1;
    }

    std::string timeOption = "--use-system-time"; // Varsay�lan olarak sistem zaman�
    if (argc >= 3) {
        timeOption = argv[2]; // Zaman se�imi arg�man� (e�er varsa)
    }

    if (timeOption != "--use-system-time" && timeOption != "--use-file-time") {
        std::cerr << "Ge�ersiz zaman se�imi! --use-system-time veya --use-file-time kullan�n.\n";
        return 1;
    }

    try {
        fs::path filePath = argv[1]; // S�r�klenen dosyan�n yolu
        if (!fs::exists(filePath)) {
            std::cerr << "Dosya bulunamad�: " << filePath << "\n";
            return 1;
        }

        std::string date;
        if (timeOption == "--use-system-time") {
            date = getCurrentDate(); // Sistem zaman�
        }
        else {
            date = getFileModificationDate(filePath); // Dosya de�i�tirilme zaman�
        }
        printf("Date: %s\n", date);
        // Dosya ad�n� i�le
        std::string newFileName = processFileName(filePath.filename().string(), date);
        fs::path parentDir = filePath.parent_path();
        fs::path newFilePath = parentDir / newFileName;

        fs::rename(filePath, newFilePath);

        std::cout << "Dosya ba�ar�yla yeniden adland�r�ld�: " << newFilePath << " Date:"<< date<< "\n";
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "Bir hata olu�tu: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
