#ifndef GODOT_FUNCTIONS_WRAPPER_H
#define GODOT_FUNCTIONS_WRAPPER_H

#include "godot_functions_macro.h"
#include "daScript/daScript.h"
#include "core/typedefs.h"


// This is a separate file because
// 1. godot_functions_macro.h can be used elsewhere (it was used in godot_utils.cpp originally)
// 2. if this file was merged into godot_functions_macro.h it would be too cluttered


template <typename T>
struct simnode_chooser {
    template <typename FuncT, FuncT fun>
    using type = das::SimNode_ExtFuncCall<FuncT, fun>;
};

template <typename TT>
struct simnode_chooser<TypedArray<TT>> {
    template <typename FuncT, FuncT fun>
    using type = das::SimNode_ExtFuncCallAndCopyOrMove<FuncT, fun>;
};

// ====================

template <typename T>
struct godot_to_das {
    typedef T type;
    _FORCE_INLINE_ static T&& ret(T&& t, das::Context *ctx) {
        return std::forward<T>(t);
    }
};


// tmp solution, let's just use floats everywhere
template <>
struct godot_to_das<double> : godot_to_das<float> {};


#define RET_T _FORCE_INLINE_ static type

// This is not really effective, but there's no better way of doing this
// because TypedArray is internally Vector<Variant>. Variant holds ObjData, not Object*, so you can't just cast buffers
// Another solution: rewrite/generate array-returning functions so they put their data directly into das::TArray
#include "core/variant/typed_array.h"

template <typename T>
struct godot_to_das<TypedArray<T>> {
    typedef das::TArray<T*> type;

    RET_T ret(const TypedArray<T>& t, das::Context *ctx) {
        das::TArray<T*> array;
        array.data = nullptr;
        array.size = 0;
        array.capacity = 0;
        array.lock = 0;
        int new_size = t.size();
        if (new_size > 0) {
            das::array_resize(*ctx, array, t.size(), sizeof(T), false, nullptr);
            for (int i = 0; i < t.size(); i++) {
                array[i] = reinterpret_cast<T*>(t[i].operator Object*());
            }
        }
        return array;
    }
};

#include "core/object/ref_counted.h"

template <typename T>
struct godot_to_das<Ref<T>> {
    typedef T* type;
    RET_T ret(Ref<T> t, das::Context *) {
        t->reference(); // should be unreferenced in `unload`
        return t.ptr();
    }
};
template <typename T> struct godot_to_das<const Ref<T>&> : godot_to_das<Ref<T>> { };

// TODO for all value types, turn `const T&` into `T`

#include "core/string/ustring.h"

template<>
struct godot_to_das<String> {
    typedef const char* type;
    RET_T ret(const String& t, das::Context *ctx) {
        return ctx->stringHeap->allocateString(t.utf8().get_data());
    }
};

template<> struct godot_to_das<const String&> : godot_to_das<String> {};

#include "core/string/string_name.h"

// tmp solution, there's probably a good reason to bind StringName
template<> struct godot_to_das<StringName> {
    typedef const char* type;
    RET_T ret(const StringName& t, das::Context *ctx) {
        return ctx->stringHeap->allocateString(t.operator String().utf8().get_data());
    }
};
template<> struct godot_to_das<const StringName&> : godot_to_das<StringName> {};

// =============================================

#define RETURN(V) return godot_to_das<decltype(V)>::ret(V, ctx);
#define CONVERT(T) typename godot_to_das<T>::type
#define CONVERT_R CONVERT(R)
// this is done because VSCode doesn't like `ESCAPE(Args)...` in function declaration for some reason
#define CONVERT_ARGS CONVERT(Args)...

// TODO maybe some functions should have modufyExternal??

// no const, return
template <typename R, typename CC, typename ...Args, R (CC::*func)(Args...) >
struct das_call_godot_member < R (CC::*)(Args...),  func> {
    static CONVERT_R invoke ( CC * THIS, CONVERT_ARGS args, CTX_AT) {
        CHECK_IF_NULL(THIS)
        RETURN( ((*THIS).*(func)) ( args... ) );
    }
    constexpr static das::SideEffects effects = das::SideEffects::modifyArgument;
    typedef std::tuple<CC, CONVERT_ARGS> args;
    typedef simnode_chooser<R> simnode;
};

// no const, no return

template <typename CC, typename ...Args, void (CC::*func)(Args...) >
struct das_call_godot_member < void (CC::*)(Args...),  func> {
    static void invoke ( CC * THIS, CONVERT_ARGS args, CTX_AT) {
        CHECK_IF_NULL_VOID(THIS)
        ((*THIS).*(func)) ( args... );
    }
    constexpr static das::SideEffects effects = das::SideEffects::modifyArgument;
    typedef std::tuple<CC, CONVERT_ARGS> args;
    typedef simnode_chooser<void> simnode;
};

// const, return

template <typename R, typename CC, typename ...Args, R (CC::*func)(Args...) const>
struct das_call_godot_member < R (CC::*)(Args...) const,  func> {
    static CONVERT_R invoke ( const CC * THIS, CONVERT_ARGS args, CTX_AT) {
        CHECK_IF_NULL(THIS)
        RETURN( ((*THIS).*(func)) ( args... ) );
    }
    constexpr static das::SideEffects effects = das::SideEffects::none;
    typedef std::tuple<CC, CONVERT_ARGS> args;
    typedef simnode_chooser<R> simnode;
};

// const, no return

template <typename CC, typename ...Args, void (CC::*func)(Args...) const>
struct das_call_godot_member < void (CC::*)(Args...) const,  func> {
    static void invoke ( const CC * THIS, CONVERT_ARGS args, CTX_AT) {
        CHECK_IF_NULL_VOID(THIS)
        ((*THIS).*(func)) ( args... );
    }
    constexpr static das::SideEffects effects = das::SideEffects::none;
    typedef std::tuple<CC, CONVERT_ARGS> args;
    typedef simnode_chooser<void> simnode;
};

// static variant

template <typename R, typename ...Args, R (*func)(Args...) >
struct das_call_godot_static_member < R (*)(Args...), func> {
    static CONVERT_R invoke (CONVERT_ARGS args, CTX_AT) {
        RETURN( (*func) ( args... ) );
    }
    // modifyExternal - because, for example, random changes global random generator
    constexpr static das::SideEffects effects = das::SideEffects::modifyExternal;
    typedef typename std::tuple<CONVERT_ARGS> args;
};

// singleton variant, const

template <typename R, typename CC, typename ...Args, R (CC::*func)(Args...) const>
struct das_call_godot_singleton_member < R (CC::*)(Args...) const,  func> {
    static CONVERT_R invoke (CONVERT_ARGS args, CTX_AT) {
        RETURN( (CC::get_singleton()->*(func)) ( args... ) );
    }
    constexpr static das::SideEffects effects = das::SideEffects::accessExternal;
    typedef std::tuple<CONVERT_ARGS> args;
};

// singleton variant, no const

template <typename R, typename CC, typename ...Args, R (CC::*func)(Args...)>
struct das_call_godot_singleton_member < R (CC::*)(Args...),  func> {
    static CONVERT_R invoke (CONVERT_ARGS args, CTX_AT) {
        RETURN( (CC::get_singleton()->*(func)) ( args... ) );
    }
    constexpr static das::SideEffects effects = das::SideEffects::accessExternal;
    typedef std::tuple<CONVERT_ARGS> args;
};

#endif // GODOT_FUNCTIONS_WRAPPER_H