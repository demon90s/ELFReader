# ELFReader
ELFReader is a tool for reading elf files, it only supports x86_64 architecture.

This tool is built for my own study and test, that is, not perfect at all. You are recommaned to test this tool on CentOS7. Just run like:

```
make
./elfreader -S /bin/ps
```

Then you will see the section header info of /bin/ps .

There is a more useful tool named [ELFIO](https://github.com/serge1/ELFIO) which you can use to read more info of elf file in your program.

Still, this tool can be use to read some useful information, such as symbol table, relocation table, which I use for my another program. All you need to do is copy ELFReader.h and ELFReader.cpp to your source code.
