
// ============ ALIASES FOR CODE HELP UTIL ========== //
// Global Aliases | Aliases Globais
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// PRAGMATIC INFOS | INFORMAÇOES PRAGMATICAS
#pragma once

// INCLUDE HEADERS 'N DEPENDENCES
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <unordered_map>
#include <variant>
#include <cstdint>
#include <memory>

#define is ==
#define is_not !=

// ENUMS

enum ORBIT_ERRORS_CODE
{
    MEMORY_ERROR = 1001,
    LOG_ERROR = 795  
};

// BASICS
using string=std::string;
using str_view=std::string_view;
using mono_state=std::monostate;
using runt_err=std::runtime_error;
using byte=std::byte;

using i64=std::int64_t;
using i32=std::int32_t;
using i16=std::int16_t;
using i8=std::int8_t;

using ui64=std::uint64_t;
using ui32=std::uint32_t;
using ui16=std::uint16_t;
using ui8=std::uint8_t;

// TEMPLATES
template<typename T>
using vec=std::vector<T>;
template<typename Key, typename Value>
using unord_map= std::unordered_map<Key, Value>;
template<typename T>
using uniq_ptr=std::unique_ptr<T>;
template<typename T1, typename T2>
using pair=std::pair<T1, T2>;
template<typename... T>
using variant=std::variant<T...>;

// HELPERS

// Print raw output | Imprime saída simples
template<typename... Args>
void Print(Args&&... args)
{
    (std::cout << ... << args);
    std::cout.flush();
}

// Print output with newline | Imprime saída com quebra de linha
template<typename... Args>
void PrintLn(Args&&... args)
{
    (std::cout << ... << args) << '\n';
    std::cout.flush();
}

// Clear current line and print new content | Limpa a linha atual e imprime novo conteúdo
template<typename... Args>
void PrintIn(Args&&... args)
{
    std::cout << "\r\033[2K";
    (std::cout << ... << args);
    std::cout.flush();
}

// Clear current line and print new content with newline | Limpa a linha atual e imprime novo conteúdo com quebra de linha
template<typename... Args>
void PrintInLn(Args&&... args)
{
    std::cout << '\r';
    std::cout << "\033[2K";
    (std::cout << ... << args) << '\n';
    std::cout.flush();
}

// Clear previous line and print new content | Limpa a linha anterior e imprime novo conteúdo
template<typename... Args>
void PrintOut(Args&&... args)
{
    std::cout << "\033[1A";
    std::cout << "\033[2K";
    (std::cout << ... << args);
    std::cout.flush();
}

// Clear previous line and print new content with newline | Limpa a linha anterior e imprime novo conteúdo com quebra de linha
template<typename... Args>
void PrintOutLn(Args&&... args)
{
    std::cout << "\033[1A";
    std::cout << "\033[2K";
    (std::cout << ... << args) << '\n';
    std::cout.flush();
}

template<typename T, typename Variant>
inline bool holds_alt(const Variant& variant)
{
    return std::holds_alternative<T>(variant);
}

template<typename T, typename Variant, typename U>
inline bool holds_alt_value(const Variant& variant, const U& value)
{
    if (const auto* ptr = std::get_if<T>(&variant))
        return *ptr == value;

    return false;
}

template<typename Container, typename T>
constexpr bool contains_at(const Container& container, const T& value)
{
    return std::find(container.begin(), container.end(), value) != container.end();
}

template<typename T, typename... Args>
constexpr auto make_uniq(Args&&... args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}

// EOF