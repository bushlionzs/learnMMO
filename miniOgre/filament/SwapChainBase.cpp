/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include <filament/SwapChain.h>

#include <filament/FEngine.h>

namespace filament {

void* SwapChain::getNativeWindow() const noexcept {
    return downcast(this)->getNativeWindow();
}

void SwapChain::setFrameScheduledCallback(
    backend::CallbackHandler* handler, FrameScheduledCallback&& callback) {
    downcast(this)->setFrameScheduledCallback(handler, std::move(callback));
}




void SwapChain::setFrameCompletedCallback(backend::CallbackHandler* handler,
    utils::Invocable<void(SwapChain*)>&& callback) noexcept {
    return downcast(this)->setFrameCompletedCallback(handler, std::move(callback));
}

bool SwapChain::isSRGBSwapChainSupported(Engine& engine) noexcept {
    return FSwapChain::isSRGBSwapChainSupported(downcast(engine));
}

} // namespace filament