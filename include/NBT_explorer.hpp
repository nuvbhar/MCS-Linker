#pragma once

#include <functional>
#include <iostream>
#include <optional>
#include <ranges>
#include <string>
#include <unordered_map>

// Force the master switch ON for the library
#define NBT_USE_ZLIB
#include <nbt_cpp/NBT_All.hpp>

using CompoundMap = std::unordered_map<NBT_Type::String, NBT_Node>;
using OptionalCompound = std::optional<std::reference_wrapper<const CompoundMap>>;

class NBTExplorer {
private:
    NBT_Type::Compound rootTag;

    static std::u8string ToU8(const std::string& str) {
        return std::u8string{str.begin(), str.end()};
    }

    static std::string FromU8(const std::u8string_view& u8str) {
        return std::string{u8str.begin(), u8str.end()};
    }

public:
    // 1. Load the world
    bool LoadWorld(const std::string& filepath) {
        return NBT_Reader::SimpleReadNbtFile(filepath.c_str(), rootTag);
    }

    // 2. Bypass the dummy wrapper and grab the master "Data" folder (Now CONST)
    OptionalCompound GetMasterDataMap() const {
        auto& rawRoot = rootTag.GetData();
        if (rawRoot.empty()) return std::nullopt;

        const auto& wrapperNode = rawRoot.begin()->second;
        auto& innerMap = wrapperNode.GetCompound().GetData();

        if (const NBT_Type::String dataKey(ToU8("Data")); innerMap.contains(dataKey)) {
            return std::cref(innerMap.at(dataKey).GetCompound().GetData());
        }
        return std::nullopt;
    }

    // 3. Print everything inside a specific folder (Now accepts CONST)
    static void PrintFolderContents(const OptionalCompound& currentMap) {
        if (!currentMap) return;

        const auto& map = currentMap->get();
        std::cout << "\n--- Folder Contents (" << map.size() << " items) ---\n";
        for (const auto& key : map | std::views::keys) {
            std::cout << "- " << key.c_str() << "\n";
        }
        std::cout << "----------------------------------\n";
    }

    // 4. Jump into a specific sub-folder safely (Now returns and accepts CONST)
    static OptionalCompound EnterSubFolder(const OptionalCompound& currentMap, const std::string& folderName) {
        if (!currentMap) return std::nullopt;

        const auto& map = currentMap->get();
        const NBT_Type::String searchKey(ToU8(folderName));
        const auto it = map.find(searchKey);

        if (it == map.end()) {
            std::cout << "[!] Error: Folder '" << folderName << "' does not exist here.\n";
            return std::nullopt;
        }
        try {
            return std::cref(it->second.GetCompound().GetData());
        } catch (...) {
            std::cout << "[!] Error: '" << folderName << "' is a raw value, not a folder!\n";
            return std::nullopt;
        }
    }

    // 5. Read a raw value (Numbers, Text, etc.)
    static std::string ReadRawValue(const OptionalCompound& currentMap, const std::string& valueName) {
        if (!currentMap) return "{error}";

        const auto& map = currentMap->get();
        const NBT_Type::String searchKey(ToU8(valueName));
        const auto it = map.find(searchKey);

        if (it == map.end()) {
            std::cout << "[!] Error: Value '" << valueName << "' does not exist here.\n";
            return "{error}";
        }

        auto& node = it->second;

        switch (node.GetTag()) {
        case NBT_TAG::Byte:
            return std::to_string(static_cast<int>(node.GetByte()));
        case NBT_TAG::Short:
            return std::to_string(node.GetShort());
        case NBT_TAG::Int:
            return std::to_string(node.GetInt());
        case NBT_TAG::Long:
            return std::to_string(node.GetLong());
        case NBT_TAG::Float:
            return std::to_string(node.GetFloat());
        case NBT_TAG::Double:
            return std::to_string(node.GetDouble());
        case NBT_TAG::String:
            return node.GetString().ToCharTypeUTF8();
        default:
            return "[Unable to print: compound or array type]";
        }
    }

    static std::string GetValueType(const OptionalCompound& currentMap, const std::string& valueName) {
        if (!currentMap) return "{error}";

        const auto& map = currentMap->get();
        const NBT_Type::String searchKey(ToU8(valueName));
        const auto it = map.find(searchKey);

        if (it == map.end()) {
            std::cout << "[!] Error: Value '" << valueName << "' does not exist here.\n";
            return "{error}";
        }

        auto& node = it->second;

        switch (node.GetTag()) {
        case NBT_TAG::Byte:
            return "(Byte/Bool)";
        case NBT_TAG::Short:
            return "(Short)";
        case NBT_TAG::Int:
            return "(Integer)";
        case NBT_TAG::Long:
            return "(Long)";
        case NBT_TAG::Float:
            return "(Float)";
        case NBT_TAG::Double:
            return "(Double)";
        case NBT_TAG::String:
            return "(String)";
        default:
            return "[Unable to print: compound or array type]";
        }
    }
};