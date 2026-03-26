#define _CRT_SECURE_NO_WARNINGS
#include "dumper.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <algorithm>

std::function<void(const std::string&)> GameDumper::s_logCallback;

std::string GameDumper::GetCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    struct tm timeinfo;
    localtime_s(&timeinfo, &time_t);
    ss << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string GameDumper::SanitizeFilename(const std::string& name) {
    std::string result = name;
    std::replace(result.begin(), result.end(), '<', '_');
    std::replace(result.begin(), result.end(), '>', '_');
    std::replace(result.begin(), result.end(), ':', '_');
    std::replace(result.begin(), result.end(), '/', '_');
    std::replace(result.begin(), result.end(), '\\', '_');
    std::replace(result.begin(), result.end(), '*', '_');
    std::replace(result.begin(), result.end(), '?', '_');
    std::replace(result.begin(), result.end(), '"', '_');
    std::replace(result.begin(), result.end(), '|', '_');
    return result;
}

std::string GameDumper::SanitizeTypeName(const std::string& type) {
    std::string result = type;
    size_t last_dot = result.find_last_of('.');
    if (last_dot != std::string::npos) {
        result = result.substr(last_dot + 1);
    }
    std::replace(result.begin(), result.end(), '<', '_');
    std::replace(result.begin(), result.end(), '>', '_');
    std::replace(result.begin(), result.end(), ',', '_');
    std::replace(result.begin(), result.end(), ' ', '_');
    return result;
}

std::string GameDumper::GetCSharpTypeName(const std::string& il2cpp_type) {
    std::string type = il2cpp_type;

    if (type == "System.Boolean" || type == "bool") return "bool";
    if (type == "System.Byte" || type == "byte") return "byte";
    if (type == "System.SByte" || type == "sbyte") return "sbyte";
    if (type == "System.Char" || type == "char") return "char";
    if (type == "System.Decimal" || type == "decimal") return "decimal";
    if (type == "System.Double" || type == "double") return "double";
    if (type == "System.Single" || type == "float") return "float";
    if (type == "System.Int32" || type == "int") return "int";
    if (type == "System.UInt32" || type == "uint") return "uint";
    if (type == "System.Int64" || type == "long") return "long";
    if (type == "System.UInt64" || type == "ulong") return "ulong";
    if (type == "System.Int16" || type == "short") return "short";
    if (type == "System.UInt16" || type == "ushort") return "ushort";
    if (type == "System.String" || type == "string") return "string";
    if (type == "System.Object" || type == "object") return "object";
    if (type == "System.Void" || type == "void") return "void";

    if (type.find("[]") != std::string::npos) {
        std::string base_type = type.substr(0, type.find('['));
        return GetCSharpTypeName(base_type) + "[]";
    }

    return type;
}

std::string GameDumper::GetAccessLevelString(uint32_t flags, bool is_method) {
    uint32_t access = flags & RRID_METHOD_ACCESS_LEVEL_MASK;
    switch (access) {
    case RRID_METHOD_ACCESS_LEVEL_PRIVATE: return "private";
    case RRID_METHOD_ACCESS_LEVEL_PRIVATE_PROTECTED: return "private protected";
    case RRID_METHOD_ACCESS_LEVEL_INTERNAL: return "internal";
    case RRID_METHOD_ACCESS_LEVEL_PROTECTED: return "protected";
    case RRID_METHOD_ACCESS_LEVEL_PROTECTED_INTERNAL: return "protected internal";
    case RRID_METHOD_ACCESS_LEVEL_PUBLIC: return "public";
    default: return "private";
    }
}

std::string GameDumper::GetFieldModifiers(uint32_t flags) {
    std::string modifiers;
    if (flags & RRID_FIELD_ATTRIBUTE_STATIC) modifiers += "static ";
    if (flags & RRID_FIELD_ATTRIBUTE_READONLY) modifiers += "readonly ";
    if (flags & RRID_FIELD_ATTRIBUTE_CONST) modifiers += "const ";
    return modifiers;
}

std::string GameDumper::GetMethodModifiers(uint32_t flags) {
    std::string modifiers;
    if (flags & RRID_METHOD_ATTRIBUTE_STATIC) modifiers += "static ";
    if (flags & RRID_METHOD_ATTRIBUTE_FINAL) modifiers += "final ";
    if (flags & RRID_METHOD_ATTRIBUTE_VIRTUAL) modifiers += "virtual ";
    if (flags & RRID_METHOD_ATTRIBUTE_ABSTRACT) modifiers += "abstract ";
    return modifiers;
}

std::string GameDumper::GetCSharpAccessLevel(uint32_t flags, bool is_method) {
    uint32_t access = flags & RRID_METHOD_ACCESS_LEVEL_MASK;
    switch (access) {
    case RRID_METHOD_ACCESS_LEVEL_PRIVATE: return "private";
    case RRID_METHOD_ACCESS_LEVEL_PRIVATE_PROTECTED: return "private protected";
    case RRID_METHOD_ACCESS_LEVEL_INTERNAL: return "internal";
    case RRID_METHOD_ACCESS_LEVEL_PROTECTED: return "protected";
    case RRID_METHOD_ACCESS_LEVEL_PROTECTED_INTERNAL: return "protected internal";
    case RRID_METHOD_ACCESS_LEVEL_PUBLIC: return "public";
    default: return "private";
    }
}

std::string GameDumper::GetCSharpFieldModifiers(uint32_t flags) {
    std::string modifiers;
    if (flags & RRID_FIELD_ATTRIBUTE_STATIC) modifiers += "static ";
    if (flags & RRID_FIELD_ATTRIBUTE_READONLY) modifiers += "readonly ";
    if (flags & RRID_FIELD_ATTRIBUTE_CONST) modifiers += "const ";
    return modifiers;
}

std::string GameDumper::GetCSharpMethodModifiers(uint32_t flags) {
    std::string modifiers;
    if (flags & RRID_METHOD_ATTRIBUTE_STATIC) modifiers += "static ";
    if (flags & RRID_METHOD_ATTRIBUTE_FINAL) modifiers += "sealed ";
    if (flags & RRID_METHOD_ATTRIBUTE_VIRTUAL) modifiers += "virtual ";
    if (flags & RRID_METHOD_ATTRIBUTE_ABSTRACT) modifiers += "abstract ";
    if (flags & RRID_METHOD_ATTRIBUTE_NEW_SLOT) modifiers += "new ";
    return modifiers;
}

void GameDumper::CreateDirectoryIfNotExists(const std::string& path) {
    if (!std::filesystem::exists(path)) {
        std::filesystem::create_directories(path);
    }
}

void GameDumper::WriteCppOffsetsFile(const std::vector<RridImage*>& images, const std::string& output_dir) {
    std::string file_path = output_dir + "/GameOffsets.h";
    std::ofstream file(file_path);
    if (!file.is_open()) return;

    file << "// Auto-generated game offsets\n";
    file << "// Generated on: " << GetCurrentTimestamp() << "\n";
    file << "#pragma once\n\n";
    file << "#include <cstdint>\n\n";
    file << "namespace GameOffsets {\n\n";

    for (auto* image : images) {
        file << "    // Image: " << image->get_name() << "\n";
        file << "    // Module Base: 0x" << std::hex << image->get_module_base() << std::dec << "\n";
        file << "    // Image RVA: 0x" << std::hex << image->get_image_rva() << std::dec << "\n\n";

        auto classes = image->get_classes();
        for (auto* cls : classes) {
            std::string class_name = SanitizeFilename(cls->get_name());
            std::string ns = cls->get_namespace();

            if (!ns.empty()) {
                file << "    // " << ns << "." << class_name << "\n";
            }
            else {
                file << "    // " << class_name << "\n";
            }
            file << "    // Class RVA: 0x" << std::hex << cls->get_class_rva() << std::dec << "\n";

            auto fields = cls->get_fields();
            for (auto* field : fields) {
                if (field->has_rva()) {
                    file << "    // static " << field->get_name() << " -> RVA: 0x"
                        << std::hex << field->get_static_rva() << std::dec << "\n";
                }
                else if (field->get_offset() != 0) {
                    file << "    // " << field->get_name() << " -> Offset: 0x"
                        << std::hex << field->get_offset() << std::dec << "\n";
                }
            }

            auto methods = cls->get_methods();
            for (auto* method : methods) {
                file << "    // " << method->get_name() << "() -> RVA: 0x"
                    << std::hex << method->get_method_rva() << std::dec << "\n";
            }
            file << "\n";
        }
    }

    file << "} // namespace GameOffsets\n";
    file.close();
}

void GameDumper::WriteClassToCppDump(RridClass* cls, std::ofstream& file) {
    std::string class_name = SanitizeFilename(cls->get_name());
    std::string ns = cls->get_namespace();

    if (!ns.empty()) {
        file << "// Namespace: " << ns << "\n";
    }

    file << "class " << class_name << " {\n";
    file << "public:\n";

    auto fields = cls->get_fields();
    if (!fields.empty()) {
        file << "    // Fields\n";
        for (auto* field : fields) {
            std::string field_type = SanitizeTypeName(field->get_type());
            std::string modifiers = GetFieldModifiers(field->get_flags());
            std::string access = GetAccessLevelString(field->get_flags());

            file << "    " << access << " " << modifiers << field_type << " "
                << SanitizeFilename(field->get_name()) << ";";

            if (field->has_rva()) {
                file << " // RVA: 0x" << std::hex << field->get_static_rva() << std::dec;
            }
            else if (field->get_offset() != 0) {
                file << " // Offset: 0x" << std::hex << field->get_offset() << std::dec;
            }
            file << "\n";
        }
        file << "\n";
    }

    auto methods = cls->get_methods();
    if (!methods.empty()) {
        file << "    // Methods\n";
        for (auto* method : methods) {
            std::string return_type = SanitizeTypeName(method->get_return_type());
            std::string modifiers = GetMethodModifiers(method->get_flags());
            std::string access = GetAccessLevelString(method->get_flags(), true);

            file << "    " << access << " " << modifiers << return_type << " "
                << SanitizeFilename(method->get_name()) << "(";

            auto params = method->get_params();
            for (size_t i = 0; i < params.size(); i++) {
                if (i > 0) file << ", ";
                file << SanitizeTypeName(params[i].first) << " " << SanitizeFilename(params[i].second);
            }

            file << "); // RVA: 0x" << std::hex << method->get_method_rva() << std::dec << "\n";
        }
        file << "\n";
    }

    file << "};\n\n";
}

void GameDumper::WriteCppDumpFile(const std::vector<RridImage*>& images, const std::string& output_dir) {
    std::string file_path = output_dir + "/GameDump.cpp";
    std::ofstream file(file_path);
    if (!file.is_open()) return;

    file << "// Auto-generated game dump\n";
    file << "// Generated on: " << GetCurrentTimestamp() << "\n\n";
    file << "#include <cstdint>\n\n";

    for (auto* image : images) {
        file << "// ==================== IMAGE: " << image->get_name() << " ====================\n";
        file << "// Module Base: 0x" << std::hex << image->get_module_base() << std::dec << "\n";
        file << "// Image RVA: 0x" << std::hex << image->get_image_rva() << std::dec << "\n\n";

        auto classes = image->get_classes();
        for (auto* cls : classes) {
            WriteClassToCppDump(cls, file);
        }
    }

    file.close();
}

void GameDumper::WriteCSharpOffsetsFile(const std::vector<RridImage*>& images, const std::string& output_dir) {
    std::string file_path = output_dir + "/GameOffsets.cs";
    std::ofstream file(file_path);
    if (!file.is_open()) return;

    file << "// Auto-generated game offsets\n";
    file << "// Generated on: " << GetCurrentTimestamp() << "\n\n";
    file << "using System;\n\n";
    file << "public static class GameOffsets {\n\n";

    for (auto* image : images) {
        file << "    // Image: " << image->get_name() << "\n";
        file << "    public const ulong " << SanitizeFilename(image->get_name()) << "_Base = 0x"
            << std::hex << image->get_module_base() << "UL;\n";
        file << "    public const ulong " << SanitizeFilename(image->get_name()) << "_RVA = 0x"
            << std::hex << image->get_image_rva() << "UL;\n\n";

        auto classes = image->get_classes();
        for (auto* cls : classes) {
            std::string class_name = SanitizeFilename(cls->get_name());
            std::string ns = cls->get_namespace();
            std::string full_name = ns.empty() ? class_name : ns + "." + class_name;

            file << "    // " << full_name << "\n";
            file << "    public const ulong " << class_name << "_ClassRVA = 0x"
                << std::hex << cls->get_class_rva() << "UL;\n";

            auto fields = cls->get_fields();
            for (auto* field : fields) {
                std::string field_const = class_name + "_" + SanitizeFilename(field->get_name());
                if (field->has_rva()) {
                    file << "    public const ulong " << field_const << "_RVA = 0x"
                        << std::hex << field->get_static_rva() << "UL; // static\n";
                }
                else if (field->get_offset() != 0) {
                    file << "    public const ulong " << field_const << "_Offset = 0x"
                        << std::hex << field->get_offset() << "UL;\n";
                }
            }

            auto methods = cls->get_methods();
            for (auto* method : methods) {
                std::string method_const = class_name + "_" + SanitizeFilename(method->get_name());
                file << "    public const ulong " << method_const << "_RVA = 0x"
                    << std::hex << method->get_method_rva() << "UL;\n";
            }
            file << "\n";
        }
    }

    file << "}\n";
    file.close();
}

void GameDumper::WriteClassToCSharpDump(RridClass* cls, std::ofstream& file) {
    std::string class_name = SanitizeFilename(cls->get_name());
    std::string ns = cls->get_namespace();

    if (!ns.empty()) {
        file << "namespace " << ns << " {\n\n";
        file << "    ";
    }

    if (cls->is_enum()) {
        file << "public enum " << class_name << " {\n";
        auto fields = cls->get_fields();
        int enum_value = 0;
        for (auto* field : fields) {
            if (field->get_flags() & RRID_FIELD_ATTRIBUTE_STATIC) {
                file << "    " << SanitizeFilename(field->get_name()) << " = " << enum_value++ << ",\n";
            }
        }
        file << "}\n";
    }
    else {
        std::string class_type = cls->is_valuetype() ? "struct" : "class";
        file << "public " << class_type << " " << class_name << " {\n";

        auto fields = cls->get_fields();
        if (!fields.empty()) {
            file << "\n";
            for (auto* field : fields) {
                std::string field_type = GetCSharpTypeName(field->get_type());
                std::string modifiers = GetCSharpFieldModifiers(field->get_flags());
                std::string access = GetCSharpAccessLevel(field->get_flags());

                file << "    " << access << " " << modifiers << field_type << " "
                    << SanitizeFilename(field->get_name()) << ";\n";
            }
        }

        auto methods = cls->get_methods();
        if (!methods.empty()) {
            file << "\n";
            for (auto* method : methods) {
                std::string return_type = GetCSharpTypeName(method->get_return_type());
                std::string modifiers = GetCSharpMethodModifiers(method->get_flags());
                std::string access = GetCSharpAccessLevel(method->get_flags(), true);

                file << "    " << access << " " << modifiers << return_type << " "
                    << SanitizeFilename(method->get_name()) << "(";

                auto params = method->get_params();
                for (size_t i = 0; i < params.size(); i++) {
                    if (i > 0) file << ", ";
                    file << GetCSharpTypeName(params[i].first) << " " << SanitizeFilename(params[i].second);
                }

                file << ");\n";
            }
        }

        file << "}\n";
    }

    if (!ns.empty()) {
        file << "\n}\n";
    }

    file << "\n";
}

void GameDumper::WriteCSharpDumpFile(const std::vector<RridImage*>& images, const std::string& output_dir) {
    std::string file_path = output_dir + "/GameDump.cs";
    std::ofstream file(file_path);
    if (!file.is_open()) return;

    file << "// Auto-generated game dump\n";
    file << "// Generated on: " << GetCurrentTimestamp() << "\n\n";
    file << "using System;\n";
    file << "using System.Runtime.InteropServices;\n\n";

    for (auto* image : images) {
        file << "// ==================== IMAGE: " << image->get_name() << " ====================\n\n";

        auto classes = image->get_classes();
        for (auto* cls : classes) {
            WriteClassToCSharpDump(cls, file);
        }
    }

    file.close();
}

bool GameDumper::DumpAll(const std::string& output_dir, std::function<void(const std::string&)> logCallback) {
    s_logCallback = logCallback;

    auto log = [&](const std::string& msg) {
        if (s_logCallback) s_logCallback(msg);
        std::cout << msg << std::endl;
        };

    if (!rrid::init()) {
        log("[!] Failed to initialize RRID");
        return false;
    }
    log("[+] RRID initialized");

    CreateDirectoryIfNotExists(output_dir);

    auto images = rrid::get_images();
    log("[*] Found " + std::to_string(images.size()) + " images");
    WriteCppOffsetsFile(images, output_dir);
    log("[+] Written C++ offsets to: " + output_dir + "/GameOffsets.h");
    WriteCppDumpFile(images, output_dir);
    log("[+] Written C++ dump to: " + output_dir + "/GameDump.cpp");
    WriteCSharpOffsetsFile(images, output_dir);
    log("[+] Written C# offsets to: " + output_dir + "/GameOffsets.cs");
    WriteCSharpDumpFile(images, output_dir);
    log("[+] Written C# dump to: " + output_dir + "/GameDump.cs");
    log("[+] Dump completed successfully!");
    return true;
}