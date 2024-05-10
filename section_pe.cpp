#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cctype>
#include <windows.h>
#include <cstdint>
#include <numeric>

std::vector<uint8_t> patch_long_name(uint32_t smptroffs, std::ifstream& file, std::vector<uint8_t> name_bytes) {
    auto index = std::find(name_bytes.begin(), name_bytes.end(), '/');
    if (index != name_bytes.end()) {
        auto offset_str = std::string(index + 1, name_bytes.end());
        auto cleaned_insn = std::accumulate(offset_str.begin(), offset_str.end(), std::string(), [](std::string acc, char c) {
            if (std::isdigit(c) || c == '-')
                acc.push_back(c);
            return acc;
        });
        uint64_t offset = std::stoull(cleaned_insn);
        std::vector<uint8_t> table_string(256, 0);
        file.seekg(offset + smptroffs);
        file.read(reinterpret_cast<char*>(table_string.data()), 256);
        auto name_length = std::find(table_string.begin(), table_string.end(), 0) - table_string.begin();
        std::vector<uint8_t> result(name_bytes.begin(), index);
        result.insert(result.end(), table_string.begin(), table_string.begin() + name_length);
        return result;
    } else {
        return name_bytes;
    }
}

int getSection(std::ifstream &file, uint16_t section_num, uint32_t symbol_table_offs) {
    std::vector<IMAGE_SECTION_HEADER> sections;
    for (int i = 0; i < section_num; ++i) {
        IMAGE_SECTION_HEADER section;
        file.read(reinterpret_cast<char*>(&section), sizeof(IMAGE_SECTION_HEADER));
        sections.push_back(section);
    }
    std::cout << "\n----------------------------------------------------------------------" << std::endl;
    std::cout << "SECTION TABLE:\n" << std::endl;
    for (const auto& sec : sections) {
        std::vector<uint8_t> name_og(sec.Name, sec.Name + sizeof(sec.Name));
        std::vector<uint8_t> name_bytes = patch_long_name(symbol_table_offs, file, name_og);
        std::cout << " Name: ";
        for (const auto& byte : name_bytes) {
            std::cout << static_cast<char>(byte);
        }
        std::cout << std::endl;
        std::cout << "    VSize: 0x" << std::hex << sec.Misc.VirtualSize << " - Va: 0x" << sec.VirtualAddress << std::endl;
        std::cout << "    SizeOfRawData: 0x" << sec.SizeOfRawData << " - PtrToRawData: 0x" << sec.PointerToRawData << std::endl;
        std::cout << "    PtrToReloc: 0x" << sec.PointerToRelocations << " - PtrToLinenum: 0x" << sec.PointerToLinenumbers << std::endl;
        std::cout << "    NumberOfReloc: " << sec.NumberOfRelocations << " - NumberOfLinenum: " << sec.NumberOfLinenumbers<< std::endl;
        std::cout << "    Characteristics: " << std::hex << sec.Characteristics << std::endl;
        std::cout << "\n-----------------------------------" << std::endl;
    }
    std::cout << "\n";
    return 0;
}
