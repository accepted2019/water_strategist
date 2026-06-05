#pragma once

enum class Lang { EN, ZH };

extern Lang g_lang;

inline const char* L(const char* en, const char* zh) {
    return g_lang == Lang::ZH ? zh : en;
}
