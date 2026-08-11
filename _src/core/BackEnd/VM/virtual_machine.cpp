
// =========== VM ========== //
// Virtual Machine ORBIT Runner | CPU-Virtual Para Rodar Codigo ORBIT.
// Developed By SpyK3(2026) | License: GitHub(MIT). 
// INCLUDE HEADERS 'N DEPENDENCES

#include "virtual_machine.hpp" // HEADER FILE | CABEÇALHO
#include "../byte_code.hpp"

#include "../../FrontEnd/SA/semantic_analysis.hpp"
#include "../../FrontEnd/parser/AST/AST.hpp"

#include "../../FrontEnd/lexer/lexer.hpp"

#include "utils/aliases.hpp"
#include "tools/console.hpp"
#include "../../RunTimeData.hpp"
#include <string>
#include <variant>

