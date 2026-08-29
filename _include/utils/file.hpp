
// ========== FILE-MANAGER ========== //
// Project File Manager | Gerenciador de Arquivos do Projeto
// Developed By: SpyK3(2026) | License: GitHub(MIT).

#pragma once
#include "utils/aliases.hpp"

#include <filesystem>
namespace fs=std::filesystem;

inline fs::path GetOrbitOrigin(char** argv)
{ return fs::absolute(argv[0]).parent_path().parent_path(); }