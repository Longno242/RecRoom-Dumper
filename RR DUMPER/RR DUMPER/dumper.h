#pragma once
#include "rrid.hpp"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <sstream>
#include <algorithm>
#include <set>
#include <string>
#include <vector>
#include <functional>

class RridImage;
class RridClass;

class GameDumper {
public:
    static bool DumpAll(const std::string& output_dir = "dump",
        std::function<void(const std::string&)> logCallback = nullptr);

private:
    static std::function<void(const std::string&)> s_logCallback;
    static std::string GetAccessLevelString(uint32_t flags, bool is_method = false);
    static std::string GetFieldModifiers(uint32_t flags);
    static std::string GetMethodModifiers(uint32_t flags);
    static std::string GetCSharpAccessLevel(uint32_t flags, bool is_method = false);
    static std::string GetCSharpFieldModifiers(uint32_t flags);
    static std::string GetCSharpMethodModifiers(uint32_t flags);
    static std::string GetCurrentTimestamp();
    static std::string SanitizeFilename(const std::string& name);
    static std::string SanitizeTypeName(const std::string& type);
    static std::string GetCSharpTypeName(const std::string& il2cpp_type);

    static void CreateDirectoryIfNotExists(const std::string& path);
    static void WriteCppOffsetsFile(const std::vector<RridImage*>& images, const std::string& output_dir);
    static void WriteCppDumpFile(const std::vector<RridImage*>& images, const std::string& output_dir);
    static void WriteCSharpOffsetsFile(const std::vector<RridImage*>& images, const std::string& output_dir);
    static void WriteCSharpDumpFile(const std::vector<RridImage*>& images, const std::string& output_dir);
    static void WriteClassToCppDump(RridClass* cls, std::ofstream& file);
    static void WriteClassToCSharpDump(RridClass* cls, std::ofstream& file);
};