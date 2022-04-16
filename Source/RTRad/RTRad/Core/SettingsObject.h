#pragma once

namespace RR {
    struct BaseSettings {

    };

    template <typename T>
    class SettingsObject
    {
        static_assert(std::is_base_of<BaseSettings, T>::value, "T must inherit from BaseSettings");

    public:
        T settings;
    };

    template <typename T>
    struct SettingsStruct
    {
        static_assert(std::is_base_of<BaseSettings, T>::value, "T must inherit from BaseSettings");

    public:
        T settings;
    };
}

