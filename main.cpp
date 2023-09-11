#include <iostream>
#include <cstdio>

#include "ELFReader.h"
#include "ELFPrinter.h"

static bool read_elf(const char *elf_file, ELFReader &elf_reader)
{
    if (elf_file == nullptr)
    {
        std::cerr << "read_elf: can not parse nullptr to elf_file" << std::endl;
        return false;
    }

    FILE *fp = fopen(elf_file, "r");
    if (fp == nullptr)
    {
        perror("read_elf fopen");
        return false;
    }

    if (!elf_reader.ReadELFFile(fp))
    {
        fclose(fp);
        return false;
    }

    fclose(fp);
    
    return true;
}

static void print_help(char *argv[])
{
    std::cerr << "usage: " << argv[0] << " <option> <elf_file>" << std::endl;
    std::cerr << "options:" << std::endl;
    std::cerr << "\t-a : all info" << std::endl;
    std::cerr << "\t-S : section info" << std::endl;
    std::cerr << "\t-s : symbol info" << std::endl;
    std::cerr << "\t-r : relocation info" << std::endl;
    std::cerr << "\t-d : dynamic info" << std::endl;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        print_help(argv);
        exit(-1);
    }
    
    const char *elf_file = argv[2];

    ELFReader elf_reader;
    if (!read_elf(elf_file, elf_reader))
    {
        exit(-1);
    }

    ELFPrinter elf_printer(&elf_reader);

    std::string opt = argv[1];
    if (opt == "-a")
    {
        elf_printer.PrintAll();
    }
    else if (opt == "-S")
    {
        elf_printer.PrintSections();
    }
    else if (opt == "-s")
    {
        elf_printer.PrintSymbols();
    }
    else if (opt == "-r")
    {
        elf_printer.PrintRelocations();
    }
    else if (opt == "-d")
    {
        elf_printer.PrintDynamics();
    }
    else
    {
        print_help(argv);
    }

    return 0;
}
