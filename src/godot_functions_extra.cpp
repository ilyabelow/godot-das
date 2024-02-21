#include "godot_module.h"

#include "godot_types_gen.h"
#include "godot_types_extra.h"

#include "godot_functions_wrapper.h"

#include "das_script_instance.h"
#include "core/core_bind.h"

#include "core/variant/variant_utility.h"
#include "core/config/engine.h"

// Here are functions that are
// 1) not generated yet, but will be
// 2) not in a large enough group to put in a separate file

void Module_Godot::bind_functions_extra(das::ModuleLibrary & lib) {
	// Color
    // not in a macro because Color::named has overloads
    using _Color_named = DAS_CALL_GODOT_STATIC_MEMBER(*static_cast<Color (*)(const String &)>(&Color::named));
    das::addExtern<DAS_BIND_FUN(_Color_named::invoke)>(*this, lib, "Color`named", das::SideEffects::modifyExternal, DAS_CALL_GODOT_STATIC_MEMBER_CPP(Color::named));
    // TODO: add constructors
    // TODO: add fields
    // TODO: add quite a lot actually. but really later - it's really not an important feature and it'll take a lot of work

    BIND_GODOT_SINGLETON_MEMBER(Engine, get_frames_per_second)
    BIND_GODOT_SINGLETON_MEMBER(Input, is_action_just_pressed, "action", "exact_match")
    SET_DEFAULT_ARG(Input, is_action_just_pressed, 1, false)

    BIND_GODOT_BUILTIN_FUNCTION(VariantUtilityFunctions, randf_range)

}
