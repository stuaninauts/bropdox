#ifndef FILE_DISPLAY_FORMATTER_HPP
#define FILE_DISPLAY_FORMATTER_HPP

#include <string>
#include <filesystem>
#include <sys/stat.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <ctime>

namespace fs = std::filesystem;

class FileDisplayFormatter {
public:
    // Métodos estáticos para formatação e exibição de informações de arquivos
    
    // Imprime o cabeçalho da tabela para listagem de arquivos
    static void print_table_header(std::ostream& out = std::cout);
    
    // Formata um timestamp para exibição legível
    static std::string format_time(std::time_t raw_time);
    
    // Verifica e obtém estatísticas do arquivo
    static bool get_file_stat(const fs::path& path, struct stat& st);
    
    // Imprime informações sobre um arquivo específico
    static void print_file_info(const fs::directory_entry& entry, std::ostream& out = std::cout);
    
    // Retorna uma string com as informações do arquivo formatadas
    static std::string get_file_info_string(const fs::directory_entry& entry);
    
    // Retorna uma string com o cabeçalho da tabela
    static std::string get_table_header_string();
    
    // Constantes para formatação
    static const int FILE_COLUMN_WIDTH = 30;
    static const int TIME_COLUMN_WIDTH = 30;
};

#endif // FILE_DISPLAY_FORMATTER_HPP