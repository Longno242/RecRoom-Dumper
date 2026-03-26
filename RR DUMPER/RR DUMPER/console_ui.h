#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <functional>

namespace ConsoleUI {
    void Initialize();
    void Shutdown();
    void Clear();
    void DrawHeader();
    void DrawFooter();
    void DrawBox(int x, int y, int width, int height, const std::string& title = "");
    void DrawProgressBar(int x, int y, int width, float progress, const std::string& label);
    void DrawSpinner(int x, int y, int frame);
    void DrawLog(int x, int y, int width, int height, const std::vector<std::string>& logs);
    void SetColor(int color);
    void ResetColor();
    void MoveCursor(int x, int y);
    void HideCursor();
    void ShowCursor();
    void ShowDumperUI(const std::function<void()>& dumpCallback);

    const int COLOR_GREEN = 10;
    const int COLOR_CYAN = 11;
    const int COLOR_RED = 12;
    const int COLOR_YELLOW = 14;
    const int COLOR_WHITE = 15;
    const int COLOR_GRAY = 8;
    const int COLOR_DARK_GREEN = 2;
}