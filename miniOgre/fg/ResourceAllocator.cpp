/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ResourceAllocator.h"

#include <filament/Engine.h>


#include <filament/DriverApiForward.h>
#include <filament/Handle.h>
#include <filament/TargetBufferInfo.h>
#include <filament/DriverEnums.h>

#include "filament/DriverApi.h"

#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Log.h>
#include <utils/ostream.h>

#include <array>
#include <algorithm>
#include <iterator>
#include <utility>

#include <stddef.h>
#include <stdint.h>

using namespace utils;

namespace filament {

    using namespace backend;

    // ------------------------------------------------------------------------------------------------

    template<typename K, typename V, typename H>
    UTILS_NOINLINE
        ResourceAllocator::AssociativeContainer<K, V, H>::AssociativeContainer() {
        mContainer.reserve(128);
    }

    template<typename K, typename V, typename H>
    UTILS_NOINLINE
        ResourceAllocator::AssociativeContainer<K, V, H>::~AssociativeContainer() noexcept = default;

    template<typename K, typename V, typename H>
    UTILS_NOINLINE
        typename ResourceAllocator::AssociativeContainer<K, V, H>::iterator
        ResourceAllocator::AssociativeContainer<K, V, H>::erase(iterator it) {
        return mContainer.erase(it);
    }

    template<typename K, typename V, typename H>
    typename ResourceAllocator::AssociativeContainer<K, V, H>::const_iterator
        ResourceAllocator::AssociativeContainer<K, V, H>::find(key_type const& key) const {
        return const_cast<AssociativeContainer*>(this)->find(key);
    }

    template<typename K, typename V, typename H>
    UTILS_NOINLINE
        typename ResourceAllocator::AssociativeContainer<K, V, H>::iterator
        ResourceAllocator::AssociativeContainer<K, V, H>::find(key_type const& key) {
        return std::find_if(mContainer.begin(), mContainer.end(), [&key](auto const& v) {
            return v.first == key;
            });
    }

    template<typename K, typename V, typename H>
    template<typename... ARGS>
    UTILS_NOINLINE
        void ResourceAllocator::AssociativeContainer<K, V, H>::emplace(ARGS&& ... args) {
        mContainer.emplace_back(std::forward<ARGS>(args)...);
    }

    // ------------------------------------------------------------------------------------------------
    ResourceAllocatorInterface::~ResourceAllocatorInterface() = default;

    size_t ResourceAllocator::TextureKey::getSize() const noexcept {
        //zhousha
        assert(false);
        return 0;
    }

    ResourceAllocator::ResourceAllocator(Engine::Config const& config, DriverApi& driverApi) noexcept
        : mCacheMaxAge(config.resourceAllocatorCacheMaxAge),
        mBackend(driverApi) {
    }

    ResourceAllocator::~ResourceAllocator() noexcept {
        assert_invariant(!mTextureCache.size());
        assert_invariant(!mInUseTextures.size());
    }

    void ResourceAllocator::terminate() noexcept {
        assert_invariant(!mInUseTextures.size());
        auto& textureCache = mTextureCache;
        for (auto it = textureCache.begin(); it != textureCache.end();) {
            //mBackend.destroyTexture(it->second.handle);
            it = textureCache.erase(it);
        }
    }

    RenderTargetHandle ResourceAllocator::createRenderTarget(const char*,
        TargetBufferFlags targetBufferFlags, uint32_t width, uint32_t height,
        uint8_t samples, uint8_t layerCount, MRT color, TargetBufferInfo depth,
        TargetBufferInfo stencil) noexcept {
        /*return mBackend.createRenderTarget(targetBufferFlags,
            width, height, samples ? samples : 1u, layerCount, color, depth, stencil);*/
        return RenderTargetHandle();
    }

    void ResourceAllocator::destroyRenderTarget(RenderTargetHandle h) noexcept {
        //mBackend.destroyRenderTarget(h);
    }

    backend::TextureHandle ResourceAllocator::createTexture(const char* name,
        backend::SamplerType target, uint8_t levels, backend::TextureFormat format, uint8_t samples,
        uint32_t width, uint32_t height, uint32_t depth,
        std::array<backend::TextureSwizzle, 4> swizzle,
        backend::TextureUsage usage) noexcept {
        
        return TextureHandle();
    }

    void ResourceAllocator::destroyTexture(TextureHandle h) noexcept {
        if constexpr (mEnabled) {
            // find the texture in the in-use list (it must be there!)
            auto it = mInUseTextures.find(h);
            assert_invariant(it != mInUseTextures.end());

            // move it to the cache
            const TextureKey key = it->second;
            uint32_t const size = key.getSize();

            mTextureCache.emplace(key, TextureCachePayload{ h, mAge, size });
            mCacheSize += size;

            // remove it from the in-use list
            mInUseTextures.erase(it);
        }
        else {
            //mBackend.destroyTexture(h);
        }
    }

    void ResourceAllocator::gc() noexcept {
        // this is called regularly -- usually once per frame of each Renderer

        // increase our age
        const size_t age = mAge++;

        // Purging strategy:
        //  - remove entries that are older than a certain age
        //      - remove only one entry per gc(),

        auto& textureCache = mTextureCache;
        for (auto it = textureCache.begin(); it != textureCache.end();) {
            const size_t ageDiff = age - it->second.age;
            if (ageDiff >= mCacheMaxAge) {
                purge(it);
                // only purge a single entry per gc
                break;
            }
            else {
                ++it;
            }
        }
    }

    UTILS_NOINLINE
        void ResourceAllocator::dump(bool brief) const noexcept {
        slog.d << "# entries=" << mTextureCache.size() << ", sz=" << mCacheSize / float(1u << 20u)
            << " MiB" << io::endl;
        if (!brief) {

        }
    }

    void ResourceAllocator::purge(
        ResourceAllocator::CacheContainer::iterator const& pos) {
        //slog.d << "purging " << pos->second.handle.getId() << ", age=" << pos->second.age << io::endl;
        //mBackend.destroyTexture(pos->second.handle);
        mCacheSize -= pos->second.size;
        mTextureCache.erase(pos);
    }

} // namespace filament
