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

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    if (argc < 2) {
        MessageBox(NULL, L"Dosya ad� girmelisiniz.", L"Hata", MB_OK | MB_ICONERROR);
        return 1;
    }

    std::wstring timeOption = L"--use-system-time"; // Varsay�lan olarak sistem zaman�
    if (argc >= 3) {
        timeOption = std::wstring(argv[2]); // Zaman se�imi arg�man� (e�er varsa)
    }

    if (timeOption != L"--use-system-time" && timeOption != L"--use-file-time") {
        MessageBox(NULL, L"Ge�ersiz zaman se�imi! --use-system-time veya --use-file-time kullan�n.", L"Hata", MB_OK | MB_ICONERROR);
        return 1;
    }

    try {
        fs::path filePath = argv[1]; // S�r�klenen dosyan�n yolu
        if (!fs::exists(filePath)) {
            MessageBox(NULL, (L"Dosya bulunamad�: " + filePath.wstring()).c_str(), L"Hata", MB_OK | MB_ICONERROR);
            return 1;
        }

        std::string date;
        if (timeOption == L"--use-system-time") {
            date = getCurrentDate(); // Sistem zaman�
        }
        else {
            date = getFileModificationDate(filePath); // Dosya de�i�tirilme zaman�
        }
        // Dosya ad�n� i�le
        std::string newFileName = processFileName(filePath.filename().string(), date);
        fs::path parentDir = filePath.parent_path();
        fs::path newFilePath = parentDir / newFileName;

        fs::rename(filePath, newFilePath);
    }
    catch (const fs::filesystem_error& e) {
        MessageBoxA(NULL, e.what(), "Bir hata olu�tu", MB_OK | MB_ICONERROR);
        return 1;
    }

    return 0;
}