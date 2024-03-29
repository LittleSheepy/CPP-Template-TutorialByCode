// xmemory internal header

// Copyright (c) Microsoft Corporation.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once
#ifndef _XMEMORY_
#define _XMEMORY_
#include <yvals_core.h>
#if _STL_COMPILER_PREPROCESSOR
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <new>
#include <xatomic.h>
#include <xutility>

#pragma pack(push, _CRT_PACKING)
#pragma warning(push, _STL_WARNING_LEVEL)
#pragma warning(disable : _STL_DISABLED_WARNINGS)
_STL_DISABLE_CLANG_WARNINGS
#pragma push_macro("new")
#undef new

_STD_BEGIN
// STRUCT TEMPLATE _Tidy_guard
template <class _Ty>
struct _Tidy_guard { // class with destructor that calls _Tidy
    _Ty* _Target;
    ~_Tidy_guard() {
        if (_Target) {
            _Target->_Tidy();
        }
    }
};

// STRUCT TEMPLATE _Tidy_deallocate_guard
template <class _Ty>
struct _Tidy_deallocate_guard { // class with destructor that calls _Tidy_deallocate
    _Ty* _Target;
    ~_Tidy_deallocate_guard() {
        if (_Target) {
            _Target->_Tidy_deallocate();
        }
    }
};

// VARIABLE TEMPLATE _Nothrow_compare
template <class _Keycmp, class _Lhs, class _Rhs>
_INLINE_VAR constexpr bool _Nothrow_compare = noexcept(
    static_cast<bool>(_STD declval<const _Keycmp&>()(_STD declval<const _Lhs&>(), _STD declval<const _Rhs&>())));

// FUNCTION TEMPLATE _Get_size_of_n
template <size_t _Ty_size>
_NODISCARD constexpr size_t _Get_size_of_n(const size_t _Count) {
    constexpr bool _Overflow_is_possible = _Ty_size > 1;

    if _CONSTEXPR_IF (_Overflow_is_possible) {
        constexpr size_t _Max_possible = static_cast<size_t>(-1) / _Ty_size;
        if (_Count > _Max_possible) {
            _Throw_bad_array_new_length(); // multiply overflow
        }
    }

    return _Count * _Ty_size;
}

// VARIABLE TEMPLATE _New_alignof
template <class _Ty>
_INLINE_VAR constexpr size_t _New_alignof = (_STD max)(alignof(_Ty),
    static_cast<size_t>(__STDCPP_DEFAULT_NEW_ALIGNMENT__) // TRANSITION, VSO-522105
);

// STRUCT _Default_allocate_traits
struct _Default_allocate_traits {
    __declspec(allocator) static void* _Allocate(const size_t _Bytes) {
        return ::operator new(_Bytes);
    }

#ifdef __cpp_aligned_new
    __declspec(allocator) static void* _Allocate_aligned(const size_t _Bytes, const size_t _Align) {
        return ::operator new (_Bytes, align_val_t{_Align});
    }
#endif // __cpp_aligned_new
};

constexpr bool _Is_pow_2(const size_t _Value) noexcept {
    return _Value != 0 && (_Value & (_Value - 1)) == 0;
}

#if defined(_M_IX86) || defined(_M_X64)
constexpr size_t _Big_allocation_threshold = 4096;
constexpr size_t _Big_allocation_alignment = 32;

static_assert(2 * sizeof(void*) <= _Big_allocation_alignment,
    "Big allocation alignment should at least match vector register alignment");
static_assert(_Is_pow_2(_Big_allocation_alignment), "Big allocation alignment must be a power of two");

#ifdef _DEBUG
constexpr size_t _Non_user_size = 2 * sizeof(void*) + _Big_allocation_alignment - 1;
#else // _DEBUG
constexpr size_t _Non_user_size           = sizeof(void*) + _Big_allocation_alignment - 1;
#endif // _DEBUG

#ifdef _WIN64
constexpr size_t _Big_allocation_sentinel = 0xFAFAFAFAFAFAFAFAULL;
#else // ^^^ _WIN64 ^^^ // vvv !_WIN64 vvv
constexpr size_t _Big_allocation_sentinel = 0xFAFAFAFAUL;
#endif // _WIN64

// FUNCTION _Allocate_manually_vector_aligned
template <class _Traits>
__declspec(allocator) void* _Allocate_manually_vector_aligned(const size_t _Bytes) {
    // allocate _Bytes manually aligned to at least _Big_allocation_alignment
    const size_t _Block_size = _Non_user_size + _Bytes;
    if (_Block_size <= _Bytes) {
        _Throw_bad_array_new_length(); // add overflow
    }

    const uintptr_t _Ptr_container = reinterpret_cast<uintptr_t>(_Traits::_Allocate(_Block_size));
    _STL_VERIFY(_Ptr_container != 0, "invalid argument"); // validate even in release since we're doing p[-1]
    void* const _Ptr = reinterpret_cast<void*>((_Ptr_container + _Non_user_size) & ~(_Big_allocation_alignment - 1));
    static_cast<uintptr_t*>(_Ptr)[-1] = _Ptr_container;

#ifdef _DEBUG
    static_cast<uintptr_t*>(_Ptr)[-2] = _Big_allocation_sentinel;
#endif // _DEBUG
    return _Ptr;
}

// FUNCTION TEMPLATE _Adjust_manually_vector_aligned
inline void _Adjust_manually_vector_aligned(void*& _Ptr, size_t& _Bytes) {
    // adjust parameters from _Allocate_manually_vector_aligned to pass to operator delete
    _Bytes += _Non_user_size;

    const uintptr_t* const _Ptr_user = reinterpret_cast<uintptr_t*>(_Ptr);
    const uintptr_t _Ptr_container   = _Ptr_user[-1];

    // If the following asserts, it likely means that we are performing
    // an aligned delete on memory coming from an unaligned allocation.
    _STL_ASSERT(_Ptr_user[-2] == _Big_allocation_sentinel, "invalid argument");

    // Extra paranoia on aligned allocation/deallocation; ensure _Ptr_container is
    // in range [_Min_back_shift, _Non_user_size]
#ifdef _DEBUG
    constexpr uintptr_t _Min_back_shift = 2 * sizeof(void*);
#else // ^^^ _DEBUG ^^^ // vvv !_DEBUG vvv
    constexpr uintptr_t _Min_back_shift = sizeof(void*);
#endif // _DEBUG
    const uintptr_t _Back_shift = reinterpret_cast<uintptr_t>(_Ptr) - _Ptr_container;
    _STL_VERIFY(_Back_shift >= _Min_back_shift && _Back_shift <= _Non_user_size, "invalid argument");
    _Ptr = reinterpret_cast<void*>(_Ptr_container);
}
#endif // defined(_M_IX86) || defined(_M_X64)

// FUNCTION TEMPLATES _Allocate and _Deallocate
#ifdef __cpp_aligned_new
template <size_t _Align, class _Traits = _Default_allocate_traits,
    enable_if_t<(_Align > __STDCPP_DEFAULT_NEW_ALIGNMENT__), int> = 0>
__declspec(allocator) void* _Allocate(const size_t _Bytes) {
    // allocate _Bytes when __cpp_aligned_new && _Align > __STDCPP_DEFAULT_NEW_ALIGNMENT__
    if (_Bytes == 0) {
        return nullptr;
    }

    size_t _Passed_align = _Align;
#if defined(_M_IX86) || defined(_M_X64)
    if (_Bytes >= _Big_allocation_threshold) {
        // boost the alignment of big allocations to help autovectorization
        _Passed_align = (_STD max)(_Align, _Big_allocation_alignment);
    }
#endif // defined(_M_IX86) || defined(_M_X64)

    return _Traits::_Allocate_aligned(_Bytes, _Passed_align);
}

template <size_t _Align, enable_if_t<(_Align > __STDCPP_DEFAULT_NEW_ALIGNMENT__), int> = 0>
void _Deallocate(void* _Ptr, const size_t _Bytes) noexcept {
    // deallocate storage allocated by _Allocate when __cpp_aligned_new && _Align > __STDCPP_DEFAULT_NEW_ALIGNMENT__
    size_t _Passed_align = _Align;
#if defined(_M_IX86) || defined(_M_X64)
    if (_Bytes >= _Big_allocation_threshold) { // boost the alignment of big allocations to help autovectorization
        _Passed_align = (_STD max)(_Align, _Big_allocation_alignment);
    }
#endif // defined(_M_IX86) || defined(_M_X64)

    ::operator delete (_Ptr, _Bytes, align_val_t{_Passed_align});
}

#define _HAS_ALIGNED_NEW 1
#else // ^^^ __cpp_aligned_new ^^^ / vvv !__cpp_aligned_new vvv
#define _HAS_ALIGNED_NEW 0
#endif // __cpp_aligned_new

template <size_t _Align, class _Traits = _Default_allocate_traits,
    enable_if_t<(!_HAS_ALIGNED_NEW || _Align <= __STDCPP_DEFAULT_NEW_ALIGNMENT__), int> = 0>
__declspec(allocator) void* _Allocate(const size_t _Bytes) {
    // allocate _Bytes when !_HAS_ALIGNED_NEW || _Align <= __STDCPP_DEFAULT_NEW_ALIGNMENT__
#if defined(_M_IX86) || defined(_M_X64)
    if (_Bytes >= _Big_allocation_threshold) { // boost the alignment of big allocations to help autovectorization
        return _Allocate_manually_vector_aligned<_Traits>(_Bytes);
    }
#endif // defined(_M_IX86) || defined(_M_X64)

    if (_Bytes != 0) {
        return _Traits::_Allocate(_Bytes);
    }

    return nullptr;
}

template <size_t _Align, enable_if_t<(!_HAS_ALIGNED_NEW || _Align <= __STDCPP_DEFAULT_NEW_ALIGNMENT__), int> = 0>
void _Deallocate(void* _Ptr, size_t _Bytes) noexcept {
    // deallocate storage allocated by _Allocate when !_HAS_ALIGNED_NEW || _Align <= __STDCPP_DEFAULT_NEW_ALIGNMENT__
#if defined(_M_IX86) || defined(_M_X64)
    if (_Bytes >= _Big_allocation_threshold) { // boost the alignment of big allocations to help autovectorization
        _Adjust_manually_vector_aligned(_Ptr, _Bytes);
    }
#endif // defined(_M_IX86) || defined(_M_X64)

    ::operator delete(_Ptr, _Bytes);
}

#undef _HAS_ALIGNED_NEW
// >>>>> list<int>
// 类型 _Construct_in_place(_Container_proxy, _Container_base12 * __ptr64)
// _Ty = _Container_proxy
// FUNCTION TEMPLATE _Construct_in_place  构造函数
template <class _Ty, class... _Types>
void _Construct_in_place(_Ty& _Obj, _Types&&... _Args) noexcept(is_nothrow_constructible_v<_Ty, _Types...>) {
    //cout <<typeid(forward<_Types>).name() << endl;
    ::new (const_cast<void*>(static_cast<const volatile void*>(_STD addressof(_Obj))))
        _Ty(_STD forward<_Types>(_Args)...);
    //上面的代码就是就是在某一块内存上面，调用某一个类的构造函数， 估计很多人都是第一次见这种语法。
}

// FUNCTION TEMPLATE _Global_new
template <class _Ty, class... _Types>
_Ty* _Global_new(_Types&&... _Args) { // acts as "new" while disallowing user overload selection
    struct _Guard_type {
        void* _Result;
        ~_Guard_type() {
            if (_Result) {
                _Deallocate<_New_alignof<_Ty>>(_Result, sizeof(_Ty));
            }
        }
    };

    _Guard_type _Guard{_Allocate<_New_alignof<_Ty>>(sizeof(_Ty))};
    ::new (_Guard._Result) _Ty(_STD forward<_Types>(_Args)...);
    return static_cast<_Ty*>(_STD exchange(_Guard._Result, nullptr));
}

// ALIAS TEMPLATE _Rebind_pointer_t
template <class _Ptr, class _Ty>
using _Rebind_pointer_t = typename pointer_traits<_Ptr>::template rebind<_Ty>;

// FUNCTION TEMPLATE _Refancy
template <class _Pointer, enable_if_t<!is_pointer_v<_Pointer>, int> = 0>
_Pointer _Refancy(typename pointer_traits<_Pointer>::element_type* _Ptr) noexcept {
    return pointer_traits<_Pointer>::pointer_to(*_Ptr);
}

template <class _Pointer, enable_if_t<is_pointer_v<_Pointer>, int> = 0>
_Pointer _Refancy(_Pointer _Ptr) noexcept {
    return _Ptr;
}

// FUNCTION TEMPLATE _Destroy_in_place
template <class _Ty>
void _Destroy_in_place(_Ty& _Obj) noexcept {
    _Obj.~_Ty();
}

// FUNCTION TEMPLATE _Const_cast
template <class _Ptrty>
auto _Const_cast(_Ptrty _Ptr) noexcept { // remove constness from a fancy pointer
    using _Elem       = typename pointer_traits<_Ptrty>::element_type;
    using _Modifiable = remove_const_t<_Elem>;
    using _Dest       = typename pointer_traits<_Ptrty>::template rebind<_Modifiable>;

    return pointer_traits<_Dest>::pointer_to(const_cast<_Modifiable&>(*_Ptr));
}

template <class _Ty>
auto _Const_cast(_Ty* _Ptr) noexcept {
    return const_cast<remove_const_t<_Ty>*>(_Ptr);
}

// STRUCT TEMPLATE _Get_pointer_type
template <class _Ty, class = void>
struct _Get_pointer_type {
    using type = typename _Ty::value_type*;
};

_STL_DISABLE_DEPRECATED_WARNING
template <class _Ty>
struct _Get_pointer_type<_Ty, void_t<typename _Ty::pointer>> {
    using type = typename _Ty::pointer;
};
_STL_RESTORE_DEPRECATED_WARNING

// STRUCT TEMPLATE _Get_const_pointer_type
template <class _Ty, class = void>
struct _Get_const_pointer_type {
    using _Ptrty = typename _Get_pointer_type<_Ty>::type;
    using _Valty = typename _Ty::value_type;
    using type   = typename pointer_traits<_Ptrty>::template rebind<const _Valty>;
};

_STL_DISABLE_DEPRECATED_WARNING
template <class _Ty>
struct _Get_const_pointer_type<_Ty, void_t<typename _Ty::const_pointer>> {
    using type = typename _Ty::const_pointer;
};
_STL_RESTORE_DEPRECATED_WARNING

// STRUCT TEMPLATE _Get_void_pointer_type
template <class _Ty, class = void>
struct _Get_void_pointer_type {
    using _Ptrty = typename _Get_pointer_type<_Ty>::type;
    using type   = typename pointer_traits<_Ptrty>::template rebind<void>;
};

template <class _Ty>
struct _Get_void_pointer_type<_Ty, void_t<typename _Ty::void_pointer>> {
    using type = typename _Ty::void_pointer;
};

// STRUCT TEMPLATE _Get_const_void_pointer_type
template <class _Ty, class = void>
struct _Get_const_void_pointer_type {
    using _Ptrty = typename _Get_pointer_type<_Ty>::type;
    using type   = typename pointer_traits<_Ptrty>::template rebind<const void>;
};

template <class _Ty>
struct _Get_const_void_pointer_type<_Ty, void_t<typename _Ty::const_void_pointer>> {
    using type = typename _Ty::const_void_pointer;
};

// STRUCT TEMPLATE _Get_difference_type
template <class _Ty, class = void>
struct _Get_difference_type {
    using _Ptrty = typename _Get_pointer_type<_Ty>::type;
    using type   = typename pointer_traits<_Ptrty>::difference_type;
};

template <class _Ty>
struct _Get_difference_type<_Ty, void_t<typename _Ty::difference_type>> {
    using type = typename _Ty::difference_type;
};

// STRUCT TEMPLATE _Get_size_type
template <class _Ty, class = void>
struct _Get_size_type {
    using type = make_unsigned_t<typename _Get_difference_type<_Ty>::type>;
};

template <class _Ty>
struct _Get_size_type<_Ty, void_t<typename _Ty::size_type>> {
    using type = typename _Ty::size_type;
};

// STRUCT TEMPLATE _Get_propagate_on_container_copy
template <class _Ty, class = void>
struct _Get_propagate_on_container_copy {
    using type = false_type;
};

template <class _Ty>
struct _Get_propagate_on_container_copy<_Ty, void_t<typename _Ty::propagate_on_container_copy_assignment>> {
    using type = typename _Ty::propagate_on_container_copy_assignment;
};

// STRUCT TEMPLATE _Get_propagate_on_container_move
template <class _Ty, class = void>
struct _Get_propagate_on_container_move {
    using type = false_type;
};

template <class _Ty>
struct _Get_propagate_on_container_move<_Ty, void_t<typename _Ty::propagate_on_container_move_assignment>> {
    using type = typename _Ty::propagate_on_container_move_assignment;
};

// STRUCT TEMPLATE _Get_propagate_on_container_swap
template <class _Ty, class = void>
struct _Get_propagate_on_container_swap {
    using type = false_type;
};

template <class _Ty>
struct _Get_propagate_on_container_swap<_Ty, void_t<typename _Ty::propagate_on_container_swap>> {
    using type = typename _Ty::propagate_on_container_swap;
};

// STRUCT TEMPLATE _Get_is_always_equal
template <class _Ty, class = void>
struct _Get_is_always_equal {
    using type = typename is_empty<_Ty>::type;
};

template <class _Ty>
struct _Get_is_always_equal<_Ty, void_t<typename _Ty::is_always_equal>> {
    using type = typename _Ty::is_always_equal;
};

// STRUCT TEMPLATE _Get_rebind_type
template <class _Ty, class _Other, class = void>
struct _Get_rebind_type {
    using type = typename _Replace_first_parameter<_Other, _Ty>::type;
};

_STL_DISABLE_DEPRECATED_WARNING
template <class _Ty, class _Other>
struct _Get_rebind_type<_Ty, _Other, void_t<typename _Ty::template rebind<_Other>::other>> {
    using type = typename _Ty::template rebind<_Other>::other;
};
_STL_RESTORE_DEPRECATED_WARNING

// STRUCT TEMPLATE _Is_default_allocator
template <class _Ty>
class allocator;

template <class _Alloc, class = void>
struct _Is_default_allocator : false_type {};

template <class _Ty>
struct _Is_default_allocator<allocator<_Ty>, void_t<typename allocator<_Ty>::_From_primary>>
    : is_same<typename allocator<_Ty>::_From_primary, allocator<_Ty>>::type {};

// ALIAS TEMPLATES _Uses_default_construct
template <class _Void, class... _Types>
struct _Has_no_allocator_construct : true_type {};

_STL_DISABLE_DEPRECATED_WARNING
template <class _Alloc, class _Ptr, class... _Args>
struct _Has_no_allocator_construct<
    void_t<decltype(_STD declval<_Alloc&>().construct(_STD declval<_Ptr>(), _STD declval<_Args>()...))>, _Alloc, _Ptr,
    _Args...> : false_type {};
_STL_RESTORE_DEPRECATED_WARNING

template <class _Alloc, class _Ptr, class... _Args>
using _Uses_default_construct =
    disjunction<_Is_default_allocator<_Alloc>, _Has_no_allocator_construct<void, _Alloc, _Ptr, _Args...>>;


// ALIAS TEMPLATE _Uses_default_destroy AND _Uses_default_destroy_t
template <class _Alloc, class _Ptr, class = void>
struct _Has_no_alloc_destroy : true_type {};

_STL_DISABLE_DEPRECATED_WARNING
template <class _Alloc, class _Ptr>
struct _Has_no_alloc_destroy<_Alloc, _Ptr, void_t<decltype(_STD declval<_Alloc&>().destroy(_STD declval<_Ptr>()))>>
    : false_type {};
_STL_RESTORE_DEPRECATED_WARNING

template <class _Alloc, class _Ptr>
using _Uses_default_destroy = disjunction<_Is_default_allocator<_Alloc>, _Has_no_alloc_destroy<_Alloc, _Ptr>>;

template <class _Alloc, class _Ptr>
using _Uses_default_destroy_t = typename _Uses_default_destroy<_Alloc, _Ptr>::type;


// STRUCT TEMPLATE _Has_allocate_hint
template <class _Alloc, class _Size_type, class _Const_void_pointer, class = void>
struct _Has_allocate_hint : false_type {};

_STL_DISABLE_DEPRECATED_WARNING
template <class _Alloc, class _Size_type, class _Const_void_pointer>
struct _Has_allocate_hint<_Alloc, _Size_type, _Const_void_pointer,
    void_t<decltype(_STD declval<_Alloc&>().allocate(
        _STD declval<const _Size_type&>(), _STD declval<const _Const_void_pointer&>()))>> : true_type {};
_STL_RESTORE_DEPRECATED_WARNING

// STRUCT TEMPLATE _Has_max_size
template <class _Alloc, class = void>
struct _Has_max_size : false_type {};

_STL_DISABLE_DEPRECATED_WARNING
template <class _Alloc>
struct _Has_max_size<_Alloc, void_t<decltype(_STD declval<const _Alloc&>().max_size())>> : true_type {};
_STL_RESTORE_DEPRECATED_WARNING

// STRUCT TEMPLATE _Has_select_on_container_copy_construction
template <class _Alloc, class = void>
struct _Has_select_on_container_copy_construction : false_type {};

template <class _Alloc>
struct _Has_select_on_container_copy_construction<_Alloc,
    void_t<decltype(_STD declval<const _Alloc&>().select_on_container_copy_construction())>> : true_type {};


// STRUCT TEMPLATE allocator_traits
template <class _Alloc>
struct allocator_traits;

_STL_DISABLE_DEPRECATED_WARNING
template <class _Alloc>
struct _Normal_allocator_traits { // defines traits for allocators
    using allocator_type = _Alloc;
    using value_type     = typename _Alloc::value_type;

    using pointer            = typename _Get_pointer_type<_Alloc>::type;
    using const_pointer      = typename _Get_const_pointer_type<_Alloc>::type;
    using void_pointer       = typename _Get_void_pointer_type<_Alloc>::type;
    using const_void_pointer = typename _Get_const_void_pointer_type<_Alloc>::type;

    using size_type       = typename _Get_size_type<_Alloc>::type;
    using difference_type = typename _Get_difference_type<_Alloc>::type;

    using propagate_on_container_copy_assignment = typename _Get_propagate_on_container_copy<_Alloc>::type;
    using propagate_on_container_move_assignment = typename _Get_propagate_on_container_move<_Alloc>::type;
    using propagate_on_container_swap            = typename _Get_propagate_on_container_swap<_Alloc>::type;
    using is_always_equal                        = typename _Get_is_always_equal<_Alloc>::type;

    template <class _Other>
    using rebind_alloc = typename _Get_rebind_type<_Alloc, _Other>::type;

    template <class _Other>
    using rebind_traits = allocator_traits<rebind_alloc<_Other>>;

    _NODISCARD static __declspec(allocator) pointer allocate(_Alloc& _Al, _CRT_GUARDOVERFLOW const size_type _Count) {
        return _Al.allocate(_Count);
    }

#if _HAS_IF_CONSTEXPR
    _NODISCARD static __declspec(allocator) pointer
        allocate(_Alloc& _Al, _CRT_GUARDOVERFLOW const size_type _Count, const const_void_pointer _Hint) {
        if constexpr (_Has_allocate_hint<_Alloc, size_type, const_void_pointer>::value) {
            return _Al.allocate(_Count, _Hint);
        } else {
            return _Al.allocate(_Count);
        }
    }
#else // ^^^ _HAS_IF_CONSTEXPR ^^^ // vvv !_HAS_IF_CONSTEXPR vvv
    static __declspec(allocator) pointer
        _Allocate1(_Alloc& _Al, _CRT_GUARDOVERFLOW const size_type _Count, const const_void_pointer _Hint, true_type) {
        return _Al.allocate(_Count, _Hint);
    }

    static __declspec(allocator) pointer
        _Allocate1(_Alloc& _Al, _CRT_GUARDOVERFLOW const size_type _Count, const_void_pointer, false_type) {
        return _Al.allocate(_Count);
    }

    _NODISCARD static __declspec(allocator) pointer
        allocate(_Alloc& _Al, _CRT_GUARDOVERFLOW const size_type _Count, const const_void_pointer _Hint) {
        return _Allocate1(_Al, _Count, _Hint, _Has_allocate_hint<_Alloc, size_type, const_void_pointer>{});
    }
#endif // _HAS_IF_CONSTEXPR

    static void deallocate(_Alloc& _Al, pointer _Ptr, size_type _Count) {
        _Al.deallocate(_Ptr, _Count);
    }

#if _HAS_IF_CONSTEXPR
    template <class _Ty, class... _Types>
    static void construct(_Alloc& _Al, _Ty* _Ptr, _Types&&... _Args) {
        if constexpr (_Uses_default_construct<_Alloc, _Ty*, _Types...>::value) {
            (void) _Al; // TRANSITION, DevCom-1004719
            ::new (static_cast<void*>(_Ptr)) _Ty(_STD forward<_Types>(_Args)...);
        } else {
            _Al.construct(_Ptr, _STD forward<_Types>(_Args)...);
        }
    }
#else // ^^^ _HAS_IF_CONSTEXPR ^^^ // vvv !_HAS_IF_CONSTEXPR vvv
    template <class _Ty, class... _Types>
    static void _Construct1(true_type, _Alloc&, _Ty* _Ptr, _Types&&... _Args) {
        ::new (static_cast<void*>(_Ptr)) _Ty(_STD forward<_Types>(_Args)...);
    }

    template <class _Ty, class... _Types>
    static void _Construct1(false_type, _Alloc& _Al, _Ty* _Ptr, _Types&&... _Args) {
        _Al.construct(_Ptr, _STD forward<_Types>(_Args)...);
    }

    template <class _Ty, class... _Types>
    static void construct(_Alloc& _Al, _Ty* _Ptr, _Types&&... _Args) {
        _Construct1(typename _Uses_default_construct<_Alloc, _Ty*, _Types...>::type{}, _Al, _Ptr,
            _STD forward<_Types>(_Args)...);
    }
#endif // _HAS_IF_CONSTEXPR

#if _HAS_IF_CONSTEXPR
    template <class _Ty>
    static void destroy(_Alloc& _Al, _Ty* _Ptr) {
        if constexpr (_Uses_default_destroy<_Alloc, _Ty*>::value) {
            _Ptr->~_Ty();
        } else {
            _Al.destroy(_Ptr);
        }
    }
#else // ^^^ _HAS_IF_CONSTEXPR ^^^ // vvv !_HAS_IF_CONSTEXPR vvv
    template <class _Ty>
    static void _Destroy1(_Alloc&, _Ty* _Ptr, true_type) {
        _Ptr->~_Ty();
    }

    template <class _Ty>
    static void _Destroy1(_Alloc& _Al, _Ty* _Ptr, false_type) {
        _Al.destroy(_Ptr);
    }

    template <class _Ty>
    static void destroy(_Alloc& _Al, _Ty* _Ptr) {
        _Destroy1(_Al, _Ptr, _Uses_default_destroy_t<_Alloc, _Ty*>());
    }
#endif // _HAS_IF_CONSTEXPR

#if _HAS_IF_CONSTEXPR
    _NODISCARD static size_type max_size(const _Alloc& _Al) noexcept {
        if constexpr (_Has_max_size<_Alloc>::value) {
            return _Al.max_size();
        } else {
            return (numeric_limits<size_type>::max)() / sizeof(value_type);
        }
    }
#else // ^^^ _HAS_IF_CONSTEXPR ^^^ // vvv !_HAS_IF_CONSTEXPR vvv
    static size_type _Max_size1(const _Alloc& _Al, true_type) noexcept {
        return _Al.max_size();
    }

    static size_type _Max_size1(const _Alloc&, false_type) noexcept {
        return (numeric_limits<size_type>::max)() / sizeof(value_type);
    }

    _NODISCARD static size_type max_size(const _Alloc& _Al) noexcept {
        return _Max_size1(_Al, _Has_max_size<_Alloc>{});
    }
#endif // _HAS_IF_CONSTEXPR

#if _HAS_IF_CONSTEXPR
    _NODISCARD static _Alloc select_on_container_copy_construction(const _Alloc& _Al) {
        if constexpr (_Has_select_on_container_copy_construction<_Alloc>::value) {
            return _Al.select_on_container_copy_construction();
        } else {
            return _Al;
        }
    }
#else // ^^^ _HAS_IF_CONSTEXPR ^^^ // vvv !_HAS_IF_CONSTEXPR vvv
    static _Alloc _Select_on_container_copy_construction1(const _Alloc& _Al, true_type) {
        return _Al.select_on_container_copy_construction();
    }

    static _Alloc _Select_on_container_copy_construction1(const _Alloc& _Al, false_type) {
        return _Al;
    }

    _NODISCARD static _Alloc select_on_container_copy_construction(const _Alloc& _Al) {
        return _Select_on_container_copy_construction1(_Al, _Has_select_on_container_copy_construction<_Alloc>{});
    }
#endif // _HAS_IF_CONSTEXPR
};
_STL_RESTORE_DEPRECATED_WARNING

template <class _Alloc>
struct _Default_allocator_traits { // traits for std::allocator
    using allocator_type = _Alloc;
    using value_type     = typename _Alloc::value_type;

    using pointer            = value_type*;
    using const_pointer      = const value_type*;
    using void_pointer       = void*;
    using const_void_pointer = const void*;

    using size_type       = size_t;
    using difference_type = ptrdiff_t;

    using propagate_on_container_copy_assignment = false_type;
    using propagate_on_container_move_assignment = true_type;
    using propagate_on_container_swap            = false_type;
    using is_always_equal                        = true_type;

    template <class _Other>
    using rebind_alloc = allocator<_Other>;

    template <class _Other>
    using rebind_traits = allocator_traits<allocator<_Other>>;

    _NODISCARD static __declspec(allocator) pointer allocate(_Alloc&, _CRT_GUARDOVERFLOW const size_type _Count) {
        return static_cast<pointer>(_Allocate<_New_alignof<value_type>>(_Get_size_of_n<sizeof(value_type)>(_Count)));
    }

    _NODISCARD static __declspec(allocator) pointer
        allocate(_Alloc&, _CRT_GUARDOVERFLOW const size_type _Count, const_void_pointer) {
        return static_cast<pointer>(_Allocate<_New_alignof<value_type>>(_Get_size_of_n<sizeof(value_type)>(_Count)));
    }

    static void deallocate(_Alloc&, const pointer _Ptr, const size_type _Count) {
        // no overflow check on the following multiply; we assume _Allocate did that check
        _Deallocate<_New_alignof<value_type>>(_Ptr, sizeof(value_type) * _Count);
    }

    template <class _Objty, class... _Types>
    static void construct(_Alloc&, _Objty* const _Ptr, _Types&&... _Args) {
        ::new (const_cast<void*>(static_cast<const volatile void*>(_Ptr))) _Objty(_STD forward<_Types>(_Args)...);
    }

    template <class _Uty>
    static void destroy(_Alloc&, _Uty* const _Ptr) {
        _Ptr->~_Uty();
    }

    _NODISCARD static size_type max_size(const _Alloc&) noexcept {
        return static_cast<size_t>(-1) / sizeof(value_type);
    }

    _NODISCARD static _Alloc select_on_container_copy_construction(const _Alloc& _Al) {
        return _Al;
    }
};
// conditional_t 条件
template <class _Alloc>
struct allocator_traits : conditional_t<_Is_default_allocator<_Alloc>::value, 
                                        _Default_allocator_traits<_Alloc>,
                                        _Normal_allocator_traits<_Alloc>
                                       > {};

// _Choose_pocca returns whether an attempt to propagate allocators is necessary in copy assignment operations.
// Note that even when false_type, callers should call _Pocca as we want to assign allocators even when equal.
template <class _Alloc>
using _Choose_pocca = bool_constant<allocator_traits<_Alloc>::propagate_on_container_copy_assignment::value
                                    && !allocator_traits<_Alloc>::is_always_equal::value>;

struct _Equal_allocators {}; // usually allows contents to be stolen (e.g. with swap)
using _Propagate_allocators    = true_type; // usually allows the allocator to be propagated, and then contents stolen
using _No_propagate_allocators = false_type; // usually turns moves into copies

template <class _Alloc>
using _Choose_pocma = conditional_t<allocator_traits<_Alloc>::is_always_equal::value, _Equal_allocators,
    typename allocator_traits<_Alloc>::propagate_on_container_move_assignment::type>;

// ALIAS TEMPLATE _Rebind_alloc_t
// 
template <class _Alloc, class _Value_type>
using _Rebind_alloc_t = typename allocator_traits<_Alloc>::template rebind_alloc<_Value_type>;

// ALIAS TEMPLATE _Maybe_rebind_alloc_t
// If _Alloc is already rebound appropriately, binds an lvalue reference to it, avoiding a copy. Otherwise, creates a
// rebound copy.
template <class _Alloc, class _Value_type>
using _Maybe_rebind_alloc_t =
    typename _Select<is_same_v<typename _Alloc::value_type, _Value_type>>::template _Apply<_Alloc&,
        _Rebind_alloc_t<_Alloc, _Value_type>>;

// VARIABLE TEMPLATE _Is_simple_alloc_v
template <class _Alloc> // tests if allocator has simple addressing
_INLINE_VAR constexpr bool _Is_simple_alloc_v = is_same_v<typename allocator_traits<_Alloc>::size_type, size_t>&&
    is_same_v<typename allocator_traits<_Alloc>::difference_type, ptrdiff_t>&&
        is_same_v<typename allocator_traits<_Alloc>::pointer, typename _Alloc::value_type*>&&
            is_same_v<typename allocator_traits<_Alloc>::const_pointer, const typename _Alloc::value_type*>;

// STRUCT TEMPLATE _Simple_types
template <class _Value_type>
struct _Simple_types { // wraps types from allocators with simple addressing for use in iterators
                       // and other SCARY machinery
    using value_type      = _Value_type;
    using size_type       = size_t;
    using difference_type = ptrdiff_t;
    using pointer         = value_type*;
    using const_pointer   = const value_type*;
};

// CLASS TEMPLATE allocator
template <class _Ty>
class allocator {
public:
    static_assert(!is_const_v<_Ty>, "The C++ Standard forbids containers of const elements "
                                    "because allocator<const T> is ill-formed.");

    using _From_primary = allocator;

    using value_type = _Ty;

    _CXX17_DEPRECATE_OLD_ALLOCATOR_MEMBERS typedef _Ty* pointer;
    _CXX17_DEPRECATE_OLD_ALLOCATOR_MEMBERS typedef const _Ty* const_pointer;

    _CXX17_DEPRECATE_OLD_ALLOCATOR_MEMBERS typedef _Ty& reference;
    _CXX17_DEPRECATE_OLD_ALLOCATOR_MEMBERS typedef const _Ty& const_reference;

    using size_type       = size_t;
    using difference_type = ptrdiff_t;

    using propagate_on_container_move_assignment = true_type;
    using is_always_equal                        = true_type;

    template <class _Other>
    struct _CXX17_DEPRECATE_OLD_ALLOCATOR_MEMBERS rebind {
        using other = allocator<_Other>;
    };

    _CXX17_DEPRECATE_OLD_ALLOCATOR_MEMBERS _NODISCARD _Ty* address(_Ty& _Val) const noexcept {
        return _STD addressof(_Val);
    }

    _CXX17_DEPRECATE_OLD_ALLOCATOR_MEMBERS _NODISCARD const _Ty* address(const _Ty& _Val) const noexcept {
        return _STD addressof(_Val);
    }

    constexpr allocator() noexcept {}

    constexpr allocator(const allocator&) noexcept = default;
    template <class _Other>
    constexpr allocator(const allocator<_Other>&) noexcept {}

    void deallocate(_Ty* const _Ptr, const size_t _Count) {
        // no overflow check on the following multiply; we assume _Allocate did that check
        _Deallocate<_New_alignof<_Ty>>(_Ptr, sizeof(_Ty) * _Count);
    }
    // _NODISCARD 返回值不处理会警告 [[nodiscard]]
    _NODISCARD __declspec(allocator) _Ty* allocate(_CRT_GUARDOVERFLOW const size_t _Count) {
        //size_t size = sizeof(_Ty);  // 16
        //size_t _Bytes = _Get_size_of_n<size>(_Count);
        //return static_cast<_Ty*>(   _Allocate<_New_alignof<_Ty>>(_Bytes) );
        // >>>>> list<int>
        // typeid(_Ty).name() = _Container_proxy
        // _New_alignof<_Ty> = 16
        // sizeof(_Ty) << " " << _Get_size_of_n<sizeof(_Ty)>(_Count) = 16 16
        // _Allocate<>() 最终 执行::operator new(_Bytes);
        cout << typeid(_Ty).name() << endl;
        cout << _New_alignof<_Ty> << endl;
        cout << sizeof(_Ty) << " " << _Get_size_of_n<sizeof(_Ty)>(_Count) << endl;
        return static_cast<_Ty*>(   _Allocate<_New_alignof<_Ty>>(_Get_size_of_n<sizeof(_Ty)>(_Count)) );
    }

    _CXX17_DEPRECATE_OLD_ALLOCATOR_MEMBERS _NODISCARD __declspec(allocator) _Ty* allocate(
        _CRT_GUARDOVERFLOW const size_t _Count, const void*) {
        return allocate(_Count);
    }

    template <class _Objty, class... _Types>
    _CXX17_DEPRECATE_OLD_ALLOCATOR_MEMBERS void construct(_Objty* const _Ptr, _Types&&... _Args) {
        ::new (const_cast<void*>(static_cast<const volatile void*>(_Ptr))) _Objty(_STD forward<_Types>(_Args)...);
    }

    template <class _Uty>
    _CXX17_DEPRECATE_OLD_ALLOCATOR_MEMBERS void destroy(_Uty* const _Ptr) {
        _Ptr->~_Uty();
    }

    _CXX17_DEPRECATE_OLD_ALLOCATOR_MEMBERS _NODISCARD size_t max_size() const noexcept {
        return static_cast<size_t>(-1) / sizeof(_Ty);
    }
};

// CLASS allocator<void>
template <>
class allocator<void> {
public:
    using value_type = void;
    _CXX17_DEPRECATE_OLD_ALLOCATOR_MEMBERS typedef void* pointer;
    _CXX17_DEPRECATE_OLD_ALLOCATOR_MEMBERS typedef const void* const_pointer;

    using size_type       = size_t;
    using difference_type = ptrdiff_t;

    using propagate_on_container_move_assignment = true_type;
    using is_always_equal                        = true_type;

    template <class _Other>
    struct _CXX17_DEPRECATE_OLD_ALLOCATOR_MEMBERS rebind {
        using other = allocator<_Other>;
    };
};

template <class _Ty, class _Other>
_NODISCARD bool operator==(const allocator<_Ty>&, const allocator<_Other>&) noexcept {
    return true;
}

template <class _Ty, class _Other>
_NODISCARD bool operator!=(const allocator<_Ty>&, const allocator<_Other>&) noexcept {
    return false;
}

#if _HAS_CXX17
// ALIAS TEMPLATE _Guide_size_type_t FOR DEDUCTION GUIDES, N4687 26.5.4.1 [unord.map.overview]/4
template <class _Alloc>
using _Guide_size_type_t =
    typename allocator_traits<conditional_t<_Is_allocator<_Alloc>::value, _Alloc, allocator<int>>>::size_type;
#endif // _HAS_CXX17

// ALIAS TEMPLATE _Alloc_ptr_t
template <class _Alloc>
using _Alloc_ptr_t = typename allocator_traits<_Alloc>::pointer;

// ALIAS_TEMPLATE _Alloc_size_t
template <class _Alloc>
using _Alloc_size_t = typename allocator_traits<_Alloc>::size_type;

// FUNCTION TEMPLATE _Pocca
#if _HAS_IF_CONSTEXPR
template <class _Alloc>
void _Pocca(_Alloc& _Left, const _Alloc& _Right) noexcept {
    if constexpr (allocator_traits<_Alloc>::propagate_on_container_copy_assignment::value) {
        _Left = _Right;
    }
}
#else // ^^^ _HAS_IF_CONSTEXPR ^^^ // vvv !_HAS_IF_CONSTEXPR vvv
template <class _Alloc>
void _Pocca(_Alloc& _Left, const _Alloc& _Right, true_type) noexcept {
    _Left = _Right;
}

template <class _Alloc>
void _Pocca(_Alloc&, const _Alloc&, false_type) noexcept {}

template <class _Alloc>
void _Pocca(_Alloc& _Left, const _Alloc& _Right) noexcept {
    _Pocca(_Left, _Right, typename allocator_traits<_Alloc>::propagate_on_container_copy_assignment{});
}
#endif // _HAS_IF_CONSTEXPR

// FUNCTION TEMPLATE _Pocma
#if _HAS_IF_CONSTEXPR
template <class _Alloc>
void _Pocma(_Alloc& _Left, _Alloc& _Right) noexcept { // (maybe) propagate on container move assignment
    if constexpr (allocator_traits<_Alloc>::propagate_on_container_move_assignment::value) {
        _Left = _STD move(_Right);
    }
}
#else // ^^^ _HAS_IF_CONSTEXPR ^^^ // vvv !_HAS_IF_CONSTEXPR vvv
template <class _Alloc>
void _Pocma(_Alloc& _Left, _Alloc& _Right, true_type) noexcept {
    _Left = _STD move(_Right);
}

template <class _Alloc>
void _Pocma(_Alloc&, _Alloc&, false_type) noexcept {}

template <class _Alloc>
void _Pocma(_Alloc& _Left, _Alloc& _Right) noexcept {
    typename allocator_traits<_Alloc>::propagate_on_container_move_assignment _Tag;
    _Pocma(_Left, _Right, _Tag);
}
#endif // _HAS_IF_CONSTEXPR

// FUNCTION TEMPLATE _Pocs
#if _HAS_IF_CONSTEXPR
template <class _Alloc>
void _Pocs(_Alloc& _Left, _Alloc& _Right) noexcept {
    if constexpr (allocator_traits<_Alloc>::propagate_on_container_swap::value) {
        _Swap_adl(_Left, _Right);
    } else {
        _STL_ASSERT(_Left == _Right, "containers incompatible for swap");
    }
}
#else // ^^^ _HAS_IF_CONSTEXPR ^^^ // vvv !_HAS_IF_CONSTEXPR vvv
template <class _Alloc>
void _Pocs(_Alloc& _Left, _Alloc& _Right, true_type) noexcept {
    _Swap_adl(_Left, _Right);
}

template <class _Alloc>
void _Pocs(_Alloc& _Left, _Alloc& _Right, false_type) noexcept {
    _STL_ASSERT(_Left == _Right, "containers incompatible for swap");
    (void) _Left;
    (void) _Right;
}

template <class _Alloc>
void _Pocs(_Alloc& _Left, _Alloc& _Right) noexcept {
    typename allocator_traits<_Alloc>::propagate_on_container_swap _Tag;
    _Pocs(_Left, _Right, _Tag);
}
#endif // _HAS_IF_CONSTEXPR


// FUNCTION TEMPLATE _Destroy_range WITH ALLOC
template <class _Alloc>
void _Destroy_range(_Alloc_ptr_t<_Alloc> _First, const _Alloc_ptr_t<_Alloc> _Last, _Alloc& _Al) noexcept {
    // note that this is an optimization for debug mode codegen; in release mode the BE removes all of this
    using _Ty = typename _Alloc::value_type;
    if _CONSTEXPR_IF (!conjunction_v<is_trivially_destructible<_Ty>, _Uses_default_destroy<_Alloc, _Ty*>>) {
        for (; _First != _Last; ++_First) {
            allocator_traits<_Alloc>::destroy(_Al, _Unfancy(_First));
        }
    }
}


// FUNCTION TEMPLATE _Destroy_range
template <class _NoThrowFwdIt>
void _Destroy_range(_NoThrowFwdIt _First, const _NoThrowFwdIt _Last) noexcept {
    // note that this is an optimization for debug mode codegen; in release mode the BE removes all of this
    if _CONSTEXPR_IF (!is_trivially_destructible_v<_Iter_value_t<_NoThrowFwdIt>>) {
        for (; _First != _Last; ++_First) {
            _Destroy_in_place(*_First);
        }
    }
}


// FUNCTION TEMPLATE _Convert_size
template <class _Size_type>
_NODISCARD constexpr _Size_type _Convert_size(const size_t _Len) noexcept {
    // convert size_t to _Size_type, avoiding truncation
    if (_Len > (numeric_limits<_Size_type>::max)()) {
        _Xlength_error("size_t too long for _Size_type");
    }

    return static_cast<_Size_type>(_Len);
}

template <>
_NODISCARD constexpr size_t _Convert_size<size_t>(const size_t _Len) noexcept {
    // convert size_t to size_t, unchanged
    return _Len;
}

// FUNCTION TEMPLATE _Deallocate_plain
#if _HAS_IF_CONSTEXPR
template <class _Alloc>
void _Deallocate_plain(_Alloc& _Al, typename _Alloc::value_type* const _Ptr) noexcept {
    // deallocate a plain pointer using an allocator
    using _Alloc_traits = allocator_traits<_Alloc>;
    if constexpr (is_same_v<_Alloc_ptr_t<_Alloc>, typename _Alloc::value_type*>) {
        _Alloc_traits::deallocate(_Al, _Ptr, 1);
    } else {
        using _Ptr_traits = pointer_traits<_Alloc_ptr_t<_Alloc>>;
        _Alloc_traits::deallocate(_Al, _Ptr_traits::pointer_to(*_Ptr), 1);
    }
}
#else // ^^^ _HAS_IF_CONSTEXPR ^^^ // vvv !_HAS_IF_CONSTEXPR vvv
template <class _Alloc, enable_if_t<is_same_v<_Alloc_ptr_t<_Alloc>, typename _Alloc::value_type*>, int> = 0>
void _Deallocate_plain(_Alloc& _Al, typename _Alloc::value_type* const _Ptr) noexcept {
    // deallocate a plain pointer using an allocator, non-fancy pointers special case
    allocator_traits<_Alloc>::deallocate(_Al, _Ptr, 1);
}

template <class _Alloc, enable_if_t<!is_same_v<_Alloc_ptr_t<_Alloc>, typename _Alloc::value_type*>, int> = 0>
void _Deallocate_plain(_Alloc& _Al, typename _Alloc::value_type* const _Ptr) noexcept {
    // deallocate a plain pointer using an allocator
    using _Alloc_traits = allocator_traits<_Alloc>;
    using _Ptr_traits = pointer_traits<_Alloc_ptr_t<_Alloc>>;
    _Alloc_traits::deallocate(_Al, _Ptr_traits::pointer_to(*_Ptr), 1);
}
#endif // _HAS_IF_CONSTEXPR

// FUNCTION TEMPLATE _Delete_plain_internal
template <class _Alloc>
void _Delete_plain_internal(_Alloc& _Al, typename _Alloc::value_type* const _Ptr) noexcept {
    // destroy *_Ptr in place, then deallocate _Ptr using _Al; used for internal container types the user didn't name
    using _Ty = typename _Alloc::value_type;
    _Ptr->~_Ty();
    _Deallocate_plain(_Al, _Ptr);
}

// STRUCT TEMPLATE _Alloc_construct_ptr
template <class _Alloc>
struct _Alloc_construct_ptr { // pointer used to help construct 1 _Alloc::value_type without EH
    using pointer = _Alloc_ptr_t<_Alloc>;
    _Alloc& _Al;
    pointer _Ptr;

    explicit _Alloc_construct_ptr(_Alloc& _Al_) : _Al(_Al_), _Ptr(nullptr) {}

    _NODISCARD pointer _Release() noexcept { // disengage *this and return contained pointer
        return _STD exchange(_Ptr, nullptr);
    }

    void _Allocate() { // disengage *this, then allocate a new memory block
        _Ptr = nullptr; // if allocate throws, prevents double-free
        _Ptr = _Al.allocate(1);
    }

    ~_Alloc_construct_ptr() { // if this instance is engaged, deallocate storage
        if (_Ptr) {
            _Al.deallocate(_Ptr, 1);
        }
    }

    _Alloc_construct_ptr(const _Alloc_construct_ptr&) = delete;
    _Alloc_construct_ptr& operator=(const _Alloc_construct_ptr&) = delete;
};

// CLASSES _Container_base*, _Iterator_base*
struct _Fake_allocator {};

struct _Container_base0 {
    void _Orphan_all() noexcept {}
    void _Swap_proxy_and_iterators(_Container_base0&) noexcept {}
    void _Alloc_proxy(const _Fake_allocator&) noexcept {}
    void _Reload_proxy(const _Fake_allocator&, const _Fake_allocator&) noexcept {}
};

struct _Iterator_base0 {
    void _Adopt(const void*) noexcept {}
    const _Container_base0* _Getcont() const noexcept {
        return nullptr;
    }

    static constexpr bool _Unwrap_when_unverified = true;
};

// CLASS _Container_proxy 容器代理
struct _Container_base12;
struct _Container_proxy { // store head of iterator chain and back pointer 存储迭代器链头和回指针
    _Container_proxy() noexcept : _Mycont(nullptr), _Myfirstiter(nullptr) {}
    _Container_proxy(_Container_base12* _Mycont_) noexcept : _Mycont(_Mycont_), _Myfirstiter(nullptr) {
        printf("run  _Container_proxy(_Container_base12* _Mycont_)");
    }

    const _Container_base12* _Mycont;
    _Iterator_base12* _Myfirstiter;
};

struct _Container_base12 {
public:
    _Container_base12() noexcept : _Myproxy(nullptr) {}

    _Container_base12(const _Container_base12&) = delete;
    _Container_base12& operator=(const _Container_base12&) = delete;

    void _Orphan_all() noexcept;
    void _Swap_proxy_and_iterators(_Container_base12&) noexcept;

    // std::allocator<std::_Container_proxy> &&
    template <class _Alloc>
    void _Alloc_proxy(_Alloc&& _Al) {
        _Container_proxy* const _New_proxy = _Unfancy(_Al.allocate(1));
        _Construct_in_place(*_New_proxy, this);
        _Myproxy            = _New_proxy;
        _New_proxy->_Mycont = this;
    }

    template <class _Alloc>
    void _Reload_proxy(_Alloc&& _Old_alloc, _Alloc&& _New_alloc) {
        // pre: no iterators refer to the existing proxy
        _Container_proxy* const _New_proxy = _Unfancy(_New_alloc.allocate(1));
        _Construct_in_place(*_New_proxy, this);
        _New_proxy->_Mycont = this;
        _Delete_plain_internal(_Old_alloc, _STD exchange(_Myproxy, _New_proxy));
    }

    _Container_proxy* _Myproxy;
};

struct _Iterator_base12 { // store links to container proxy, next iterator
    _Iterator_base12() noexcept : _Myproxy(nullptr), _Mynextiter(nullptr) {} // construct orphaned iterator

    _Iterator_base12(const _Iterator_base12& _Right) noexcept : _Myproxy(nullptr), _Mynextiter(nullptr) {
        *this = _Right;
    }

    _Iterator_base12& operator=(const _Iterator_base12& _Right) noexcept {
        if (_Myproxy != _Right._Myproxy) {
            if (_Right._Myproxy) {
                _Adopt(_Right._Myproxy->_Mycont);
            } else { // becoming invalid, disown current parent
#if _ITERATOR_DEBUG_LEVEL == 2
                _Lockit _Lock(_LOCK_DEBUG);
                _Orphan_me();
#else // _ITERATOR_DEBUG_LEVEL == 2
                _Myproxy = nullptr;
#endif // _ITERATOR_DEBUG_LEVEL == 2
            }
        }

        return *this;
    }

    ~_Iterator_base12() noexcept {
#if _ITERATOR_DEBUG_LEVEL == 2
        _Lockit _Lock(_LOCK_DEBUG);
        _Orphan_me();
#endif // _ITERATOR_DEBUG_LEVEL == 2
    }

    void _Adopt(const _Container_base12* _Parent) noexcept {
        if (_Parent) {
            // have a parent, do adoption
            _Container_proxy* _Parent_proxy = _Parent->_Myproxy;

#if _ITERATOR_DEBUG_LEVEL == 2
            if (_Myproxy != _Parent_proxy) { // change parentage
                _Lockit _Lock(_LOCK_DEBUG);
                _Orphan_me();
                _Mynextiter                 = _Parent_proxy->_Myfirstiter;
                _Parent_proxy->_Myfirstiter = this;
                _Myproxy                    = _Parent_proxy;
            }

#else // _ITERATOR_DEBUG_LEVEL == 2
            _Myproxy = _Parent_proxy;
#endif // _ITERATOR_DEBUG_LEVEL == 2
        } else {
            // no future parent, just disown current parent
#if _ITERATOR_DEBUG_LEVEL == 2
            _Lockit _Lock(_LOCK_DEBUG);
            _Orphan_me();
#else // _ITERATOR_DEBUG_LEVEL == 2
            _Myproxy = nullptr;
#endif // _ITERATOR_DEBUG_LEVEL == 2
        }
    }

    const _Container_base12* _Getcont() const noexcept {
        return _Myproxy ? _Myproxy->_Mycont : nullptr;
    }

#if _ITERATOR_DEBUG_LEVEL == 2
    void _Orphan_me() noexcept {
        if (_Myproxy) { // adopted, remove self from list
            _Iterator_base12** _Pnext = &_Myproxy->_Myfirstiter;
            while (*_Pnext && *_Pnext != this) {
                _Pnext = &(*_Pnext)->_Mynextiter;
            }

            _STL_VERIFY(*_Pnext, "ITERATOR LIST CORRUPTED!");
            *_Pnext  = _Mynextiter;
            _Myproxy = nullptr;
        }
    }
#endif // _ITERATOR_DEBUG_LEVEL == 2

    static constexpr bool _Unwrap_when_unverified = _ITERATOR_DEBUG_LEVEL == 0;

    _Container_proxy* _Myproxy;
    _Iterator_base12* _Mynextiter;
};

// MEMBER FUNCTIONS FOR _Container_base12
inline void _Container_base12::_Orphan_all() noexcept {
#if _ITERATOR_DEBUG_LEVEL == 2
    if (_Myproxy) { // proxy allocated, drain it
        _Lockit _Lock(_LOCK_DEBUG);

        for (auto _Pnext = &_Myproxy->_Myfirstiter; *_Pnext; *_Pnext = (*_Pnext)->_Mynextiter) {
            (*_Pnext)->_Myproxy = nullptr;
        }

        _Myproxy->_Myfirstiter = nullptr;
    }
#endif // _ITERATOR_DEBUG_LEVEL == 2
}

inline void _Container_base12::_Swap_proxy_and_iterators(_Container_base12& _Right) noexcept {
#if _ITERATOR_DEBUG_LEVEL == 2
    _Lockit _Lock(_LOCK_DEBUG);
#endif // _ITERATOR_DEBUG_LEVEL == 2

    _Container_proxy* _Temp = _Myproxy;
    _Myproxy                = _Right._Myproxy;
    _Right._Myproxy         = _Temp;

    if (_Myproxy) {
        _Myproxy->_Mycont = this;
    }

    if (_Right._Myproxy) {
        _Right._Myproxy->_Mycont = &_Right;
    }
}

#if _ITERATOR_DEBUG_LEVEL == 0
using _Container_base = _Container_base0;
using _Iterator_base  = _Iterator_base0;

#else // _ITERATOR_DEBUG_LEVEL == 0
using _Container_base = _Container_base12;
using _Iterator_base = _Iterator_base12;
#endif // _ITERATOR_DEBUG_LEVEL == 0


// ALIAS TEMPLATE _Container_proxy_ptr AND SUPPORTING MACHINERY
struct _Leave_proxy_unbound {
    explicit _Leave_proxy_unbound() = default;
}; // tag to indicate that a proxy is being allocated before it is safe to bind to a _Container_base12

struct _Fake_proxy_ptr_impl { // fake replacement for a container proxy smart pointer when no container proxy is in use
    _Fake_proxy_ptr_impl(const _Fake_proxy_ptr_impl&) = delete;
    _Fake_proxy_ptr_impl& operator=(const _Fake_proxy_ptr_impl&) = delete;
    _Fake_proxy_ptr_impl(const _Fake_allocator&, _Leave_proxy_unbound) noexcept {}
    _Fake_proxy_ptr_impl(const _Fake_allocator&, const _Container_base0&) noexcept {}

    void _Bind(const _Fake_allocator&, _Container_base0*) noexcept {}
    void _Release() noexcept {}
};

struct _Basic_container_proxy_ptr12 {
    // smart pointer components for a _Container_proxy * that don't depend on the allocator
    _Container_proxy* _Ptr;

    void _Release() noexcept { // disengage this _Basic_container_proxy_ptr12
        _Ptr = nullptr;
    }

protected:
    _Basic_container_proxy_ptr12()                                    = default;
    _Basic_container_proxy_ptr12(const _Basic_container_proxy_ptr12&) = delete;
    _Basic_container_proxy_ptr12(_Basic_container_proxy_ptr12&&)      = delete;
};

template <class _Alloc>
struct _Container_proxy_ptr12 : _Basic_container_proxy_ptr12 {
    // smart pointer components for a _Container_proxy * for an allocator family
    _Alloc& _Al;

    _Container_proxy_ptr12(_Alloc& _Al_, _Leave_proxy_unbound) : _Al(_Al_) { // create a new unbound _Container_proxy
        _Ptr = _Unfancy(_Al_.allocate(1));
        _Construct_in_place(*_Ptr);
    }

    _Container_proxy_ptr12(_Alloc& _Al_, _Container_base12& _Mycont)
        : _Al(_Al_) { // create a new _Container_proxy pointing at _Mycont 创建一个新的指向_Mycont的_Container_proxy
        //>>>> list<int>
        // allocator<_Container_proxy>.allocate(1)
        // _Container_proxy* _Ptr;
        _Ptr = _Unfancy(_Al_.allocate(1));                          // 申请1个_Container_proxy大小的内存
        // _Ptr = _Container_proxy * __ptr64
        // *_Ptr = _Container_proxy
        // _Mycont = _Container_base12
        // addressof(_Mycont) = _Container_base12 * __ptr64
        //cout << typeid(_Ptr).name() << endl;
        //cout << typeid(*_Ptr).name() << endl;
        //cout << typeid(_Mycont).name() << endl;
        //cout << typeid(addressof(_Mycont)).name() << endl;
        // 类型 _Construct_in_place(_Container_proxy, _Container_base12 * __ptr64)
        _Construct_in_place(*_Ptr, _STD addressof(_Mycont));        // 构造函数
        _Mycont._Myproxy = _Ptr;
    }

    void _Bind(_Alloc& _Old_alloc, _Container_base12* _Mycont) noexcept {
        // Attach the proxy stored in *this to _Mycont, and destroy _Mycont's existing proxy
        // with _Old_alloc. Requires that no iterators are alive referring to _Mycont.
        _Ptr->_Mycont = _Mycont;
        _Delete_plain_internal(_Old_alloc, _STD exchange(_Mycont->_Myproxy, _STD exchange(_Ptr, nullptr)));
    }

    ~_Container_proxy_ptr12() {
        if (_Ptr) {
            _Delete_plain_internal(_Al, _Ptr);
        }
    }
};

#if _ITERATOR_DEBUG_LEVEL == 0
#define _GET_PROXY_ALLOCATOR(_Alty, _Al) _Fake_allocator()
template <class _Alloc>
using _Container_proxy_ptr = _Fake_proxy_ptr_impl;
#else // _ITERATOR_DEBUG_LEVEL == 0
// static_cast <type-id> (expression)  将expression转换为type-id类型
// static_cast<_Rebind_alloc_t<allocator<int>, _Container_proxy>>(allocator<int>)     _Getal():_Alty
#define _GET_PROXY_ALLOCATOR(_Alty, _Al) static_cast<_Rebind_alloc_t<_Alty, _Container_proxy>>(_Al)

// _Container_proxy_ptr12< allocator<_Container_proxy> >
template <class _Alloc>
using _Container_proxy_ptr = _Container_proxy_ptr12<_Rebind_alloc_t<_Alloc, _Container_proxy>>;
#endif // _ITERATOR_DEBUG_LEVEL == 0

// CLASS TEMPLATE _Compressed_pair
// explicit 显式构造
struct _Zero_then_variadic_args_t {
    explicit _Zero_then_variadic_args_t() = default;
}; // tag type for value-initializing first, constructing second from remaining args

struct _One_then_variadic_args_t {
    explicit _One_then_variadic_args_t() = default;
}; // tag type for constructing first from one arg, constructing second from remaining args

// <std::allocator<int>, std::_Vector_val<std::_Simple_types<int>>, 1>
template <class _Ty1, class _Ty2, bool = is_empty_v<_Ty1> && !is_final_v<_Ty1>>
class _Compressed_pair final : private _Ty1 { // store a pair of values, deriving from empty first
public:
    _Ty2 _Myval2;   // std::_Vector_val<std::_Simple_types<int>>


    using _Mybase = _Ty1; // for visualization

    // constexpr 修饰构造函数，类可以是常量类
    // explicit 显式调用
    // noexcept 不抛异常
    template <class... _Other2>
    constexpr explicit _Compressed_pair(_Zero_then_variadic_args_t, _Other2&&... _Val2)
        // conjunction_v 模板参数都为true_type，其值为true
        noexcept(conjunction_v<is_nothrow_default_constructible<_Ty1>,
                               is_nothrow_constructible<_Ty2, _Other2...>>)
        // _Ty1() 派生类调用基类构造函数
        // std::forward 完美转发
        : _Ty1(), _Myval2(_STD forward<_Other2>(_Val2)...) {}

    template <class _Other1, class... _Other2>
    constexpr _Compressed_pair(_One_then_variadic_args_t, _Other1&& _Val1, _Other2&&... _Val2) noexcept(
        conjunction_v<is_nothrow_constructible<_Ty1, _Other1>, is_nothrow_constructible<_Ty2, _Other2...>>)
        : _Ty1(_STD forward<_Other1>(_Val1)), _Myval2(_STD forward<_Other2>(_Val2)...) {}

    constexpr _Ty1& _Get_first() noexcept {
        return *this;
    }

    constexpr const _Ty1& _Get_first() const noexcept {
        return *this;
    }
};

template <class _Ty1, class _Ty2>
class _Compressed_pair<_Ty1, _Ty2, false> final { // store a pair of values, not deriving from first
public:
    _Ty1 _Myval1;
    _Ty2 _Myval2;

    template <class... _Other2>
    constexpr explicit _Compressed_pair(_Zero_then_variadic_args_t, _Other2&&... _Val2) noexcept(
        conjunction_v<is_nothrow_default_constructible<_Ty1>, is_nothrow_constructible<_Ty2, _Other2...>>)
        : _Myval1(), _Myval2(_STD forward<_Other2>(_Val2)...) {}

    template <class _Other1, class... _Other2>
    constexpr _Compressed_pair(_One_then_variadic_args_t, _Other1&& _Val1, _Other2&&... _Val2) noexcept(
        conjunction_v<is_nothrow_constructible<_Ty1, _Other1>, is_nothrow_constructible<_Ty2, _Other2...>>)
        : _Myval1(_STD forward<_Other1>(_Val1)), _Myval2(_STD forward<_Other2>(_Val2)...) {}

    constexpr _Ty1& _Get_first() noexcept {
        return _Myval1;
    }

    constexpr const _Ty1& _Get_first() const noexcept {
        return _Myval1;
    }
};

struct _Move_allocator_tag {
    explicit _Move_allocator_tag() = default;
};

// FUNCTION TEMPLATE _Get_temporary_buffer
template <class _Ty>
pair<_Ty*, ptrdiff_t> _Get_temporary_buffer(ptrdiff_t _Count) noexcept {
    if (static_cast<size_t>(_Count) <= static_cast<size_t>(-1) / sizeof(_Ty)) {
        for (; 0 < _Count; _Count /= 2) {
            const auto _Size = static_cast<size_t>(_Count) * sizeof(_Ty);
            void* _Pbuf;
#ifdef __cpp_aligned_new
            if constexpr (alignof(_Ty) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
                _Pbuf = ::operator new (_Size, align_val_t{alignof(_Ty)}, nothrow);
            } else
#endif // __cpp_aligned_new
            {
                _Pbuf = ::operator new(_Size, nothrow);
            }

            if (_Pbuf) {
                return {static_cast<_Ty*>(_Pbuf), _Count};
            }
        }
    }

    return {nullptr, 0};
}

// FUNCTION TEMPLATE _Return_temporary_buffer
template <class _Ty>
void _Return_temporary_buffer(_Ty* const _Pbuf) noexcept {
#ifdef __cpp_aligned_new
    if constexpr (alignof(_Ty) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
        ::operator delete (_Pbuf, align_val_t{alignof(_Ty)});
    } else
#endif // __cpp_aligned_new
    {
        ::operator delete(_Pbuf);
    }
}

// STRUCT TEMPLATE _Uninitialized_backout
template <class _NoThrowFwdIt>
struct _Uninitialized_backout { // struct to undo partially constructed ranges in _Uninitialized_xxx algorithms
    _NoThrowFwdIt _First;
    _NoThrowFwdIt _Last;

    explicit _Uninitialized_backout(_NoThrowFwdIt _Dest) : _First(_Dest), _Last(_Dest) {}

    _Uninitialized_backout(_NoThrowFwdIt _First_, _NoThrowFwdIt _Last_) : _First(_First_), _Last(_Last_) {}

    _Uninitialized_backout(const _Uninitialized_backout&) = delete;
    _Uninitialized_backout& operator=(const _Uninitialized_backout&) = delete;

    ~_Uninitialized_backout() {
        _Destroy_range(_First, _Last);
    }

    template <class... _Types>
    void _Emplace_back(_Types&&... _Vals) { // construct a new element at *_Last and increment
        _Construct_in_place(*_Last, _STD forward<_Types>(_Vals)...);
        ++_Last;
    }

    _NoThrowFwdIt _Release() { // suppress any exception handling backout and return _Last
        _First = _Last;
        return _Last;
    }
};

// FUNCTION TEMPLATE _Uninitialized_move_unchecked
#if _HAS_IF_CONSTEXPR
template <class _InIt, class _NoThrowFwdIt>
_NoThrowFwdIt _Uninitialized_move_unchecked(_InIt _First, const _InIt _Last, _NoThrowFwdIt _Dest) {
    // move [_First, _Last) to raw [_Dest, ...)
    if constexpr (_Ptr_move_cat<_InIt, _NoThrowFwdIt>::_Really_trivial) {
        return _Copy_memmove(_First, _Last, _Dest);
    } else {
        _Uninitialized_backout<_NoThrowFwdIt> _Backout{_Dest};
        for (; _First != _Last; ++_First) {
            _Backout._Emplace_back(_STD move(*_First));
        }

        return _Backout._Release();
    }
}
#else // ^^^ _HAS_IF_CONSTEXPR ^^^ // vvv !_HAS_IF_CONSTEXPR vvv
template <class _InIt, class _NoThrowFwdIt>
_NoThrowFwdIt _Uninitialized_move_unchecked1(_InIt _First, const _InIt _Last, const _NoThrowFwdIt _Dest, false_type) {
    // move [_First, _Last) to raw [_Dest, ...), no special optimization
    _Uninitialized_backout<_NoThrowFwdIt> _Backout{_Dest};
    for (; _First != _Last; ++_First) {
        _Backout._Emplace_back(_STD move(*_First));
    }

    return _Backout._Release();
}

template <class _InIt, class _NoThrowFwdIt>
_NoThrowFwdIt _Uninitialized_move_unchecked1(
    const _InIt _First, const _InIt _Last, const _NoThrowFwdIt _Dest, true_type) {
    // move [_First, _Last) to raw [_Dest, ...), memmove optimization
    return _Copy_memmove(_First, _Last, _Dest);
}

template <class _InIt, class _NoThrowFwdIt>
_NoThrowFwdIt _Uninitialized_move_unchecked(_InIt _First, const _InIt _Last, _NoThrowFwdIt _Dest) {
    // move [_First, _Last) to raw [_Dest, ...), choose optimization
    return _Uninitialized_move_unchecked1(
        _First, _Last, _Dest, bool_constant<_Ptr_move_cat<_InIt, _NoThrowFwdIt>::_Really_trivial>{});
}
#endif // _HAS_IF_CONSTEXPR

// STRUCT TEMPLATE _Uninitialized_backout_al
template <class _Alloc>
class _Uninitialized_backout_al { // struct to undo partially constructed ranges in _Uninitialized_xxx_al algorithms
    using pointer = _Alloc_ptr_t<_Alloc>;

public:
    _Uninitialized_backout_al(pointer _Dest, _Alloc& _Al_) : _First(_Dest), _Last(_Dest), _Al(_Al_) {}

    _Uninitialized_backout_al(const _Uninitialized_backout_al&) = delete;
    _Uninitialized_backout_al& operator=(const _Uninitialized_backout_al&) = delete;

    ~_Uninitialized_backout_al() {
        _Destroy_range(_First, _Last, _Al);
    }

    template <class... _Types>
    void _Emplace_back(_Types&&... _Vals) { // construct a new element at *_Last and increment
        allocator_traits<_Alloc>::construct(_Al, _Unfancy(_Last), _STD forward<_Types>(_Vals)...);
        ++_Last;
    }

    pointer _Release() { // suppress any exception handling backout and return _Last
        _First = _Last;
        return _Last;
    }

private:
    pointer _First;
    pointer _Last;
    _Alloc& _Al;
};

// FUNCTION TEMPLATE _Uninitialized_copy WITH ALLOCATOR
#if _HAS_IF_CONSTEXPR
template <class _InIt, class _Alloc>
_Alloc_ptr_t<_Alloc> _Uninitialized_copy(
    const _InIt _First, const _InIt _Last, _Alloc_ptr_t<_Alloc> _Dest, _Alloc& _Al) {
    // copy [_First, _Last) to raw _Dest, using _Al
    // note: only called internally from elsewhere in the STL
    using _Ptrval = typename _Alloc::value_type*;

    auto _UFirst      = _Get_unwrapped(_First);
    const auto _ULast = _Get_unwrapped(_Last);

    if constexpr (conjunction_v<bool_constant<_Ptr_copy_cat<decltype(_UFirst), _Ptrval>::_Really_trivial>,
                      _Uses_default_construct<_Alloc, _Ptrval, decltype(*_UFirst)>>) {
        _Copy_memmove(_UFirst, _ULast, _Unfancy(_Dest));
        _Dest += _ULast - _UFirst;
    } else {
        _Uninitialized_backout_al<_Alloc> _Backout{_Dest, _Al};
        for (; _UFirst != _ULast; ++_UFirst) {
            _Backout._Emplace_back(*_UFirst);
        }

        _Dest = _Backout._Release();
    }

    return _Dest;
}
#else // ^^^ _HAS_IF_CONSTEXPR ^^^ // vvv !_HAS_IF_CONSTEXPR vvv
template <class _InIt, class _Alloc>
_Alloc_ptr_t<_Alloc> _Uninitialized_copy_al_unchecked(
    _InIt _First, const _InIt _Last, const _Alloc_ptr_t<_Alloc> _Dest, _Alloc& _Al, false_type) {
    // copy [_First, _Last) to raw _Dest, using _Al, no special optimization
    _Uninitialized_backout_al<_Alloc> _Backout{_Dest, _Al};
    for (; _First != _Last; ++_First) {
        _Backout._Emplace_back(*_First);
    }

    return _Backout._Release();
}

template <class _InIt, class _Alloc>
_Alloc_ptr_t<_Alloc> _Uninitialized_copy_al_unchecked(
    const _InIt _First, const _InIt _Last, const _Alloc_ptr_t<_Alloc> _Dest, _Alloc&, true_type) {
    // copy [_First, _Last) to raw _Dest, using default _Alloc construct, memmove optimization
    return _Copy_memmove(_First, _Last, _Unfancy(_Dest));
}

template <class _InIt, class _Alloc>
_Alloc_ptr_t<_Alloc> _Uninitialized_copy(
    const _InIt _First, const _InIt _Last, _Alloc_ptr_t<_Alloc> _Dest, _Alloc& _Al) {
    // copy [_First, _Last) to raw _Dest, using _Al
    // note: only called internally from elsewhere in the STL

    // clang-format off
    auto _UFirst      = _Get_unwrapped(_First);
    const auto _ULast = _Get_unwrapped(_Last);
    // clang-format on

    using _Ptrval = typename _Alloc::value_type*;
    return _Uninitialized_copy_al_unchecked(_UFirst, _ULast, _Dest, _Al,
        bool_constant<conjunction_v<bool_constant<_Ptr_copy_cat<decltype(_UFirst), _Ptrval>::_Really_trivial>,
            _Uses_default_construct<_Alloc, _Ptrval, decltype(*_UFirst)>>>{});
}
#endif // _HAS_IF_CONSTEXPR

// FUNCTION TEMPLATE uninitialized_copy
#if _HAS_IF_CONSTEXPR
template <class _InIt, class _NoThrowFwdIt>
_NoThrowFwdIt uninitialized_copy(const _InIt _First, const _InIt _Last, _NoThrowFwdIt _Dest) {
    // copy [_First, _Last) to raw [_Dest, ...)
    _Adl_verify_range(_First, _Last);
    auto _UFirst      = _Get_unwrapped(_First);
    const auto _ULast = _Get_unwrapped(_Last);
    auto _UDest       = _Get_unwrapped_n(_Dest, _Idl_distance<_InIt>(_UFirst, _ULast));
    if constexpr (_Ptr_copy_cat<decltype(_UFirst), decltype(_UDest)>::_Really_trivial) {
        _UDest = _Copy_memmove(_UFirst, _ULast, _UDest);
    } else {
        _Uninitialized_backout<decltype(_UDest)> _Backout{_UDest};
        for (; _UFirst != _ULast; ++_UFirst) {
            _Backout._Emplace_back(*_UFirst);
        }

        _UDest = _Backout._Release();
    }

    _Seek_wrapped(_Dest, _UDest);
    return _Dest;
}
#else // ^^^ _HAS_IF_CONSTEXPR / !_HAS_IF_CONSTEXPR vvv
template <class _InIt, class _NoThrowFwdIt>
_NoThrowFwdIt _Uninitialized_copy_unchecked(_InIt _First, const _InIt _Last, const _NoThrowFwdIt _Dest, false_type) {
    // copy [_First, _Last) to raw [_Dest, ...), no special optimization
    _Uninitialized_backout<_NoThrowFwdIt> _Backout{_Dest};
    for (; _First != _Last; ++_First) {
        _Backout._Emplace_back(*_First);
    }

    return _Backout._Release();
}

template <class _InIt, class _NoThrowFwdIt>
_NoThrowFwdIt _Uninitialized_copy_unchecked(
    const _InIt _First, const _InIt _Last, const _NoThrowFwdIt _Dest, true_type) {
    // copy [_First, _Last) to raw [_Dest, ...), memmove optimization
    return _Copy_memmove(_First, _Last, _Dest);
}

template <class _InIt, class _NoThrowFwdIt>
_NoThrowFwdIt uninitialized_copy(const _InIt _First, const _InIt _Last, _NoThrowFwdIt _Dest) {
    // copy [_First, _Last) to raw [_Dest, ...)
    _Adl_verify_range(_First, _Last);
    auto _UFirst = _Get_unwrapped(_First);
    const auto _ULast = _Get_unwrapped(_Last);
    auto _UDest = _Get_unwrapped_n(_Dest, _Idl_distance<_InIt>(_UFirst, _ULast));
    _Seek_wrapped(_Dest, _Uninitialized_copy_unchecked(_UFirst, _ULast, _UDest,
                             bool_constant<_Ptr_copy_cat<decltype(_UFirst), decltype(_UDest)>::_Really_trivial>{}));
    return _Dest;
}
#endif // _HAS_IF_CONSTEXPR

// FUNCTION TEMPLATE _Uninitialized_move WITH ALLOCATOR
#if _HAS_IF_CONSTEXPR
template <class _InIt, class _Alloc>
_Alloc_ptr_t<_Alloc> _Uninitialized_move(
    const _InIt _First, const _InIt _Last, _Alloc_ptr_t<_Alloc> _Dest, _Alloc& _Al) {
    // move [_First, _Last) to raw _Dest, using _Al
    // note: only called internally from elsewhere in the STL
    using _Ptrval     = typename _Alloc::value_type*;
    auto _UFirst      = _Get_unwrapped(_First);
    const auto _ULast = _Get_unwrapped(_Last);
    if constexpr (conjunction_v<bool_constant<_Ptr_move_cat<decltype(_UFirst), _Ptrval>::_Really_trivial>,
                      _Uses_default_construct<_Alloc, _Ptrval, decltype(_STD move(*_UFirst))>>) {
        _Copy_memmove(_UFirst, _ULast, _Unfancy(_Dest));
        return _Dest + (_ULast - _UFirst);
    } else {
        _Uninitialized_backout_al<_Alloc> _Backout{_Dest, _Al};
        for (; _UFirst != _ULast; ++_UFirst) {
            _Backout._Emplace_back(_STD move(*_UFirst));
        }

        return _Backout._Release();
    }
}
#else // ^^^ _HAS_IF_CONSTEXPR ^^^ // vvv !_HAS_IF_CONSTEXPR vvv
template <class _InIt, class _Alloc>
_Alloc_ptr_t<_Alloc> _Uninitialized_move_al_unchecked(
    _InIt _First, const _InIt _Last, _Alloc_ptr_t<_Alloc> _Dest, _Alloc& _Al, false_type) {
    // move [_First, _Last) to raw _Dest, using _Al, no special optimization
    _Uninitialized_backout_al<_Alloc> _Backout{_Dest, _Al};
    for (; _First != _Last; ++_First) {
        _Backout._Emplace_back(_STD move(*_First));
    }

    return _Backout._Release();
}

template <class _InIt, class _Alloc>
_Alloc_ptr_t<_Alloc> _Uninitialized_move_al_unchecked(
    const _InIt _First, const _InIt _Last, _Alloc_ptr_t<_Alloc> _Dest, _Alloc&, true_type) {
    // move [_First, _Last) to raw _Dest, using default _Alloc construct, memmove optimization
    _Copy_memmove(_First, _Last, _Unfancy(_Dest));
    return _Dest + (_Last - _First);
}

template <class _InIt, class _Alloc>
_Alloc_ptr_t<_Alloc> _Uninitialized_move(
    const _InIt _First, const _InIt _Last, _Alloc_ptr_t<_Alloc> _Dest, _Alloc& _Al) {
    // move [_First, _Last) to raw _Dest, using _Al
    // note: only called internally from elsewhere in the STL

    // clang-format off
    auto _UFirst      = _Get_unwrapped(_First);
    const auto _ULast = _Get_unwrapped(_Last);
    auto _UDest       = _Get_unwrapped_n(_Dest, _Idl_distance<_InIt>(_UFirst, _ULast));
    // clang-format on

    using _Ptrval = typename _Alloc::value_type*;
    _Seek_wrapped(
        _Dest, _Uninitialized_move_al_unchecked(_UFirst, _ULast, _UDest, _Al,
                   bool_constant < _Ptr_move_cat<decltype(_UFirst), _Ptrval>::_Really_trivial
                       && _Uses_default_construct<_Alloc, _Ptrval, decltype(_STD move(*_UFirst))>::value > {}));
    return _Dest;
}
#endif // _HAS_IF_CONSTEXPR

// FUNCTION TEMPLATE _Uninitialized_fill_n WITH ALLOCATOR
#if _HAS_IF_CONSTEXPR
template <class _Alloc>
_Alloc_ptr_t<_Alloc> _Uninitialized_fill_n(
    _Alloc_ptr_t<_Alloc> _First, _Alloc_size_t<_Alloc> _Count, const typename _Alloc::value_type& _Val, _Alloc& _Al) {
    // copy _Count copies of _Val to raw _First, using _Al
    using _Ty = typename _Alloc::value_type;
    if constexpr (_Fill_memset_is_safe<_Ty*, _Ty> && _Uses_default_construct<_Alloc, _Ty*, _Ty>::value) {
        _CSTD memset(_Unfancy(_First), static_cast<unsigned char>(_Val), static_cast<size_t>(_Count));
        return _First + _Count;
    } else {
        _Uninitialized_backout_al<_Alloc> _Backout{_First, _Al};
        for (; 0 < _Count; --_Count) {
            _Backout._Emplace_back(_Val);
        }

        return _Backout._Release();
    }
}
#else // ^^^ _HAS_IF_CONSTEXPR // !_HAS_IF_CONSTEXPR vvv
template <class _Alloc>
_Alloc_ptr_t<_Alloc> _Uninit_alloc_fill_n1(_Alloc_ptr_t<_Alloc> _First, _Alloc_size_t<_Alloc> _Count,
    const typename _Alloc::value_type& _Val, _Alloc& _Al, false_type) {
    // copy _Count copies of _Val to raw _First, using _Al, no special optimization
    _Uninitialized_backout_al<_Alloc> _Backout{_First, _Al};
    for (; 0 < _Count; --_Count) {
        _Backout._Emplace_back(_Val);
    }

    return _Backout._Release();
}

template <class _Alloc>
_Alloc_ptr_t<_Alloc> _Uninit_alloc_fill_n1(_Alloc_ptr_t<_Alloc> _First, _Alloc_size_t<_Alloc> _Count,
    const typename _Alloc::value_type& _Val, _Alloc&, true_type) {
    // copy _Count copies of _Val to raw _First, using default _Alloc construct, memset optimization
    _CSTD memset(_Unfancy(_First), static_cast<unsigned char>(_Val), _Count);
    return _First + _Count;
}

template <class _Alloc>
_Alloc_ptr_t<_Alloc> _Uninitialized_fill_n(_Alloc_ptr_t<_Alloc> _First, const _Alloc_size_t<_Alloc> _Count,
    const typename _Alloc::value_type& _Val, _Alloc& _Al) {
    // copy _Count copies of _Val to raw _First, using _Al
    using _Ty = typename _Alloc::value_type;
    return _Uninit_alloc_fill_n1(_First, _Count, _Val, _Al,
        bool_constant < _Fill_memset_is_safe<_Ty*, _Ty> && _Uses_default_construct<_Alloc, _Ty*, _Ty>::value > {});
}
#endif // _HAS_IF_CONSTEXPR

// FUNCTION TEMPLATE uninitialized_fill
#if _HAS_IF_CONSTEXPR
template <class _NoThrowFwdIt, class _Tval>
void uninitialized_fill(const _NoThrowFwdIt _First, const _NoThrowFwdIt _Last, const _Tval& _Val) {
    // copy _Val throughout raw [_First, _Last)
    _Adl_verify_range(_First, _Last);
    auto _UFirst      = _Get_unwrapped(_First);
    const auto _ULast = _Get_unwrapped(_Last);
    if constexpr (_Fill_memset_is_safe<_Unwrapped_t<const _NoThrowFwdIt&>, _Tval>) {
        _CSTD memset(_UFirst, static_cast<unsigned char>(_Val), static_cast<size_t>(_ULast - _UFirst));
    } else {
        _Uninitialized_backout<_Unwrapped_t<const _NoThrowFwdIt&>> _Backout{_UFirst};
        while (_Backout._Last != _ULast) {
            _Backout._Emplace_back(_Val);
        }

        _Backout._Release();
    }
}
#else // ^^^ _HAS_IF_CONSTEXPR // !_HAS_IF_CONSTEXPR vvv
template <class _NoThrowFwdIt, class _Tval>
void _Uninitialized_fill_unchecked(
    const _NoThrowFwdIt _First, const _NoThrowFwdIt _Last, const _Tval& _Val, false_type) {
    // copy _Val throughout raw [_First, _Last), no special optimization
    _Uninitialized_backout<_NoThrowFwdIt> _Backout{_First};
    while (_Backout._Last != _Last) {
        _Backout._Emplace_back(_Val);
    }

    _Backout._Release();
}

template <class _NoThrowFwdIt, class _Tval>
void _Uninitialized_fill_unchecked(
    const _NoThrowFwdIt _First, const _NoThrowFwdIt _Last, const _Tval& _Val, true_type) {
    // copy _Val throughout raw [_First, _Last), memset optimization
    _CSTD memset(_First, static_cast<unsigned char>(_Val), static_cast<size_t>(_Last - _First));
}

template <class _NoThrowFwdIt, class _Tval>
void uninitialized_fill(const _NoThrowFwdIt _First, const _NoThrowFwdIt _Last, const _Tval& _Val) {
    // copy _Val throughout raw [_First, _Last)
    _Adl_verify_range(_First, _Last);
    const auto _UFirst = _Get_unwrapped(_First);
    _Uninitialized_fill_unchecked(_UFirst, _Get_unwrapped(_Last), _Val,
        bool_constant<_Fill_memset_is_safe<_Unwrapped_t<const _NoThrowFwdIt&>, _Tval>>{});
}
#endif // _HAS_IF_CONSTEXPR

// FUNCTION TEMPLATE _Uninitialized_value_construct_n WITH ALLOCATOR
template <class _NoThrowFwdIt>
_INLINE_VAR constexpr bool _Use_memset_value_construct_v = conjunction_v<is_pointer<_NoThrowFwdIt>,
    is_scalar<_Iter_value_t<_NoThrowFwdIt>>, negation<is_volatile<remove_reference_t<_Iter_ref_t<_NoThrowFwdIt>>>>,
    negation<is_member_pointer<_Iter_value_t<_NoThrowFwdIt>>>>;

template <class _Ptr>
_Ptr _Zero_range(const _Ptr _First, const _Ptr _Last) { // fill [_First, _Last) with zeroes
    char* const _First_ch = reinterpret_cast<char*>(_First);
    char* const _Last_ch  = reinterpret_cast<char*>(_Last);
    _CSTD memset(_First_ch, 0, static_cast<size_t>(_Last_ch - _First_ch));
    return _Last;
}

#if _HAS_IF_CONSTEXPR
template <class _Alloc>
_Alloc_ptr_t<_Alloc> _Uninitialized_value_construct_n(
    _Alloc_ptr_t<_Alloc> _First, _Alloc_size_t<_Alloc> _Count, _Alloc& _Al) {
    // value-initialize _Count objects to raw _First, using _Al
    using _Ptrty = typename _Alloc::value_type*;
    if constexpr (_Use_memset_value_construct_v<_Ptrty> && _Uses_default_construct<_Alloc, _Ptrty>::value) {
        auto _PFirst = _Unfancy(_First);
        _Zero_range(_PFirst, _PFirst + _Count);
        return _First + _Count;
    } else {
        _Uninitialized_backout_al<_Alloc> _Backout{_First, _Al};
        for (; 0 < _Count; --_Count) {
            _Backout._Emplace_back();
        }

        return _Backout._Release();
    }
}
#else // ^^^ _HAS_IF_CONSTEXPR // !_HAS_IF_CONSTEXPR vvv
template <class _Alloc>
_Alloc_ptr_t<_Alloc> _Uninitialized_value_construct_n1(
    _Alloc_ptr_t<_Alloc> _First, _Alloc_size_t<_Alloc> _Count, _Alloc& _Al, false_type) {
    // value-initialize _Count objects to raw _First, using _Al, no special optimization
    _Uninitialized_backout_al<_Alloc> _Backout{_First, _Al};
    for (; 0 < _Count; --_Count) {
        _Backout._Emplace_back();
    }

    return _Backout._Release();
}

template <class _Alloc>
_Alloc_ptr_t<_Alloc> _Uninitialized_value_construct_n1(
    _Alloc_ptr_t<_Alloc> _First, _Alloc_size_t<_Alloc> _Count, _Alloc&, true_type) {
    // value-initialize _Count objects to raw _First, using default _Alloc construct, all-bits-zero type
    auto _PFirst = _Unfancy(_First);
    _Zero_range(_PFirst, _PFirst + _Count);
    return _First + _Count;
}

template <class _Alloc>
_Alloc_ptr_t<_Alloc> _Uninitialized_value_construct_n(
    _Alloc_ptr_t<_Alloc> _First, _Alloc_size_t<_Alloc> _Count, _Alloc& _Al) {
    // value-initialize _Count objects to raw _First, using _Al
    using _Ptrty = typename _Alloc::value_type*;
    return _Uninitialized_value_construct_n1(_First, _Count, _Al,
        bool_constant < _Use_memset_value_construct_v<_Ptrty> && _Uses_default_construct<_Alloc, _Ptrty>::value > {});
}
#endif // _HAS_IF_CONSTEXPR

#if _HAS_IF_CONSTEXPR
template <class _NoThrowFwdIt, class _Diff>
_NoThrowFwdIt _Uninitialized_value_construct_n_unchecked1(_NoThrowFwdIt _UFirst, _Diff _Count) {
    // value-initialize all elements in [_UFirst, _UFirst + _Count_raw)
    _STL_INTERNAL_CHECK(_Count >= 0);
    if constexpr (_Use_memset_value_construct_v<_NoThrowFwdIt>) {
        return _Zero_range(_UFirst, _UFirst + _Count);
    } else {
        _Uninitialized_backout<_NoThrowFwdIt> _Backout{_UFirst};
        for (; 0 < _Count; --_Count) {
            _Backout._Emplace_back();
        }

        return _Backout._Release();
    }
}
#else // ^^^ _HAS_IF_CONSTEXPR // !_HAS_IF_CONSTEXPR vvv
template <class _NoThrowFwdIt, class _Diff>
_NoThrowFwdIt _Uninitialized_value_construct_n_unchecked2(_NoThrowFwdIt _UFirst, _Diff _Count, false_type) {
    _Uninitialized_backout<_NoThrowFwdIt> _Backout{_UFirst};
    for (; 0 < _Count; --_Count) {
        _Backout._Emplace_back();
    }

    return _Backout._Release();
}
template <class _NoThrowFwdIt, class _Diff>
_NoThrowFwdIt _Uninitialized_value_construct_n_unchecked2(_NoThrowFwdIt _UFirst, _Diff _Count, true_type) {
    return _Zero_range(_UFirst, _UFirst + _Count);
}
template <class _NoThrowFwdIt, class _Diff>
_NoThrowFwdIt _Uninitialized_value_construct_n_unchecked1(_NoThrowFwdIt _UFirst, _Diff _Count) {
    return _Uninitialized_value_construct_n_unchecked2(
        _UFirst, _Count, bool_constant<_Use_memset_value_construct_v<_NoThrowFwdIt>>{});
}
#endif // _HAS_IF_CONSTEXPR

#if _HAS_DEPRECATED_TEMPORARY_BUFFER
// FUNCTION TEMPLATE get_temporary_buffer
template <class _Ty>
_CXX17_DEPRECATE_TEMPORARY_BUFFER _NODISCARD pair<_Ty*, ptrdiff_t> get_temporary_buffer(ptrdiff_t _Count) noexcept {
    return _Get_temporary_buffer<_Ty>(_Count);
}

// FUNCTION TEMPLATE return_temporary_buffer
template <class _Ty>
_CXX17_DEPRECATE_TEMPORARY_BUFFER void return_temporary_buffer(_Ty* _Pbuf) {
    _Return_temporary_buffer(_Pbuf);
}
#endif // _HAS_DEPRECATED_TEMPORARY_BUFFER

// STRUCT TEMPLATE _In_place_key_extract_set
// assumes _Args have already been _Remove_cvref_t'd
template <class _Key, class... _Args>
struct _In_place_key_extract_set {
    // by default we can't extract the key in the emplace family and must construct a node we might not use
    static constexpr bool _Extractable = false;
};

template <class _Key>
struct _In_place_key_extract_set<_Key, _Key> {
    // we can extract the key in emplace if the emplaced type is identical to the key type
    static constexpr bool _Extractable = true;
    static const _Key& _Extract(const _Key& _Val) noexcept {
        return _Val;
    }
};

// STRUCT TEMPLATE _In_place_key_extract_map
// assumes _Args have already been _Remove_cvref_t'd
template <class _Key, class... Args>
struct _In_place_key_extract_map {
    // by default we can't extract the key in the emplace family and must construct a node we might not use
    static constexpr bool _Extractable = false;
};

template <class _Key, class _Second>
struct _In_place_key_extract_map<_Key, _Key, _Second> {
    // if we would call the pair(key, value) constructor family, we can use the first parameter as the key
    static constexpr bool _Extractable = true;
    static const _Key& _Extract(const _Key& _Val, const _Second&) noexcept {
        return _Val;
    }
};

template <class _Key, class _First, class _Second>
struct _In_place_key_extract_map<_Key, pair<_First, _Second>> {
    // if we would call the pair(pair<other, other>) constructor family, we can use the pair.first member as the key
    static constexpr bool _Extractable = is_same_v<_Key, _Remove_cvref_t<_First>>;
    static const _Key& _Extract(const pair<_First, _Second>& _Val) {
        return _Val.first;
    }
};

// STRUCT TEMPLATE _Wrap
template <class _Ty>
struct _Wrap {
    _Ty _Value; // workaround for "T^ is not allowed in a union"
};

// STRUCT TEMPLATE _Alloc_temporary
template <class _Alloc>
struct _Alloc_temporary {
    using value_type = typename _Alloc::value_type;
    using _Traits    = allocator_traits<_Alloc>;

    _Alloc& _Al;
    union {
        _Wrap<value_type> _Storage;
    };

    template <class... _Args>
    explicit _Alloc_temporary(_Alloc& _Al_, _Args&&... _Vals) noexcept(
        noexcept(_Traits::construct(_Al_, _STD addressof(_Storage._Value), _STD forward<_Args>(_Vals)...)))
        : _Al(_Al_) {
        _Traits::construct(_Al, _STD addressof(_Storage._Value), _STD forward<_Args>(_Vals)...);
    }

    _Alloc_temporary(const _Alloc_temporary&) = delete;
    _Alloc_temporary& operator=(const _Alloc_temporary&) = delete;

    ~_Alloc_temporary() {
        _Traits::destroy(_Al, _STD addressof(_Storage._Value));
    }
};

template <class _Alloc>
_NODISCARD constexpr bool _Allocators_equal(const _Alloc& _Lhs, const _Alloc& _Rhs) noexcept {
    if _CONSTEXPR_IF (allocator_traits<_Alloc>::is_always_equal::value) {
        return true;
    } else {
        return _Lhs == _Rhs;
    }
}

// FUNCTION TEMPLATE remove
template <class _FwdIt, class _Ty>
_NODISCARD _CONSTEXPR20 _FwdIt remove(_FwdIt _First, const _FwdIt _Last, const _Ty& _Val) {
    // remove each matching _Val
    _Adl_verify_range(_First, _Last);
    auto _UFirst      = _Get_unwrapped(_First);
    const auto _ULast = _Get_unwrapped(_Last);
    _UFirst           = _Find_unchecked(_UFirst, _ULast, _Val);
    auto _UNext       = _UFirst;
    if (_UFirst != _ULast) {
        while (++_UFirst != _ULast) {
            if (!(*_UFirst == _Val)) {
                *_UNext = _STD move(*_UFirst);
                ++_UNext;
            }
        }
    }

    _Seek_wrapped(_First, _UNext);
    return _First;
}

// FUNCTION TEMPLATE remove_if
template <class _FwdIt, class _Pr>
_NODISCARD _CONSTEXPR20 _FwdIt remove_if(_FwdIt _First, const _FwdIt _Last, _Pr _Pred) {
    // remove each satisfying _Pred
    _Adl_verify_range(_First, _Last);
    auto _UFirst      = _Get_unwrapped(_First);
    const auto _ULast = _Get_unwrapped(_Last);
    _UFirst           = _STD find_if(_UFirst, _ULast, _Pass_fn(_Pred));
    auto _UNext       = _UFirst;
    if (_UFirst != _ULast) {
        while (++_UFirst != _ULast) {
            if (!_Pred(*_UFirst)) {
                *_UNext = _STD move(*_UFirst);
                ++_UNext;
            }
        }
    }

    _Seek_wrapped(_First, _UNext);
    return _First;
}

// FUNCTION TEMPLATE _Erase_remove
template <class _Container, class _Uty>
typename _Container::size_type _Erase_remove(_Container& _Cont, const _Uty& _Val) { // erase each element matching _Val
    auto _First          = _Cont.begin();
    const auto _Last     = _Cont.end();
    const auto _Old_size = _Cont.size();
    _Seek_wrapped(_First, _STD remove(_Get_unwrapped(_First), _Get_unwrapped(_Last), _Val));
    _Cont.erase(_First, _Last);
    return _Old_size - _Cont.size();
}

// FUNCTION TEMPLATE _Erase_remove_if
template <class _Container, class _Pr>
typename _Container::size_type _Erase_remove_if(_Container& _Cont, _Pr _Pred) { // erase each element satisfying _Pred
    auto _First          = _Cont.begin();
    const auto _Last     = _Cont.end();
    const auto _Old_size = _Cont.size();
    _Seek_wrapped(_First, _STD remove_if(_Get_unwrapped(_First), _Get_unwrapped(_Last), _Pred));
    _Cont.erase(_First, _Last);
    return _Old_size - _Cont.size();
}

// FUNCTION TEMPLATE _Erase_nodes_if
template <class _Container, class _Pr>
typename _Container::size_type _Erase_nodes_if(_Container& _Cont, _Pr _Pred) { // erase each element satisfying _Pred
    auto _First          = _Cont.begin();
    const auto _Last     = _Cont.end();
    const auto _Old_size = _Cont.size();
    while (_First != _Last) {
        if (_Pred(*_First)) {
            _First = _Cont.erase(_First);
        } else {
            ++_First;
        }
    }
    return _Old_size - _Cont.size();
}
_STD_END

#pragma pop_macro("new")
_STL_RESTORE_CLANG_WARNINGS
#pragma warning(pop)
#pragma pack(pop)
#endif // _STL_COMPILER_PREPROCESSOR
#endif // _XMEMORY_
