#include <iostream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <regex>
#include <string>
#include "resources.h"
#include <windows.h>

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

// Dosya isminden tarih prefix'ini kaldýr
std::string removeDatePrefix(const std::string& fileName) {
    std::regex dateRegex(R"(^\d{8}\d{4}\s)"); // YYYYMMDDHHMM formatýnda tarih kontrolü (boþluk dahil)
    return std::regex_replace(fileName, dateRegex, "");
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    if (argc < 2) {
        MessageBox(NULL, L"Dosya adý girmelisiniz.", L"Hata", MB_OK | MB_ICONERROR);
        return 1;
    }

    std::wstring timeOption = L"--use-system-time"; // Varsayýlan olarak sistem zamaný
    bool removeDate = false;

    if (argc >= 3) {
        if (std::wstring(argv[2]) == L"--remove-date") {
            removeDate = true; // Tarihi kaldýrma seçeneði
        }
        else {
            timeOption = std::wstring(argv[2]); // Zaman seçimi argümaný
        }
    }

    try {
        fs::path filePath = argv[1]; // Sürüklenen dosyanýn yolu
        if (!fs::exists(filePath)) {
            MessageBox(NULL, (L"Dosya bulunamadý: " + filePath.wstring()).c_str(), L"Hata", MB_OK | MB_ICONERROR);
            return 1;
        }

        if (removeDate) {
            // Tarih prefix'ini kaldýr
            std::string newFileName = removeDatePrefix(filePath.filename().string());
            fs::path parentDir = filePath.parent_path();
            fs::path newFilePath = parentDir / newFileName;

            fs::rename(filePath, newFilePath);
        }
        else {
            std::string date;
            if (timeOption == L"--use-system-time") {
                date = getCurrentDate(); // Sistem zamaný
            }
            else if (timeOption == L"--use-file-time") {
                date = getFileModificationDate(filePath); // Dosya deðiþtirilme zamaný
            }

            // Dosya adýný iþle
            std::string newFileName = processFileName(filePath.filename().string(), date);
            fs::path parentDir = filePath.parent_path();
            fs::path newFilePath = parentDir / newFileName;

            fs::rename(filePath, newFilePath);
        }
    }
    catch (const fs::filesystem_error& e) {
        MessageBoxA(NULL, e.what(), "Bir hata oluþtu", MB_OK | MB_ICONERROR);
        return 1;
    }

    return 0;
}