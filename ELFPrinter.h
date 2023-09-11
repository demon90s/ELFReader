#pragma once

#include <string>

class ELFReader;

class ELFPrinter
{
public:
    ELFPrinter(ELFReader *elf_reader) : m_elf_reader(elf_reader)
    {

    }

    void PrintAll() const;

    void PrintSections() const;
    void PrintSymbols() const;
    void PrintRelocations() const;
    void PrintDynamics() const;

private:
    std::string GetSectionsString() const;
    std::string GetSymbolString() const;
    std::string GetRelocationString() const;
    std::string GetDynamicString() const;

    ELFReader *m_elf_reader;
};
