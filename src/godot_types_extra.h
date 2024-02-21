#ifndef GODOT_TYPES_EXTRA_H
#define GODOT_TYPES_EXTRA_H

// this file is for everything that I don't yet know how to generate

#include "godot_types_macro.h"

#include "core/math/color.h"

MAKE_NATIVE_TYPE_FACTORY(Color)

// I'm actually not sure that making `Color` a `dasLLManagedValueAnnotation` is a good idea
// pros: you can write both `set_modulate(Color(1.,0.,0.,1.))` and `set_modulate(colors[i])`
// cons: you can't easily add fields with addField
template <> struct das::cast<Color> : das::cast_fVec<Color> {};
template <> struct das::WrapType<Color> { enum { value = true }; typedef das::float4 type; typedef das::float4 rettype; };
template <> struct das::WrapArgType<Color> { struct type : Color {type ( das::float4 t ) : Color(t.x, t.y, t.z, t.w) {}} ; };
template <> struct das::WrapRetType<Color> { struct type : das::float4 {type ( Color t ) : das::float4(t.r, t.g, t.b, t.a) {}} ; };

#include "core/core_bind.h"

DAS_BIND_ENUM_CAST(core_bind::ResourceLoader::CacheMode)
DAS_BASE_BIND_ENUM_SAFE(core_bind::ResourceLoader::CacheMode, ResourceLoader`CacheMode, ResourceLoader_CacheMode, CACHE_MODE_IGNORE, CACHE_MODE_REUSE, CACHE_MODE_REPLACE)

#include "core/variant/variant_utility.h"
#include "core/config/engine.h"


#endif // GODOT_TYPES_EXTRA_H