#include "console_ui.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <sstream>
#include <iomanip>

#define _CRT_SECURE_NO_WARNINGS

static HANDLE hConsole;
static int consoleWidth = 100;
static int consoleHeight = 30;

void ConsoleUI::Initialize() {
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    COORD bufferSize = { (SHORT)consoleWidth, (SHORT)consoleHeight };
    SetConsoleScreenBufferSize(hConsole, bufferSize);
    SMALL_RECT windowSize = { 0, 0, (SHORT)(consoleWidth - 1), (SHORT)(consoleHeight - 1) };
    SetConsoleWindowInfo(hConsole, TRUE, &windowSize);
    DWORD mode;
    GetConsoleMode(hConsole, &mode);
    SetConsoleMode(hConsole, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    HideCursor();
    Clear();
}

void ConsoleUI::Shutdown() {
    ShowCursor();
    ResetColor();
    Clear();
}

void ConsoleUI::Clear() {
    system("cls");
}

void ConsoleUI::SetColor(int color) {
    SetConsoleTextAttribute(hConsole, color);
}

void ConsoleUI::ResetColor() {
    SetConsoleTextAttribute(hConsole, 7);
}

void ConsoleUI::MoveCursor(int x, int y) {
    COORD pos = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(hConsole, pos);
}

void ConsoleUI::HideCursor() {
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &info);
}

void ConsoleUI::ShowCursor() {
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = TRUE;
    SetConsoleCursorInfo(hConsole, &info);
}

void ConsoleUI::DrawHeader() {
    SetColor(COLOR_CYAN);

    MoveCursor(0, 0);
    std::cout << "╔";
    for (int i = 0; i < consoleWidth - 2; i++) std::cout << "═";
    std::cout << "╗";

    MoveCursor(2, 1);
    std::cout << "  ██████╗ ███████╗██╗██████╗     ██████╗ ██╗   ██╗███╗   ███╗██████╗ ███████╗██████╗ ";
    MoveCursor(2, 2);
    std::cout << "  ██╔══██╗██╔════╝██║██╔══██╗    ██╔══██╗██║   ██║████╗ ████║██╔══██╗██╔════╝██╔══██╗";
    MoveCursor(2, 3);
    std::cout << "  ██████╔╝█████╗  ██║██║  ██║    ██║  ██║██║   ██║██╔████╔██║██████╔╝█████╗  ██████╔╝";
    MoveCursor(2, 4);
    std::cout << "  ██╔══██╗██╔══╝  ██║██║  ██║    ██║  ██║██║   ██║██║╚██╔╝██║██╔══██╗██╔══╝  ██╔══██╗";
    MoveCursor(2, 5);
    std::cout << "  ██║  ██║██║     ██║██████╔╝    ██████╔╝╚██████╔╝██║ ╚═╝ ██║██████╔╝███████╗██║  ██║";
    MoveCursor(2, 6);
    std::cout << "  ╚═╝  ╚═╝╚═╝     ╚═╝╚═════╝     ╚═════╝  ╚═════╝ ╚═╝     ╚═╝╚═════╝ ╚══════╝╚═╝  ╚═╝";

    MoveCursor(0, 7);
    std::cout << "╠";
    for (int i = 0; i < consoleWidth - 2; i++) std::cout << "═";
    std::cout << "╣";

    ResetColor();
}

void ConsoleUI::DrawBox(int x, int y, int width, int height, const std::string& title) {
    SetColor(COLOR_GRAY);

    MoveCursor(x, y);
    std::cout << "┌";
    for (int i = 0; i < width - 2; i++) std::cout << "─";
    std::cout << "┐";

    if (!title.empty()) {
        MoveCursor(x + 2, y);
        std::cout << " " << title << " ";
    }

    for (int i = 1; i < height - 1; i++) {
        MoveCursor(x, y + i);
        std::cout << "│";
        MoveCursor(x + width - 1, y + i);
        std::cout << "│";
    }

    MoveCursor(x, y + height - 1);
    std::cout << "└";
    for (int i = 0; i < width - 2; i++) std::cout << "─";
    std::cout << "┘";

    ResetColor();
}

void ConsoleUI::DrawProgressBar(int x, int y, int width, float progress, const std::string& label) {
    int filled = (int)((width - 2) * progress);

    MoveCursor(x, y - 1);
    SetColor(COLOR_WHITE);
    std::cout << label;

    MoveCursor(x, y);
    SetColor(COLOR_GRAY);
    std::cout << "│";

    for (int i = 0; i < width - 2; i++) {
        if (i < filled) {
            SetColor(COLOR_GREEN);
            std::cout << "█";
        }
        else {
            SetColor(COLOR_GRAY);
            std::cout << "░";
        }
    }

    SetColor(COLOR_GRAY);
    std::cout << "│";

    MoveCursor(x + width + 1, y);
    SetColor(COLOR_CYAN);
    std::cout << std::setw(3) << (int)(progress * 100) << "%";

    ResetColor();
}

void ConsoleUI::DrawSpinner(int x, int y, int frame) {
    const char* spinner = "⠋⠙⠹⠸⠼⠴⠦⠧⠇⠏";
    MoveCursor(x, y);
    SetColor(COLOR_YELLOW);
    std::cout << spinner[frame % 10];
    ResetColor();
}

void ConsoleUI::DrawLog(int x, int y, int width, int height, const std::vector<std::string>& logs) {
    DrawBox(x, y, width, height, "LOG");

    int startIdx = logs.size() > (height - 2) ? logs.size() - (height - 2) : 0;

    for (int i = 0; i < height - 2 && (startIdx + i) < logs.size(); i++) {
        MoveCursor(x + 1, y + 1 + i);

        const std::string& log = logs[startIdx + i];

        if (log.find("[+]") != std::string::npos) SetColor(COLOR_GREEN);
        else if (log.find("[!]") != std::string::npos) SetColor(COLOR_RED);
        else if (log.find("[*]") != std::string::npos) SetColor(COLOR_CYAN);
        else if (log.find("[~]") != std::string::npos) SetColor(COLOR_YELLOW);
        else SetColor(COLOR_GRAY);

        std::string display = log;
        if (display.length() > width - 2) {
            display = display.substr(0, width - 5) + "...";
        }

        std::cout << display;

        for (size_t j = display.length(); j < width - 2; j++) {
            std::cout << " ";
        }
    }

    ResetColor();
}

void ConsoleUI::DrawFooter() {
    SetColor(COLOR_DARK_GREEN);
    MoveCursor(0, consoleHeight - 1);
    std::cout << "╚";
    for (int i = 0; i < consoleWidth - 2; i++) std::cout << "═";
    std::cout << "╝";
    ResetColor();
}

void ConsoleUI::ShowDumperUI(const std::function<void()>& dumpCallback) {
    Initialize();

    std::vector<std::string> logs;
    auto addLog = [&logs](const std::string& msg, const std::string& type = "[*]") {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;

        struct tm timeinfo;
        localtime_s(&timeinfo, &time);
        ss << "[" << std::put_time(&timeinfo, "%H:%M:%S") << "] "
            << type << " " << msg;
        logs.push_back(ss.str());
        };

    bool running = true;
    int spinnerFrame = 0;
    float progress = 0.0f;

    std::string currentTask = "Initializing...";
    std::thread dumpThread([&]() {
        addLog("Starting dump process...", "[*]");

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        progress = 0.1f;
        currentTask = "Initializing RRID...";
        addLog("Initializing RRID...", "[*]");

        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        progress = 0.2f;
        currentTask = "Scanning images...";
        addLog("Scanning for IL2CPP images...", "[*]");

        dumpCallback();

        progress = 1.0f;
        currentTask = "Complete!";
        addLog("Dump completed successfully!", "[+]");

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        running = false;
        });

    while (running) {
        Clear();
        DrawHeader();
        DrawBox(2, 9, 40, 6, "STATUS");
        MoveCursor(4, 11);
        SetColor(COLOR_WHITE);
        std::cout << "Task: ";
        SetColor(COLOR_YELLOW);
        std::cout << currentTask;
        DrawProgressBar(4, 13, 32, progress, "");
        DrawBox(44, 9, 54, 6, "STATISTICS");
        MoveCursor(46, 11);
        SetColor(COLOR_GRAY);
        std::cout << "Images: ";
        SetColor(COLOR_CYAN);
        std::cout << "Scanning...";
        MoveCursor(46, 13);
        SetColor(COLOR_GRAY);
        std::cout << "Output: ";
        SetColor(COLOR_CYAN);
        std::cout << "./dump/";
        DrawLog(2, 16, 96, 12, logs);
        DrawSpinner(90, 11, spinnerFrame);
        DrawFooter();

        spinnerFrame++;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    dumpThread.join();

    Clear();
    DrawHeader();
    SetColor(COLOR_GREEN);
    MoveCursor(35, 12);
    std::cout << "✓ DUMP COMPLETED SUCCESSFULLY!";
    SetColor(COLOR_WHITE);
    MoveCursor(30, 14);
    std::cout << "Files generated in ./dump/:";
    SetColor(COLOR_CYAN);
    MoveCursor(35, 16);
    std::cout << "├── GameOffsets.h  (C++ offsets)";
    MoveCursor(35, 17);
    std::cout << "├── GameDump.cpp   (C++ classes)";
    MoveCursor(35, 18);
    std::cout << "├── GameOffsets.cs (C# offsets)";
    MoveCursor(35, 19);
    std::cout << "└── GameDump.cs    (C# classes)";
    SetColor(COLOR_GRAY);
    MoveCursor(30, 22);
    std::cout << "Press any key to exit...";
    ResetColor();
    ShowCursor();
    system("pause > nul");
    Shutdown();
}