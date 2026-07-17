
#pragma once

#include "../../_src/core/RunTimeData.hpp"

#include "utils/aliases.hpp"
#include <cstdlib>

namespace OrbitLog {

    namespace COLORS {
        constexpr const char* RESET  = "\033[0m";

        constexpr const char* BLACK  = "\033[30m";
        constexpr const char* RED    = "\033[31m";
        constexpr const char* GREEN  = "\033[32m";
        constexpr const char* YELLOW = "\033[33m";
        constexpr const char* BLUE   = "\033[34m";
        constexpr const char* MAGENTA= "\033[35m";
        constexpr const char* CYAN   = "\033[36m";
        constexpr const char* WHITE  = "\033[37m";
        constexpr const char* GRAY = "\033[90m";

        constexpr const char* BRIGHT_BLACK  = "\033[90m";
        constexpr const char* BRIGHT_RED    = "\033[91m";
        constexpr const char* BRIGHT_GREEN  = "\033[92m";
        constexpr const char* BRIGHT_YELLOW = "\033[93m";
        constexpr const char* BRIGHT_BLUE   = "\033[94m";
        constexpr const char* BRIGHT_MAGENTA= "\033[95m";
        constexpr const char* BRIGHT_CYAN   = "\033[96m";
        constexpr const char* BRIGHT_WHITE  = "\033[97m";    
    }

    namespace STYLES {

        // Reset
        constexpr const char* RESET = "\033[0m";

        // Styles
        constexpr const char* BOLD          = "\033[1m";
        constexpr const char* DIM           = "\033[2m";
        constexpr const char* ITALIC         = "\033[3m";
        constexpr const char* UNDERLINE      = "\033[4m";
        constexpr const char* BLINK          = "\033[5m";
        constexpr const char* RAPID_BLINK    = "\033[6m";
        constexpr const char* REVERSE        = "\033[7m";
        constexpr const char* HIDDEN         = "\033[8m";
        constexpr const char* STRIKETHROUGH  = "\033[9m";

        // Reset individual
        constexpr const char* RESET_BOLD         = "\033[22m";
        constexpr const char* RESET_ITALIC       = "\033[23m";
        constexpr const char* RESET_UNDERLINE    = "\033[24m";
        constexpr const char* RESET_BLINK        = "\033[25m";
        constexpr const char* RESET_REVERSE      = "\033[27m";
        constexpr const char* RESET_HIDDEN       = "\033[28m";
        constexpr const char* RESET_STRIKE       = "\033[29m";
    }

    inline void Error(string origin, string mess, bool finalize=false)
    {
        PrintLn(
            COLORS::RED,
            "[", 
            STYLES::UNDERLINE,
            "ERROR",
            STYLES::RESET,
            COLORS::RED,
            "] ",
            origin,
            ": ",
            STYLES::ITALIC,
            mess,
            STYLES::RESET,
            COLORS::RESET
        );
        if (finalize)
        {    
            Print(
                COLORS::GRAY,
                "|_ Program Exit Whit Exit Code '1'. .. ..."
            );
            exit(1);
        } 
    }

    inline void Warn(string origin, string mess)
    {
        PrintLn(
            COLORS::BRIGHT_YELLOW,
            "[", 
            STYLES::UNDERLINE,
            "WARN",
            STYLES::RESET,
            COLORS::BRIGHT_YELLOW,
            "] ",
            origin,
            ": ",
            STYLES::ITALIC,
            mess,
            STYLES::RESET,
            COLORS::RESET
        );
    }

    namespace SyntaxLog {
    
        enum class LogTypes
        {
            ERROR,
            WARN
        };
        struct LogObj {

            LogTypes type;
            string origin;
            string mess;
        }; inline vec<LogObj> Logs;

        inline void ThrowLog(RunTimeData& Data)
        {
            // ERROR
            for (LogObj obj : Logs)
            {
                if (obj.type is_not LogTypes::WARN) 
                    continue;
                if (Data.flags.debugMode)
                    {
                        PrintInLn(""); PrintInLn("");
                        Error(obj.origin, obj.mess, false); 
                    }
                else Error(obj.origin, obj.mess, true);
                
            }

            // WARNS
            for (LogObj obj : Logs)
            {
                if (obj.type is_not LogTypes::ERROR) 
                    continue;
                if (Data.flags.debugMode)
                {
                    PrintInLn(""); PrintInLn("");
                    Warn(obj.origin, obj.mess);
                }
            }
        }

        inline void SyntaxError(
            string origin, 
            string mess, 
            string why, 
            string sol,
            int line=-1,
            int index=-1
        )
        { 
            mess =+ 
            " | WHY: "
            +why
            +" | SOL: "
            +sol
            +" | SOL(line, index): "
            +std::to_string(line)+";"+std::to_string(index); 

            Logs.emplace_back(LogTypes::ERROR, origin, mess); 
        }

        inline void SyntaxWarn(
            string origin, 
            string mess, 
            string why, 
            string sol,
            int line=-1,
            int index=-1
        )
        { 
            mess =+ 
            " | WHY: "
            +why
            +" | SOL: "
            +sol
            +" | SOL(line, index): "
            +std::to_string(line)+";"+std::to_string(index); 

            Logs.emplace_back(LogTypes::WARN, origin, mess); 
        }
    }
}
