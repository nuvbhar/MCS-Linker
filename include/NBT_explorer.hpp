#pragma once
#include <iostream>
#include <string>
#include <unordered_map>

// Force the master switch ON for the library
#define NBT_USE_ZLIB 
#include <nbt_cpp/NBT_All.hpp>

class NBTExplorer {
private:
    NBT_Type::Compound rootTag;

    // Helper: Convert standard string to C++20 u8string
    std::u8string ToU8(const std::string& str) {
        return std::u8string(str.begin(), str.end());
    }

    // Helper: Convert C++20 u8string back to standard string for terminal printing
    std::string FromU8(const std::u8string_view& u8str) {
        return std::string(u8str.begin(), u8str.end());
    }

public:
    // 1. Load the world
    bool LoadWorld(const std::string& filepath) {
        return NBT_Reader::SimpleReadNbtFile(filepath.c_str(), rootTag);
    }

    // 2. Bypass the dummy wrapper and grab the master "Data" folder (Now CONST)
    const std::unordered_map<NBT_Type::String, NBT_Node>* GetMasterDataMap() {
        auto& rawRoot = rootTag.GetData();
        if (rawRoot.empty()) return nullptr;

        auto it = rawRoot.begin();
        auto& innerMap = it->second.GetCompound().GetData();
        
        NBT_Type::String dataKey(ToU8("Data"));
        if (innerMap.count(dataKey)) {
            return &innerMap.at(dataKey).GetCompound().GetData();
        }
        return nullptr;
    }

    // 3. Print everything inside a specific folder (Now accepts CONST)
    void PrintFolderContents(const std::unordered_map<NBT_Type::String, NBT_Node>* currentMap) {
        if (!currentMap) return;
        
        std::cout << "\n--- Folder Contents (" << currentMap->size() << " items) ---\n";
        for (auto& pair : *currentMap) {
            std::cout << "- " << reinterpret_cast<const char*>(pair.first.c_str()) << "\n";
        }
        std::cout << "----------------------------------\n";
    }

    // 4. Jump into a specific sub-folder safely (Now returns and accepts CONST)
    const std::unordered_map<NBT_Type::String, NBT_Node>* EnterSubFolder(const std::unordered_map<NBT_Type::String, NBT_Node>* currentMap, const std::string& folderName) {
        NBT_Type::String searchKey(ToU8(folderName));
        
        if (currentMap->count(searchKey)) {
            try {
                // Try to crack it open as a compound (folder)
                return &currentMap->at(searchKey).GetCompound().GetData();
            } catch (...) {
                std::cout << "[!] Error: '" << folderName << "' is a raw value, not a folder!\n";
                return nullptr;
            }
        }
        std::cout << "[!] Error: Folder '" << folderName << "' does not exist here.\n";
        return nullptr;
    }
    // 5. Read a raw value (Numbers, Text, etc.)
    void ReadRawValue(const std::unordered_map<NBT_Type::String, NBT_Node>* currentMap, const std::string& valueName) {
        NBT_Type::String searchKey(ToU8(valueName));
        
        if (currentMap->count(searchKey)) {
            auto& node = currentMap->at(searchKey);
            std::cout << ">> " << valueName << " = ";
            
            // The Try-Catch Ladder: Test every common data type until one succeeds!
            try { std::cout << node.GetLong() << " (Long)\n"; return; } catch (...) {}
            try { std::cout << node.GetInt() << " (Integer)\n"; return; } catch (...) {}
            try { std::cout << node.GetFloat() << " (Float)\n"; return; } catch (...) {}
            try { std::cout << node.GetDouble() << " (Double)\n"; return; } catch (...) {}
            try { std::cout << (int)node.GetByte() << " (Byte/Bool)\n"; return; } catch (...) {}
            
            std::cout << "[Unable to print: Might be text or an array]\n";
        } else {
            std::cout << "[!] Error: Value '" << valueName << "' does not exist here.\n";
        }
    }
};