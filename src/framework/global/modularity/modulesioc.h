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

#ifndef MU_MODULARITY_MODULESIOC_H
#define MU_MODULARITY_MODULESIOC_H

#include <memory>
#include <map>
#include <string>
#include <cassert>
#include <iostream>

#include "imoduleinterface.h"

namespace mu::modularity {

template< typename I > constexpr InterfaceInfo getInterfaceInfo() { 
    // This function does not reqire declaration of I to be available, as it does not access anything
    // inside I namespace. This has the following consequences:
    //  1) INTERFACE_ID() macro can be removed.
    //  2) INJECT() needs only forward declarations, thus headers with the interfaces to be injected
    //      can be removed
    return InterfaceInfo(interfaceName(__PRETTY_FUNCTION__), moduleNameByInterface(__PRETTY_FUNCTION__), false);
}

class ModulesIoC
{
public:

    static ModulesIoC* instance();

    // Register Export
    template<class I>
    void registerExportCreator(const std::string& module, IModuleCreator* c)
    {
        if (!c) {
            assert(c);
            return;
        }
        registerService(module, getInterfaceInfo<I>(), std::shared_ptr<void>(), c);
    }

    template<class I>
    void registerExport(const std::string& module, I* p)
    {
        if (!p) {
            assert(p);
            return;
        }
        registerExport<I>(module, std::shared_ptr<I>(p));
    }

    template<class I>
    void registerExportNoDelete(const std::string& module, I* p)
    {
        if (!p) {
            assert(p);
            return;
        }        
        registerExport<I>(module, std::shared_ptr<I>(p, [](I*) {}));
    }

    template<class I>
    void registerExport(const std::string& module, std::shared_ptr<I> p)
    {
        if (!p) {
            assert(p);
            return;
        }
        registerService(module, getInterfaceInfo<I>(), p, nullptr);
    }

    // Register Internal
    template<class I>
    void registerInternalCreator(const std::string& module, IModuleCreator* c)
    {
        if (!c) {
            assert(c);
            return;
        }
        registerService(module, getInterfaceInfo<I>(), std::shared_ptr<void>(), c);
    }

    template<class I>
    void registerInternal(const std::string& module, I* p)
    {
        if (!p) {
            assert(p);
            return;
        }
        registerInternal<I>(module, std::shared_ptr<I>(p));
    }

    template<class I>
    void registerInternalNoDelete(const std::string& module, I* p)
    {
        if (!p) {
            assert(p);
            return;
        }
        registerInternal<I>(module, std::shared_ptr<I>(p, [](I*) {}));
    }

    template<class I>
    void registerInternal(const std::string& module, std::shared_ptr<I> p)
    {
        if (!p) {
            assert(p);
            return;
        }
        registerService(module, getInterfaceInfo<I>(), p, nullptr);
    }

    // Unregister
    template<class I>
    void unregister(const std::string& /*module*/)
    {
        unregisterService(getInterfaceInfo<I>());
    }

    template<class I>
    void unregisterIfRegistered(const std::string& module, std::shared_ptr<I> p)
    {
        if (resolve<I>(module, std::string_view()) == p) {
            unregister<I>(module);
        }
    }

    // Resolve
    template<class I>
    std::shared_ptr<I> resolve(const std::string_view& module, const std::string_view& callInfo = std::string_view())
    {
        std::shared_ptr<void> p = doResolvePtrByInfo(module, getInterfaceInfo<I>(), callInfo);
        // TODO: both static/dynamic cast does not work with forward-declared types.
        // But the check done here is in fact done when the type is registered, as it can't happen
        // that for type I, an pointer to unrelated type J would be registered.
        // The key is that all the registration methods need to work with the equivalent
        //      getInterfaceInfo<I> and std::shared_ptr<I>. It is also important to avoid any casts 
        // during the registration as that could backfire with multiple inheritance. If those rules
        // are followed (can be enforced by design / tests), than the reinterpret_cast<> done here
        // from I* to void* and from void* to I* has to be correct.
        return std::reinterpret_pointer_cast<I>(p);
    }

    template<class I>
    std::shared_ptr<I> resolveRequiredImport(const std::string& module)
    {
        std::shared_ptr<void> p = doResolvePtrByInfo(module, getInterfaceInfo<I>(), std::string_view());
        if (!p) {
            std::cerr << "not found implementation for interface: " << getInterfaceInfo<I>().id << std::endl;
            assert(false);
        }
        return std::reinterpret_pointer_cast<I>(p);
    }

    void reset()
    {
        m_map.clear();
    }

private:

    ModulesIoC() = default;

    void unregisterService(const InterfaceInfo& info)
    {
        m_map.erase(info.id);
    }

    void registerService(const std::string& module,
                         const InterfaceInfo& info,
                         std::shared_ptr<void> p,
                         IModuleCreator* c)
    {
        auto foundIt = m_map.find(info.id);
        if (foundIt != m_map.end()) {
            std::cerr << module << ": double register:"
                      << info.id << ", first register in" << m_map[info.id].sourceModule << std::endl;
            assert(false);
            return;
        }

        Service inj;
        inj.sourceModule = module;
        inj.c = c;
        inj.p = p;
        m_map[info.id] = inj;

        
    }

    std::shared_ptr<void> doResolvePtrByInfo(const std::string_view& usageModule,
                                                         const InterfaceInfo& info,
                                                         const std::string_view& callInfo)
    {
        //! TODO add statistics collection / monitoring, who resolves what

        if (info.internal) {
            if (usageModule != info.module) {
                std::cerr << "Assertion failed!! Interface '" << info.id << "' is internal"
                          << ", usage module: '" << usageModule << "'"
                          << ", interface module: '" << info.module << "'"
                          << ", called from: " << (callInfo.empty() ? std::string_view("unknown") : callInfo)
                          << std::endl;

                #ifndef NDEBUG
                std::abort();
                #endif
            }
        }

        auto it = m_map.find(info.id);
        if (it == m_map.end()) {
            return nullptr;
        }

        Service& inj = it->second;
        if (inj.p) {
            return inj.p;
        }

        if (inj.c) {
            return inj.c->create();
        }

        return nullptr;
    }

    struct Service {
        IModuleCreator* c = nullptr;
        std::string sourceModule;
        std::shared_ptr<void> p;
    };

    std::map<std::string_view, Service > m_map;
};

template<class T>
struct Creator : MODULE_EXPORT_CREATOR
{
    std::shared_ptr<void> create() { return std::make_shared<T>(); }
};
}

#endif // MU_MODULARITY_MODULESIOC_H
