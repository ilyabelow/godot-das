#include "godot_types_extra.h"
#include "godot_module.h"


void Module_Godot::bind_types_extra(das::ModuleLibrary & lib) {
	BIND_NATIVE_VALUE_TYPE(Color)

	// BIND_ENUM(ResourceLoader, CacheMode)
}