// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <concepts>

namespace saam
{

template <typename T>
concept has_post_constructor = requires(T t) {
    { t.post_constructor() } -> std::same_as<void>;
};

template <typename T>
concept has_pre_destructor = requires(T t) {
    { t.pre_destructor() } -> std::same_as<void>;
};

}  // namespace saam
