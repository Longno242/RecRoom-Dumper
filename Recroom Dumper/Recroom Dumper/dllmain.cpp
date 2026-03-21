#define _CRT_SECURE_NO_WARNINGS

#include "DumpBypass.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <ctime>
#include <string>
#include <vector>
#include <algorithm>
#include <set>

class ConsoleLogger {
private:
    HANDLE hConsole;
    std::ofstream logFile;
    std::ofstream csDumpFile;
    std::ofstream cppDumpFile;

public:
    ConsoleLogger() {
        AllocConsole();

        FILE* fDummy;
        freopen_s(&fDummy, "CONOUT$", "w", stdout);
        freopen_s(&fDummy, "CONIN$", "r", stdin);

        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        logFile.open("rrid_dump.log", std::ios::out | std::ios::app);
        csDumpFile.open("dump.cs", std::ios::out);
        cppDumpFile.open("dump.cpp", std::ios::out);

        if (csDumpFile.is_open()) {
            csDumpFile << "// Auto-generated C# Dump from RRID\n";
            csDumpFile << "// Generated: " << GetCurrentTime() << "\n\n";
            csDumpFile << "using System;\n";
            csDumpFile << "using System.Collections.Generic;\n";
            csDumpFile << "using System.Runtime.InteropServices;\n\n";
            csDumpFile << "namespace RRID_Dump\n";
            csDumpFile << "{\n";
            csDumpFile << "    public static class Offsets\n";
            csDumpFile << "    {\n";
        }

        if (cppDumpFile.is_open()) {
            cppDumpFile << "// Auto-generated C++ Dump from RRID\n";
            cppDumpFile << "// Generated: " << GetCurrentTime() << "\n\n";
            cppDumpFile << "#pragma once\n";
            cppDumpFile << "#include <cstdint>\n\n";
            cppDumpFile << "namespace RRID_Dump\n";
            cppDumpFile << "{\n";
        }

        SetConsoleTitleA("RRID IL2CPP Dumper");

        Log("=", 80);
        Log("RRID IL2CPP Dumper Initialized");
        Log("Time: " + GetCurrentTime());
        Log("=", 80);
    }

    ~ConsoleLogger() {

        if (csDumpFile.is_open()) {
            csDumpFile << "    }\n";
            csDumpFile << "}\n";
            csDumpFile.close();
        }

        if (cppDumpFile.is_open()) {
            cppDumpFile << "}\n";
            cppDumpFile.close();
        }

        Log("=", 80);
        Log("RRID Dumper Shutting Down");
        Log("Dumps saved to: dump.cs and dump.cpp");
        Log("=", 80);

        if (logFile.is_open()) {
            logFile.close();
        }

        system("pause");
        FreeConsole();
    }

    std::string GetCurrentTime() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        struct tm timeInfo;
        char timeStr[100];
        localtime_s(&timeInfo, &time);
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeInfo);
        return std::string(timeStr);
    }

    void Log(const std::string& message, bool toConsole = true, bool toFile = true) {
        if (toConsole) {
            std::cout << message << std::endl;
        }
        if (toFile && logFile.is_open()) {
            logFile << message << std::endl;
            logFile.flush();
        }
    }

    void Log(const std::string& message, int length, char fill = '=') {
        std::string line(length, fill);
        Log(line + " " + message + " " + line);
    }

    void LogSuccess(const std::string& message) {
        SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);
        Log("[+] " + message);
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }

    void LogError(const std::string& message) {
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
        Log("[-] " + message);
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }

    void LogWarning(const std::string& message) {
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
        Log("[!] " + message);
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }

    void LogInfo(const std::string& message) {
        SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN);
        Log("[*] " + message);
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }

    void WriteToDumpFiles(const std::string& csLine, const std::string& cppLine) {
        if (csDumpFile.is_open()) {
            csDumpFile << csLine << std::endl;
        }
        if (cppDumpFile.is_open()) {
            cppDumpFile << cppLine << std::endl;
        }
    }

    void WriteClassStart(const std::string& className, const std::string& namespaceName, uint64_t rva) {
        std::string fullName = namespaceName.empty() ? className : namespaceName + "." + className;

        std::string cs = "        // Class: " + fullName + " (RVA: 0x" + ToHex(rva) + ")";
        std::string cpp = "    // Class: " + fullName + " (RVA: 0x" + ToHex(rva) + ")";
        WriteToDumpFiles(cs, cpp);

        cs = "        public static class " + SanitizeName(className);
        cpp = "    struct " + SanitizeName(className);
        WriteToDumpFiles(cs, cpp);

        WriteToDumpFiles("        {", "    {");
    }

    void WriteClassEnd() {
        WriteToDumpFiles("        }", "    };");
        WriteToDumpFiles("", "");
    }

    void WriteField(const std::string& fieldName, const std::string& fieldType, uint64_t offset, bool isStatic, uint64_t staticRVA = 0) {
        std::string sanitizedName = SanitizeName(fieldName);

        if (isStatic) {
            std::string cs = "            public static IntPtr " + sanitizedName + " = (IntPtr)0x" + ToHex(staticRVA) + "; // RVA: 0x" + ToHex(staticRVA);
            std::string cpp = "        static constexpr uintptr_t " + sanitizedName + " = 0x" + ToHex(staticRVA) + "; // RVA: 0x" + ToHex(staticRVA);
            WriteToDumpFiles(cs, cpp);
        }
        else {
            std::string cs = "            public const int " + sanitizedName + " = 0x" + ToHex(offset) + ";";
            std::string cpp = "        static constexpr uint32_t " + sanitizedName + " = 0x" + ToHex(offset) + ";";
            WriteToDumpFiles(cs, cpp);
        }
    }

    void WriteMethod(const std::string& methodName, const std::string& returnType, uint64_t rva, bool isStatic) {
        std::string sanitizedName = SanitizeName(methodName);

        std::string cs = "            public static IntPtr " + sanitizedName + " = (IntPtr)0x" + ToHex(rva) + "; // RVA: 0x" + ToHex(rva);
        std::string cpp = "        static constexpr uintptr_t " + sanitizedName + " = 0x" + ToHex(rva) + "; // RVA: 0x" + ToHex(rva);
        WriteToDumpFiles(cs, cpp);
    }

private:
    std::string ToHex(uint64_t value) {
        std::stringstream ss;
        ss << std::hex << std::uppercase << value;
        return ss.str();
    }

    std::string SanitizeName(const std::string& name) {
        std::string result = name;
        for (char& c : result) {
            if (!isalnum(c) && c != '_') {
                c = '_';
            }
        }
        if (!result.empty() && isdigit(result[0])) {
            result = "_" + result;
        }
        return result;
    }
};

std::string GetAccessLevelString(uint32_t flags, bool isMethod = false) {
    uint32_t mask = isMethod ? RRID_METHOD_ACCESS_LEVEL_MASK : RRID_FIELD_ACCESS_LEVEL_MASK;
    uint32_t access = flags & mask;

    switch (access) {
    case RRID_FIELD_ACCESS_LEVEL_PRIVATE:
        return "private";
    case RRID_FIELD_ACCESS_LEVEL_PRIVATE_PROTECTED:
        return "private protected";
    case RRID_FIELD_ACCESS_LEVEL_INTERNAL:
        return "internal";
    case RRID_FIELD_ACCESS_LEVEL_PROTECTED:
        return "protected";
    case RRID_FIELD_ACCESS_LEVEL_PROTECTED_INTERNAL:
        return "protected internal";
    case RRID_FIELD_ACCESS_LEVEL_PUBLIC:
        return "public";
    default:
        return "unknown";
    }
}

std::string GetFieldAttributesString(uint32_t flags) {
    std::string attrs;
    if (flags & RRID_FIELD_ATTRIBUTE_STATIC) attrs += "static ";
    if (flags & RRID_FIELD_ATTRIBUTE_READONLY) attrs += "readonly ";
    if (flags & RRID_FIELD_ATTRIBUTE_CONST) attrs += "const ";
    if (attrs.empty()) return "";
    return attrs.substr(0, attrs.length() - 1);
}

std::string GetMethodAttributesString(uint32_t flags) {
    std::string attrs;
    if (flags & RRID_METHOD_ATTRIBUTE_STATIC) attrs += "static ";
    if (flags & RRID_METHOD_ATTRIBUTE_FINAL) attrs += "final ";
    if (flags & RRID_METHOD_ATTRIBUTE_VIRTUAL) attrs += "virtual ";
    if (flags & RRID_METHOD_ATTRIBUTE_ABSTRACT) attrs += "abstract ";
    if (flags & RRID_METHOD_ATTRIBUTE_NEW_SLOT) attrs += "new ";
    if (flags & RRID_METHOD_ATTRIBUTE_PINVOKE_IMPL) attrs += "pinvoke ";
    if (attrs.empty()) return "";
    return attrs.substr(0, attrs.length() - 1);
}

std::string ToHex(uint64_t value) {
    std::stringstream ss;
    ss << std::hex << std::uppercase << value;
    return ss.str();
}

void DumpImageClasses(ConsoleLogger& logger, RridImage* image) {
    if (!image) return;

    logger.LogInfo("Dumping classes from image: " + image->get_name());
    logger.Log("Image RVA: 0x" + ToHex(image->get_image_rva()));
    logger.Log("Module Base: 0x" + ToHex(image->get_module_base()));
    logger.Log("-", 60);

    const auto& classes = image->get_classes();
    logger.LogInfo("Found " + std::to_string(classes.size()) + " classes");

    int classCount = 0;
    std::set<std::string> dumpedClasses;

    for (auto* cls : classes) {
        if (!cls) continue;

        std::string fullName = cls->get_namespace().empty() ?
            cls->get_name() : cls->get_namespace() + "." + cls->get_name();

        if (dumpedClasses.find(fullName) != dumpedClasses.end()) continue;
        dumpedClasses.insert(fullName);

        classCount++;

        logger.Log("");
        logger.Log("┌─ [" + std::to_string(classCount) + "] " + fullName);
        logger.Log("│   Class RVA: 0x" + ToHex(cls->get_class_rva()));
        logger.Log("│   Class Ptr: 0x" + ToHex((uint64_t)cls->get_raw()));

        logger.WriteClassStart(cls->get_name(), cls->get_namespace(), cls->get_class_rva());

        std::string classFlags;
        if (cls->is_enum()) classFlags += "enum ";
        if (cls->is_valuetype()) classFlags += "valuetype ";
        if (cls->is_generic()) classFlags += "generic ";
        if (!classFlags.empty()) {
            logger.Log("│   Flags: " + classFlags);
        }

        const auto& fields = cls->get_fields();
        if (!fields.empty()) {
            logger.Log("│");
            logger.Log("│   📁 Fields (" + std::to_string(fields.size()) + "):");
            for (auto* field : fields) {
                std::string access = GetAccessLevelString(field->get_flags(), false);
                std::string attrs = GetFieldAttributesString(field->get_flags());

                std::string fieldInfo = "│       " + access;
                if (!attrs.empty()) fieldInfo += " " + attrs;
                fieldInfo += " " + field->get_type() + " " + field->get_name();

                if (field->has_rva()) {
                    fieldInfo += " → [RVA: 0x" + ToHex(field->get_static_rva()) +
                        " | Abs: 0x" + ToHex(field->get_static_address()) + "]";
                    logger.WriteField(field->get_name(), field->get_type(), 0, true, field->get_static_rva());
                }
                else {
                    fieldInfo += " → [Offset: 0x" + ToHex(field->get_offset()) + "]";
                    logger.WriteField(field->get_name(), field->get_type(), field->get_offset(), false);
                }

                logger.Log(fieldInfo);
            }
        }

        const auto& methods = cls->get_methods();
        if (!methods.empty()) {
            logger.Log("│");
            logger.Log("│   ⚡ Methods (" + std::to_string(methods.size()) + "):");
            int methodCount = 0;
            for (auto* method : methods) {
                methodCount++;
                std::string access = GetAccessLevelString(method->get_flags(), true);
                std::string attrs = GetMethodAttributesString(method->get_flags());
                std::string params;
                for (size_t i = 0; i < method->get_param_count(); i++) {
                    auto param = method->get_param(i);
                    if (i > 0) params += ", ";
                    params += param.first + " " + param.second;
                }

                std::string methodInfo = "│       " + std::to_string(methodCount) + ". " + access;
                if (!attrs.empty()) methodInfo += " " + attrs;
                methodInfo += " " + method->get_return_type() + " " + method->get_name() + "(" + params + ")";

                if (method->get_method_rva() != 0) {
                    methodInfo += "\n│           🎯 Code RVA: 0x" + ToHex(method->get_method_rva());
                    methodInfo += " | MethodInfo RVA: 0x" + ToHex(method->get_method_info_rva());
                    methodInfo += " | Address: 0x" + ToHex(method->get_absolute_address());

                    logger.WriteMethod(method->get_name(), method->get_return_type(), method->get_method_rva(),
                        method->get_flags() & RRID_METHOD_ATTRIBUTE_STATIC);
                }

                logger.Log(methodInfo);
            }
        }

        const auto& nested = cls->get_nested_types();
        if (!nested.empty()) {
            logger.Log("│");
            logger.Log("│   📦 Nested Types (" + std::to_string(nested.size()) + "):");
            for (auto* nestedCls : nested) {
                std::string nestedName = nestedCls->get_namespace().empty() ?
                    nestedCls->get_name() : nestedCls->get_namespace() + "." + nestedCls->get_name();
                logger.Log("│       " + nestedName + " [RVA: 0x" + ToHex(nestedCls->get_class_rva()) + "]");
            }
        }

        logger.Log("└─");
        logger.WriteClassEnd();

        if (classCount % 50 == 0) {
            Sleep(10);
        }
    }

    logger.LogSuccess("Finished dumping " + std::to_string(classCount) + " classes from " + image->get_name());
}

void DumpSpecificClass(ConsoleLogger& logger, RridImage* image, const std::string& className, const std::string& namespaceName = "") {
    if (!image) {
        logger.LogError("Image is null");
        return;
    }

    RridClass* cls = image->get_class(className, namespaceName);
    if (!cls) {
        logger.LogError("Class not found: " + (namespaceName.empty() ? className : namespaceName + "." + className));
        return;
    }

    std::string fullName = cls->get_namespace().empty() ? cls->get_name() : cls->get_namespace() + "." + cls->get_name();
    logger.LogSuccess("Found class: " + fullName);
    logger.Log("┌─ Class Details");
    logger.Log("│   Class RVA: 0x" + ToHex(cls->get_class_rva()));
    logger.Log("│   Class Address: 0x" + ToHex((uint64_t)cls->get_raw()));
    logger.Log("│");

    logger.Log("│   📁 Fields:");
    for (auto* field : cls->get_fields()) {
        std::string access = GetAccessLevelString(field->get_flags(), false);
        std::string attrs = GetFieldAttributesString(field->get_flags());

        std::stringstream ss;
        ss << "│       " << access;
        if (!attrs.empty()) ss << " " << attrs;
        ss << " " << field->get_type() << " " << field->get_name();

        if (field->has_rva()) {
            ss << "\n│           Static Data: RVA=0x" << std::hex << field->get_static_rva()
                << " | Absolute=0x" << std::hex << field->get_static_address()
                << " | Flags=0x" << std::hex << field->get_flags();
        }
        else {
            ss << "\n│           Instance Offset: 0x" << std::hex << field->get_offset()
                << " | Flags=0x" << std::hex << field->get_flags();
        }

        logger.Log(ss.str());
    }

    logger.Log("│");

    logger.Log("│   ⚡ Methods:");
    int methodIdx = 0;
    for (auto* method : cls->get_methods()) {
        methodIdx++;
        std::string access = GetAccessLevelString(method->get_flags(), true);
        std::string attrs = GetMethodAttributesString(method->get_flags());

        std::stringstream ss;
        ss << "│       " << methodIdx << ". " << access;
        if (!attrs.empty()) ss << " " << attrs;
        ss << " " << method->get_return_type() << " " << method->get_name() << "(";

        for (size_t i = 0; i < method->get_param_count(); i++) {
            auto param = method->get_param(i);
            if (i > 0) ss << ", ";
            ss << param.first << " " << param.second;
        }
        ss << ")";

        ss << "\n│           🎯 Code RVA: 0x" << std::hex << method->get_method_rva()
            << " | MethodInfo RVA: 0x" << std::hex << method->get_method_info_rva()
            << " | Absolute: 0x" << std::hex << method->get_absolute_address()
            << " | Flags: 0x" << std::hex << method->get_flags();

        logger.Log(ss.str());
    }

    logger.Log("└─");
}

void InteractiveMode(ConsoleLogger& logger) {
    logger.LogInfo("Entering interactive mode. Type 'help' for commands.");

    const auto& images = rrid::get_images();
    if (images.empty()) {
        logger.LogError("No images found!");
        return;
    }

    RridImage* currentImage = images[0];
    logger.LogSuccess("Current image: " + currentImage->get_name());

    std::string input;
    while (true) {
        std::cout << "\nRRID> ";
        std::getline(std::cin, input);

        if (input == "exit" || input == "quit") {
            break;
        }
        else if (input == "help") {
            logger.Log("Commands:");
            logger.Log("  list images          - List all loaded images");
            logger.Log("  list classes         - List all classes in current image");
            logger.Log("  use <image_name>     - Switch to a different image");
            logger.Log("  dump <class_name>    - Dump details of a specific class");
            logger.Log("  dump <namespace>.<class> - Dump class with namespace");
            logger.Log("  search <pattern>     - Search for classes containing pattern");
            logger.Log("  exit                 - Exit interactive mode");
        }
        else if (input == "list images") {
            const auto& imgs = rrid::get_images();
            logger.LogInfo("Loaded images:");
            for (size_t i = 0; i < imgs.size(); i++) {
                std::string marker = (imgs[i] == currentImage) ? " -> " : "    ";
                logger.Log(marker + "[" + std::to_string(i) + "] " + imgs[i]->get_name() +
                    " (RVA: 0x" + ToHex(imgs[i]->get_image_rva()) + ")");
            }
        }
        else if (input == "list classes") {
            const auto& classes = currentImage->get_classes();
            logger.LogInfo("Classes in " + currentImage->get_name() + ":");
            int count = 0;
            for (auto* cls : classes) {
                count++;
                std::string fullName = cls->get_namespace().empty() ?
                    cls->get_name() : cls->get_namespace() + "." + cls->get_name();
                logger.Log("  " + std::to_string(count) + ". " + fullName +
                    " [RVA: 0x" + ToHex(cls->get_class_rva()) + "]");
                if (count >= 100) {
                    logger.Log("  ... and " + std::to_string(classes.size() - count) + " more");
                    break;
                }
            }
            logger.Log("Total: " + std::to_string(classes.size()) + " classes");
        }
        else if (input.find("use ") == 0) {
            std::string imageName = input.substr(4);
            RridImage* img = rrid::get_image(imageName);
            if (img) {
                currentImage = img;
                logger.LogSuccess("Switched to image: " + imageName);
            }
            else {
                logger.LogError("Image not found: " + imageName);
            }
        }
        else if (input.find("dump ") == 0) {
            std::string className = input.substr(5);
            size_t dotPos = className.find('.');
            if (dotPos != std::string::npos) {
                std::string ns = className.substr(0, dotPos);
                std::string name = className.substr(dotPos + 1);
                DumpSpecificClass(logger, currentImage, name, ns);
            }
            else {
                DumpSpecificClass(logger, currentImage, className);
            }
        }
        else if (input.find("search ") == 0) {
            std::string pattern = input.substr(7);
            const auto& classes = currentImage->get_classes();
            logger.LogInfo("Searching for classes containing '" + pattern + "'...");
            int found = 0;
            for (auto* cls : classes) {
                std::string fullName = cls->get_namespace().empty() ?
                    cls->get_name() : cls->get_namespace() + "." + cls->get_name();
                if (fullName.find(pattern) != std::string::npos) {
                    logger.Log("  " + fullName + " [RVA: 0x" + ToHex(cls->get_class_rva()) + "]");
                    found++;
                }
            }
            logger.Log("Found " + std::to_string(found) + " matching classes");
        }
        else if (!input.empty()) {
            logger.LogWarning("Unknown command: " + input + ". Type 'help' for available commands.");
        }
    }
}

ConsoleLogger* g_logger = nullptr;

void RunRRIDDumper() {
    if (!g_logger) return;

    g_logger->LogInfo("Initializing RRID...");

    if (!rrid::init()) {
        g_logger->LogError("Failed to initialize RRID!");
        return;
    }

    g_logger->LogSuccess("RRID initialized successfully!");
    const auto& images = rrid::get_images();
    g_logger->LogInfo("Found " + std::to_string(images.size()) + " images:");

    for (const auto* img : images) {
        g_logger->Log("  - " + img->get_name() + " (RVA: 0x" + ToHex(img->get_image_rva()) + ")");
    }

    if (images.empty()) {
        g_logger->LogError("No images found!");
        return;
    }

    g_logger->LogInfo("Starting full dump of all classes...");
    g_logger->Log("");

    for (auto* img : images) {
        DumpImageClasses(*g_logger, img);
        g_logger->Log("", 80);
    }

    g_logger->LogSuccess("Full dump completed!");
    g_logger->LogInfo("Dumps saved to: dump.cs and dump.cpp");
    g_logger->LogInfo("Log saved to: rrid_dump.log");

    InteractiveMode(*g_logger);
}

// DLL Entry Point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH: {
        DisableThreadLibraryCalls(hModule);

        g_logger = new ConsoleLogger();

        HANDLE hThread = CreateThread(NULL, 0, [](LPVOID) -> DWORD {
            RunRRIDDumper();
            return 0;
            }, NULL, 0, NULL);

        if (hThread) {
            CloseHandle(hThread);
        }
        break;
    }

    case DLL_PROCESS_DETACH: {
        if (g_logger) {
            delete g_logger;
            g_logger = nullptr;
        }
        break;
    }
    }
    return TRUE;
}