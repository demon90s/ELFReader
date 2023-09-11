#include "ELFPrinter.h"
#include "ELFReader.h"
#include "formattedtable.hpp"

static std::string DecToHex(long decimal)
{
	char buffer[1024] {};
	snprintf(buffer, sizeof(buffer), "0x%lx", decimal);
	return buffer;
}

void ELFPrinter::PrintAll() const
{
    std::cout << "ELF class: " << m_elf_reader->GetELFClass() << "\n";
    std::cout << "ELF type: " << m_elf_reader->GetELFType() << "\n";

    this->PrintSections();
    this->PrintSymbols();
    this->PrintRelocations();
    this->PrintDynamics();
}

void ELFPrinter::PrintSections() const
{
    std::cout << this->GetSectionsString() << std::endl;
}

void ELFPrinter::PrintSymbols() const
{
    std::cout << this->GetSymbolString() << std::endl;
}

void ELFPrinter::PrintRelocations() const
{
    std::cout << this->GetRelocationString() << std::endl;
}

void ELFPrinter::PrintDynamics() const
{
    std::cout << this->GetDynamicString() << std::endl;
}

std::string ELFPrinter::GetSectionsString() const
{
    std::ostringstream oss;

    const std::vector<ELFReader::Section> &sections = m_elf_reader->GetSections();

    oss << "ELF section num: " << sections.size() << "\n";
	{
    	oss << "sections:\n";
		FormattedTable ftable;
		ftable.SetFieldList({ "Number", "Type", "Name", "Flags", "Virtual Address", "File Offset", "Section Size", "Entry Size" });
		for (const ELFReader::Section &section : sections)
		{
            std::string flags;
            if (section.section_header.sh_flags & SHF_WRITE) flags += "SHF_WRITE ";
            if (section.section_header.sh_flags & SHF_ALLOC) flags += "SHF_ALLOC ";
            if (section.section_header.sh_flags & SHF_EXECINSTR) flags += "SHF_EXECINSTR ";

			ftable.AddRow(section.number, section.section_header.sh_type, section.get_name(*m_elf_reader), flags, DecToHex(section.section_header.sh_addr), section.section_header.sh_offset, section.section_header.sh_size, section.section_header.sh_entsize);
		}
		oss << ftable.GetFormattedTable() << "\n";
	}
	oss << "\n";

    return oss.str();
}

std::string ELFPrinter::GetSymbolString() const
{
    std::ostringstream oss;

    auto build = [&oss, this](const std::string &symtable_name, const std::vector<ELFReader::Symbol> &symbols, bool is_dyn)
    {
        oss << symtable_name << " num: " << symbols.size() << "\n";
        {
            FormattedTable ftable;
            ftable.SetFieldList({ "index", "Type", "Bind", "Section", "Name", "Value", "Size" });
            int index = 0;
            for (const ELFReader::Symbol &symbol_item : symbols)
            {
                std::string sym_name;
                if (is_dyn)
                {
                    sym_name = symbol_item.get_dynsym_name(*m_elf_reader);
                }
                else
                {
                    sym_name = symbol_item.get_sym_name(*m_elf_reader);
                }
                if (sym_name.length() > 30)
                {
                    sym_name = sym_name.substr(0, 30);
                }

                ftable.AddRow(index, symbol_item.get_sym_type_desc(), symbol_item.get_sym_bind_desc(), symbol_item.get_sym_section_desc(*m_elf_reader), sym_name, DecToHex(symbol_item.sym.st_value), symbol_item.sym.st_size);
                index++;
            }
            oss << "symbols:\n" << ftable.GetFormattedTable() << "\n";
        }
        oss << "\n";
    };

    build(".symtab", m_elf_reader->GetSymbols(), false);
    build(".dynsym", m_elf_reader->GetDynSyms(), true);

    return oss.str();
}

std::string ELFPrinter::GetRelocationString() const
{
    std::ostringstream oss;

    const std::map<std::string, std::vector<ELFReader::Relocation>> &relocations = m_elf_reader->GetRelocations();

    for (const auto &rel_section : relocations)
	{
		const std::string &section_name = rel_section.first;
		const auto &relocations = rel_section.second;

		oss << section_name << " relocation num: " << relocations.size() << "\n";
		{
			FormattedTable ftable;
			ftable.SetFieldList({ "Offset", "Type", "Symbol Name", "Type", "Bind", "Section" });
			for (const ELFReader::Relocation &rel_item : relocations)
			{
                const ELFReader::Symbol *symbol = nullptr;
                std::string symbol_name;

                if (section_name == ".rela.dyn" || section_name == ".rela.plt")
                {
                    symbol_name = rel_item.get_dynsym_name(*m_elf_reader);

                    if (rel_item.symbol_index >= 0 && rel_item.symbol_index < static_cast<int>(m_elf_reader->GetDynSyms().size()))
                    {
                        symbol = &m_elf_reader->GetDynSyms().at(rel_item.symbol_index);
                    }
                }
                else if (section_name == ".rela.text")
                {
                    symbol_name = rel_item.get_symbol_name(*m_elf_reader);

                    if (rel_item.symbol_index >= 0 && rel_item.symbol_index < static_cast<int>(m_elf_reader->GetSymbols().size()))
                    {
                        symbol = &m_elf_reader->GetSymbols().at(rel_item.symbol_index);
                    }
                }

                if (nullptr != symbol)
                {
                    ftable.AddRow(DecToHex(rel_item.rel.r_offset), rel_item.get_type_desc(), symbol_name,
                        symbol->get_sym_type_desc(), symbol->get_sym_bind_desc(), symbol->get_sym_section_desc(*m_elf_reader));
                }
                
			}
			oss << "relocations:\n" << ftable.GetFormattedTable() << "\n";
		}

		oss << "\n";
	}

    return oss.str();
}

std::string ELFPrinter::GetDynamicString() const
{
    std::ostringstream oss;

    const std::vector<ELFReader::Dynamic> &dynamics = m_elf_reader->GetDynamics();

    oss << "dynamic num: " << dynamics.size() << "\n";
    FormattedTable ftable;
    ftable.SetFieldList({ "Tag", "Value" });
    for (const auto &dynamic_item : dynamics)
	{
		std::string tag = "";
        std::string value = "";

        const Elf64_Dyn &dyn = dynamic_item.dyn;

        switch (dyn.d_tag)
        {
        case DT_SYMTAB:
            tag = "DT_SYMTAB";
            value = DecToHex(dyn.d_un.d_ptr);
            break;

        case DT_STRTAB:
            tag = "DT_STRTAB";
            value = DecToHex(dyn.d_un.d_ptr);
            break;

        case DT_STRSZ:
            tag = "DT_STRSZ";
            value = std::to_string(dyn.d_un.d_val);
            break;

        case DT_HASH:
            tag = "DT_HASH";
            value = DecToHex(dyn.d_un.d_ptr);
            break;

        case DT_SONAME:
            tag = "DT_SONAME";
            value = &m_elf_reader->GetDynamicStrs().at(dyn.d_un.d_val);
            break;

        case DT_RPATH:
            tag = "DT_RPATH";
            value = &m_elf_reader->GetDynamicStrs().at(dyn.d_un.d_val);
            break;

        case DT_INIT:
            tag = "DT_INIT";
            value = DecToHex(dyn.d_un.d_ptr);
            break;

        case DT_FINI:
            tag = "DT_FINI";
            value = DecToHex(dyn.d_un.d_ptr);
            break;

        case DT_NEEDED:
            tag = "DT_NEEDED";
            value = &m_elf_reader->GetDynamicStrs().at(dyn.d_un.d_val);
            break;

        case DT_REL:
            tag = "DT_REL";
            value = DecToHex(dyn.d_un.d_ptr);
            break;

        case DT_RELA:
            tag = "DT_RELA";
            value = DecToHex(dyn.d_un.d_ptr);
            break;
        
        case DT_RELENT:
            tag = "DT_RELENT";
            value = std::to_string(dyn.d_un.d_val);
            break;

        case DT_RELAENT:
            tag = "DT_RELAENT";
            value = std::to_string(dyn.d_un.d_val);
            break;

        case DT_PLTGOT:
            tag = "DT_PLTGOT";
            value = DecToHex(dyn.d_un.d_ptr);
            break;

        default:
            tag = std::to_string(dyn.d_tag);
            value = std::to_string(dyn.d_un.d_ptr);
            break;
        }
		
        ftable.AddRow(tag, value);
	}

    oss << "dynamics:\n" << ftable.GetFormattedTable() << "\n";

    oss << "\n";

    return oss.str();
}
