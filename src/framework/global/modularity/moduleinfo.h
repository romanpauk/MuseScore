/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MU_MODULARITY_MODULEINFO_H
#define MU_MODULARITY_MODULEINFO_H

#include <string_view>

#ifndef IOC_FUNC_SIG
#if defined(_MSC_VER)
    #define IOC_FUNC_SIG __FUNCSIG__
#else
    #define IOC_FUNC_SIG __PRETTY_FUNCTION__
#endif
#endif

#if defined(__GNUC__) && !defined(__clang__)
#if (__GNUC__ < 11)
#define IOC_NO_STRING_VIEW_CONSTEXPR_METHODS
#endif
#endif

namespace mu::modularity {
struct InterfaceInfo {
    std::string_view id;
    std::string_view module;
    bool internal = false;
    constexpr InterfaceInfo(std::string_view i, std::string_view m, bool intr)
        : id(i), module(m), internal(intr) {}
};

#ifndef IOC_NO_STRING_VIEW_CONSTEXPR_METHODS

constexpr std::string_view moduleNameBySig(const std::string_view& sig)
{
    constexpr std::string_view ArgBegin("(");
    constexpr std::string_view Space(" ");
    constexpr std::string_view Colon("::");

    //! NOTE Signature should be like
    //! SomeType mu::modulename::maybe::ClassName::methodName()

    std::size_t endMethod = sig.find_first_of(ArgBegin);
    if (endMethod == std::string_view::npos) {
        return sig;
    }

    std::size_t beginMethod = sig.find_last_of(Space, endMethod);
    if (beginMethod == std::string_view::npos) {
        return sig;
    }

    size_t beginModule = sig.find_first_of(Colon, beginMethod) + 2;
    size_t endModule = sig.find_first_of(Colon, beginModule);
    std::string_view module = sig.substr(beginModule, endModule - beginModule);
    return module;
}

constexpr std::string_view moduleNameByInterface(const std::string_view& sig)
{
    //! NOTE Signature should be like
    //! constexpr mu::modularity::InterfaceInfo mu::modularity::getInterfaceInfo() [with I = mu::update::IUpdateConfiguration]
    
    // TODO: 'module' concept is wide
    //  mu::ICryptographicHash (module should be ICryptographisHash)
    //  mu::audio::synth::ISynthResolver (module should be audio::synth)

    constexpr std::string_view InterfaceBegin("[with ");
    constexpr std::string_view Colon("::");

    std::size_t beginInterface = sig.find(InterfaceBegin);
    if (beginInterface == std::string_view::npos) {
        return sig;
    }

    std::size_t beginModule = sig.find(Colon, beginInterface);
    if (beginModule == std::string_view::npos) {
        return sig;
    }
    beginModule += 2;

    std::size_t endModule = sig.find(Colon, beginModule);
    if (endModule == std::string_view::npos) {
        std::string_view module = sig.substr(beginModule, sig.size() - beginModule - 1); // -1 is for the ']'
        return module;
    }
    
    std::string_view module = sig.substr(beginModule, endModule - beginModule);
    return module;
}

inline std::string_view interfaceName(const std::string_view& sig)
{
    //! NOTE Signature should be like
    //! constexpr mu::modularity::InterfaceInfo mu::modularity::getInterfaceInfo() [with I = mu::update::IUpdateConfiguration]

    constexpr std::string_view InterfaceEnd("]");
    constexpr std::string_view Colon("::");

    std::size_t endInterface = sig.find(InterfaceEnd);
    if (endInterface == std::string_view::npos) {
        return sig;
    }

    std::size_t beginInterface = sig.rfind(Colon, endInterface);
    if (beginInterface == std::string_view::npos) {
        return sig;
    }
    beginInterface += 2;

    std::string_view interface = sig.substr(beginInterface, endInterface - beginInterface);
    return interface;
}

#else
inline std::string_view moduleNameBySig(const std::string_view& sig)
{
    static const std::string_view ArgBegin("(");
    static const std::string_view Space(" ");
    static const std::string_view Colon("::");

    //! NOTE Signature should be like
    //! SomeType mu::modulename::maybe::ClassName::methodName()

    std::size_t endMethod = sig.find_first_of(ArgBegin);
    if (endMethod == std::string_view::npos) {
        return sig;
    }

    std::size_t beginMethod = sig.find_last_of(Space, endMethod);
    if (beginMethod == std::string_view::npos) {
        return sig;
    }

    size_t beginModule = sig.find_first_of(Colon, beginMethod) + 2;
    size_t endModule = sig.find_first_of(Colon, beginModule);
    std::string_view module = sig.substr(beginModule, endModule - beginModule);
    return module;
}

inline std::string_view moduleNameByInterface(const std::string_view& sig)
{
    //! NOTE Signature should be like
    //! constexpr mu::modularity::InterfaceInfo mu::modularity::getInterfaceInfo() [with I = mu::update::IUpdateConfiguration]
    
    // TODO: 'module' concept is wide
    //  mu::ICryptographicHash (module should be ICryptographisHash)
    //  mu::audio::synth::ISynthResolver (module should be audio::synth)

    static const std::string_view InterfaceBegin("[with ");
    static const std::string_view Colon("::");

    std::size_t beginInterface = sig.find(InterfaceBegin);
    if (beginInterface == std::string_view::npos) {
        return sig;
    }

    std::size_t beginModule = sig.find(Colon, beginInterface);
    if (beginModule == std::string_view::npos) {
        return sig;
    }
    beginModule += 2;

    std::size_t endModule = sig.find(Colon, beginModule);
    if (endModule == std::string_view::npos) {
        std::string_view module = sig.substr(beginModule, sig.size() - beginModule - 1); // -1 is for the ']'
        return module;
    }
    
    std::string_view module = sig.substr(beginModule, endModule - beginModule);
    return module;
}

inline std::string_view interfaceName(const std::string_view& sig)
{
    //! NOTE Signature should be like
    //! constexpr mu::modularity::InterfaceInfo mu::modularity::getInterfaceInfo() [with I = mu::update::IUpdateConfiguration]

    static const std::string_view InterfaceEnd("]");
    static const std::string_view Colon("::");

    std::size_t endInterface = sig.find(InterfaceEnd);
    if (endInterface == std::string_view::npos) {
        return sig;
    }

    std::size_t beginInterface = sig.rfind(Colon, endInterface);
    if (beginInterface == std::string_view::npos) {
        return sig;
    }
    beginInterface += 2;

    std::string_view interface = sig.substr(beginInterface, endInterface - beginInterface);
    return interface;
}
#endif
}

#endif // MU_MODULARITY_MODULEINFO_H
