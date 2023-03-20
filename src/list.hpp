// list standard header

// Copyright (c) Microsoft Corporation.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once
#ifndef _LIST_
#define _LIST_
#include <yvals_core.h>
#include <iostream>
#if _STL_COMPILER_PREPROCESSOR
//#include <xmemory>
#include "xmemory.hpp"

#if _HAS_CXX17
#include <xpolymorphic_allocator.h>
#endif // _HAS_CXX17

#pragma pack(push, _CRT_PACKING)
#pragma warning(push, _STL_WARNING_LEVEL)
#pragma warning(disable : _STL_DISABLED_WARNINGS)
_STL_DISABLE_CLANG_WARNINGS
#pragma push_macro("new")
#undef new

_STD_BEGIN
// CLASS TEMPLATE _List_unchecked_const_iterator
template <class _Mylist, class _Base = _Iterator_base0>
class _List_unchecked_const_iterator : public _Base {
public:
    using iterator_category = bidirectional_iterator_tag;

    using _Nodeptr        = typename _Mylist::_Nodeptr;
    using value_type      = typename _Mylist::value_type;
    using difference_type = typename _Mylist::difference_type;
    using pointer         = typename _Mylist::const_pointer;
    using reference       = const value_type&;

    _List_unchecked_const_iterator() noexcept : _Ptr() {}

    _List_unchecked_const_iterator(_Nodeptr _Pnode, const _Mylist* _Plist) noexcept : _Ptr(_Pnode) {
        this->_Adopt(_Plist);
    }

    _NODISCARD reference operator*() const {
        return _Ptr->_Myval;
    }

    _NODISCARD pointer operator->() const {
        return pointer_traits<pointer>::pointer_to(**this);
    }

    _List_unchecked_const_iterator& operator++() {
        _Ptr = _Ptr->_Next;
        return *this;
    }

    _List_unchecked_const_iterator operator++(int) {
        _List_unchecked_const_iterator _Tmp = *this;
        _Ptr                                = _Ptr->_Next;
        return _Tmp;
    }

    _List_unchecked_const_iterator& operator--() {
        _Ptr = _Ptr->_Prev;
        return *this;
    }

    _List_unchecked_const_iterator operator--(int) {
        _List_unchecked_const_iterator _Tmp = *this;
        _Ptr                                = _Ptr->_Prev;
        return _Tmp;
    }

    _NODISCARD bool operator==(const _List_unchecked_const_iterator& _Right) const {
        return _Ptr == _Right._Ptr;
    }

    _NODISCARD bool operator!=(const _List_unchecked_const_iterator& _Right) const {
        return !(*this == _Right);
    }

    _Nodeptr _Ptr; // pointer to node
};

// CLASS TEMPLATE _List_unchecked_iterator
template <class _Mylist>
class _List_unchecked_iterator : public _List_unchecked_const_iterator<_Mylist> {
public:
    using _Mybase           = _List_unchecked_const_iterator<_Mylist>;
    using iterator_category = bidirectional_iterator_tag;

    using _Nodeptr        = typename _Mylist::_Nodeptr;
    using value_type      = typename _Mylist::value_type;
    using difference_type = typename _Mylist::difference_type;
    using pointer         = typename _Mylist::pointer;
    using reference       = value_type&;

    using _Mybase::_Mybase;

    _NODISCARD reference operator*() const {
        return const_cast<reference>(_Mybase::operator*());
    }

    _NODISCARD pointer operator->() const {
        return pointer_traits<pointer>::pointer_to(**this);
    }

    _List_unchecked_iterator& operator++() {
        _Mybase::operator++();
        return *this;
    }

    _List_unchecked_iterator operator++(int) {
        _List_unchecked_iterator _Tmp = *this;
        _Mybase::operator++();
        return _Tmp;
    }

    _List_unchecked_iterator& operator--() {
        _Mybase::operator--();
        return *this;
    }

    _List_unchecked_iterator operator--(int) {
        _List_unchecked_iterator _Tmp = *this;
        _Mybase::operator--();
        return _Tmp;
    }
};

// CLASS TEMPLATE _List_const_iterator
template <class _Mylist>
class _List_const_iterator : public _List_unchecked_const_iterator<_Mylist, _Iterator_base> {
public:
    using _Mybase           = _List_unchecked_const_iterator<_Mylist, _Iterator_base>;
    using iterator_category = bidirectional_iterator_tag;

    using _Nodeptr        = typename _Mylist::_Nodeptr;
    using value_type      = typename _Mylist::value_type;
    using difference_type = typename _Mylist::difference_type;
    using pointer         = typename _Mylist::const_pointer;
    using reference       = const value_type&;

    using _Mybase::_Mybase;

    _NODISCARD reference operator*() const {
#if _ITERATOR_DEBUG_LEVEL == 2
        const auto _Mycont = static_cast<const _Mylist*>(this->_Getcont());
        _STL_ASSERT(_Mycont, "cannot dereference value-initialized list iterator");
        _STL_VERIFY(this->_Ptr != _Mycont->_Myhead, "cannot dereference end list iterator");
#endif // _ITERATOR_DEBUG_LEVEL == 2

        return this->_Ptr->_Myval;
    }

    _NODISCARD pointer operator->() const {
        return pointer_traits<pointer>::pointer_to(**this);
    }

    _List_const_iterator& operator++() {
#if _ITERATOR_DEBUG_LEVEL == 2
        const auto _Mycont = static_cast<const _Mylist*>(this->_Getcont());
        _STL_ASSERT(_Mycont, "cannot increment value-initialized list iterator");
        _STL_VERIFY(this->_Ptr != _Mycont->_Myhead, "cannot increment end list iterator");
#endif // _ITERATOR_DEBUG_LEVEL == 2

        this->_Ptr = this->_Ptr->_Next;
        return *this;
    }

    _List_const_iterator operator++(int) {
        _List_const_iterator _Tmp = *this;
        ++*this;
        return _Tmp;
    }

    _List_const_iterator& operator--() {
        const auto _New_ptr = this->_Ptr->_Prev;
#if _ITERATOR_DEBUG_LEVEL == 2
        const auto _Mycont = static_cast<const _Mylist*>(this->_Getcont());
        _STL_ASSERT(_Mycont, "cannot decrement value-initialized list iterator");
        _STL_VERIFY(_New_ptr != _Mycont->_Myhead, "cannot decrement begin list iterator");
#endif // _ITERATOR_DEBUG_LEVEL == 2

        this->_Ptr = _New_ptr;
        return *this;
    }

    _List_const_iterator operator--(int) {
        _List_const_iterator _Tmp = *this;
        --*this;
        return _Tmp;
    }

    _NODISCARD bool operator==(const _List_const_iterator& _Right) const {
#if _ITERATOR_DEBUG_LEVEL == 2
        _STL_VERIFY(this->_Getcont() == _Right._Getcont(), "list iterators incompatible");
#endif // _ITERATOR_DEBUG_LEVEL == 2

        return this->_Ptr == _Right._Ptr;
    }

    _NODISCARD bool operator!=(const _List_const_iterator& _Right) const {
        return !(*this == _Right);
    }

#if _ITERATOR_DEBUG_LEVEL == 2
    friend void _Verify_range(const _List_const_iterator& _First, const _List_const_iterator& _Last) {
        _STL_VERIFY(_First._Getcont() == _Last._Getcont(), "list iterators in range are from different containers");
    }
#endif // _ITERATOR_DEBUG_LEVEL == 2

    using _Prevent_inheriting_unwrap = _List_const_iterator;

    _NODISCARD _List_unchecked_const_iterator<_Mylist> _Unwrapped() const {
        return _List_unchecked_const_iterator<_Mylist>(this->_Ptr, static_cast<const _Mylist*>(this->_Getcont()));
    }

    void _Seek_to(const _List_unchecked_const_iterator<_Mylist> _It) {
        this->_Ptr = _It._Ptr;
    }
};

// CLASS TEMPLATE _List_iterator
template <class _Mylist>
class _List_iterator : public _List_const_iterator<_Mylist> {
public:
    using _Mybase           = _List_const_iterator<_Mylist>;
    using iterator_category = bidirectional_iterator_tag;

    using _Nodeptr        = typename _Mylist::_Nodeptr;
    using value_type      = typename _Mylist::value_type;
    using difference_type = typename _Mylist::difference_type;
    using pointer         = typename _Mylist::pointer;
    using reference       = value_type&;

    using _Mybase::_Mybase;

    _NODISCARD reference operator*() const {
        return const_cast<reference>(_Mybase::operator*());
    }

    _NODISCARD pointer operator->() const {
        return pointer_traits<pointer>::pointer_to(**this);
    }

    _List_iterator& operator++() {
        _Mybase::operator++();
        return *this;
    }

    _List_iterator operator++(int) {
        _List_iterator _Tmp = *this;
        _Mybase::operator++();
        return _Tmp;
    }

    _List_iterator& operator--() {
        _Mybase::operator--();
        return *this;
    }

    _List_iterator operator--(int) {
        _List_iterator _Tmp = *this;
        _Mybase::operator--();
        return _Tmp;
    }

    using _Prevent_inheriting_unwrap = _List_iterator;

    _NODISCARD _List_unchecked_iterator<_Mylist> _Unwrapped() const {
        return _List_unchecked_iterator<_Mylist>(this->_Ptr, static_cast<const _Mylist*>(this->_Getcont()));
    }
};

// list TYPE WRAPPERS
template <class _Value_type, class _Size_type, class _Difference_type, class _Pointer, class _Const_pointer,
    class _Reference, class _Const_reference, class _Nodeptr_type>
struct _List_iter_types {
    using value_type      = _Value_type;
    using size_type       = _Size_type;
    using difference_type = _Difference_type;
    using pointer         = _Pointer;
    using const_pointer   = _Const_pointer;
    using _Nodeptr        = _Nodeptr_type;
};

template <class _Value_type, class _Voidptr>
struct _List_node { // list node
    using value_type = _Value_type;
    using _Nodeptr   = _Rebind_pointer_t<_Voidptr, _List_node>;
    _Nodeptr _Next; // successor node, or first element if head
    _Nodeptr _Prev; // predecessor node, or last element if head
    _Value_type _Myval; // the stored value, unused if head

    _List_node(const _List_node&) = delete;
    _List_node& operator=(const _List_node&) = delete;

    template <class _Alnode>
    static _Nodeptr _Buyheadnode(_Alnode& _Al) {
        const auto _Result = _Al.allocate(1);
        _Construct_in_place(_Result->_Next, _Result);
        _Construct_in_place(_Result->_Prev, _Result);
        return _Result;
    }

    template <class _Alnode>
    static void _Freenode0(_Alnode& _Al, _Nodeptr _Ptr) noexcept {
        // destroy pointer members in _Ptr and deallocate with _Al
        static_assert(is_same_v<typename _Alnode::value_type, _List_node>, "Bad _Freenode0 call");
        _Destroy_in_place(_Ptr->_Next);
        _Destroy_in_place(_Ptr->_Prev);
        allocator_traits<_Alnode>::deallocate(_Al, _Ptr, 1);
    }

    template <class _Alnode>
    static void _Freenode(_Alnode& _Al, _Nodeptr _Ptr) noexcept { // destroy all members in _Ptr and deallocate with _Al
        allocator_traits<_Alnode>::destroy(_Al, _STD addressof(_Ptr->_Myval));
        _Freenode0(_Al, _Ptr);
    }

    template <class _Alnode>
    static void _Free_non_head(
        _Alnode& _Al, _Nodeptr _Head) noexcept { // free a list starting at _First and terminated at nullptr
        _Head->_Prev->_Next = nullptr;

        auto _Pnode = _Head->_Next;
        for (_Nodeptr _Pnext; _Pnode; _Pnode = _Pnext) {
            _Pnext = _Pnode->_Next;
            _Freenode(_Al, _Pnode);
        }
    }
};

template <class _Ty>
struct _List_simple_types : _Simple_types<_Ty> {
    using _Node    = _List_node<_Ty, void*>;
    using _Nodeptr = _Node*;
};

// CLASS TEMPLATE _List_val
template <class _Val_types>
class _List_val : public _Container_base {
public:
    using _Nodeptr = typename _Val_types::_Nodeptr;

    using value_type      = typename _Val_types::value_type;
    using size_type       = typename _Val_types::size_type;
    using difference_type = typename _Val_types::difference_type;
    using pointer         = typename _Val_types::pointer;
    using const_pointer   = typename _Val_types::const_pointer;
    using reference       = value_type&;
    using const_reference = const value_type&;

    _List_val() noexcept : _Myhead(), _Mysize(0) {} // initialize data

    void _Orphan_ptr2(_Nodeptr _Ptr) noexcept { // orphan iterators with specified node pointers
#if _ITERATOR_DEBUG_LEVEL == 2
        _Lockit _Lock(_LOCK_DEBUG);
        _Iterator_base12** _Pnext = &this->_Myproxy->_Myfirstiter;
        const auto _Head          = _Myhead;
        while (*_Pnext) {
            _Iterator_base12** _Pnextnext = &(*_Pnext)->_Mynextiter;
            const auto _Pnextptr          = static_cast<_List_const_iterator<_List_val>&>(**_Pnext)._Ptr;
            if (_Pnextptr == _Head || _Pnextptr != _Ptr) {
                // iterator is end() or doesn't point at the one we are orphaning, move on
                _Pnext = _Pnextnext;
            } else { // orphan the iterator
                (*_Pnext)->_Myproxy = nullptr;
                *_Pnext             = *_Pnextnext;
            }
        }
#else // ^^^ _ITERATOR_DEBUG_LEVEL == 2 ^^^ // vvv _ITERATOR_DEBUG_LEVEL != 2 vvv
        (void) _Ptr;
#endif // _ITERATOR_DEBUG_LEVEL == 2
    }

    void _Orphan_non_end() noexcept { // orphan iterators except end()
#if _ITERATOR_DEBUG_LEVEL == 2
        _Lockit _Lock(_LOCK_DEBUG);
        _Iterator_base12** _Pnext = &this->_Myproxy->_Myfirstiter;
        const auto _Head          = _Myhead;
        while (*_Pnext) {
            _Iterator_base12** _Pnextnext = &(*_Pnext)->_Mynextiter;
            if (static_cast<_List_const_iterator<_List_val>&>(**_Pnext)._Ptr == _Head) { // iterator is end(), move on
                _Pnext = _Pnextnext;
            } else { // orphan the iterator
                (*_Pnext)->_Myproxy = nullptr;
                *_Pnext             = *_Pnextnext;
            }
        }
#endif // _ITERATOR_DEBUG_LEVEL == 2
    }

    _Nodeptr _Unlinknode(_Nodeptr _Pnode) noexcept { // unlink node at _Where from the list
        _Orphan_ptr2(_Pnode);
        _Pnode->_Prev->_Next = _Pnode->_Next;
        _Pnode->_Next->_Prev = _Pnode->_Prev;
        --_Mysize;
        return _Pnode;
    }

#if _ITERATOR_DEBUG_LEVEL == 2
    void _Adopt_unique(_List_val& _Other, _Nodeptr _Pnode) noexcept {
        // adopt iterators pointing to the spliced node
        _Lockit _Lock(_LOCK_DEBUG);
        _Iterator_base12** _Pnext = &_Other._Myproxy->_Myfirstiter;
        const auto _Myproxy       = this->_Myproxy;
        while (*_Pnext) {
            auto& _Iter = static_cast<_List_const_iterator<_List_val>&>(**_Pnext);
            if (_Iter._Ptr == _Pnode) { // adopt the iterator
                *_Pnext                = _Iter._Mynextiter;
                _Iter._Myproxy         = _Myproxy;
                _Iter._Mynextiter      = _Myproxy->_Myfirstiter;
                _Myproxy->_Myfirstiter = _STD addressof(_Iter);
            } else { // skip the iterator
                _Pnext = &_Iter._Mynextiter;
            }
        }
    }

    void _Adopt_all(_List_val& _Other) noexcept {
        // adopt all iterators (except _Other.end())
        _Lockit _Lock(_LOCK_DEBUG);
        _Iterator_base12** _Pnext = &_Other._Myproxy->_Myfirstiter;
        const auto _Myproxy       = this->_Myproxy;
        const auto _Otherhead     = _Other._Myhead;
        while (*_Pnext) {
            auto& _Iter = static_cast<_List_const_iterator<_List_val>&>(**_Pnext);
            if (_Iter._Ptr != _Otherhead) { // adopt the iterator
                *_Pnext                = _Iter._Mynextiter;
                _Iter._Myproxy         = _Myproxy;
                _Iter._Mynextiter      = _Myproxy->_Myfirstiter;
                _Myproxy->_Myfirstiter = _STD addressof(_Iter);
            } else { // skip the iterator
                _Pnext = &_Iter._Mynextiter;
            }
        }
    }

    void _Adopt_range(_List_val& _Other, const _Nodeptr _First, const _Nodeptr _Last) noexcept {
        // adopt all iterators pointing to nodes in the "range" [_First, _Last) by marking nodes
        _Lockit _Lock(_LOCK_DEBUG);
        _Iterator_base12** _Pnext = &_Other._Myproxy->_Myfirstiter;
        const auto _Myproxy       = this->_Myproxy;
        _Nodeptr _Oldprev         = _First->_Prev;
        for (_Nodeptr _Ptr = _First; _Ptr != _Last; _Ptr = _Ptr->_Next) { // mark _Prev pointers
            _Ptr->_Prev = nullptr;
        }

        while (*_Pnext) { // check the iterator
            auto& _Iter = static_cast<_List_const_iterator<_List_val>&>(**_Pnext);
            if (_Iter._Ptr->_Prev) { // skip the iterator
                _Pnext = &_Iter._Mynextiter;
            } else { // adopt the iterator
                *_Pnext                = _Iter._Mynextiter;
                _Iter._Myproxy         = _Myproxy;
                _Iter._Mynextiter      = _Myproxy->_Myfirstiter;
                _Myproxy->_Myfirstiter = _STD addressof(_Iter);
            }
        }

        for (_Nodeptr _Ptr = _First; _Ptr != _Last; _Ptr = _Ptr->_Next) { // restore _Prev pointers
            _Ptr->_Prev = _Oldprev;
            _Oldprev    = _Ptr;
        }
    }
#endif // _ITERATOR_DEBUG_LEVEL == 2

    static _Nodeptr _Unchecked_splice(const _Nodeptr _Before, const _Nodeptr _First, const _Nodeptr _Last) noexcept {
        // splice [_First, _Last) before _Before; returns _Last
        _STL_INTERNAL_CHECK(_Before != _First && _Before != _Last && _First != _Last);
        // 3 reads and 6 writes

        // fixup the _Next values
        const auto _First_prev  = _First->_Prev;
        _First_prev->_Next      = _Last;
        const auto _Last_prev   = _Last->_Prev;
        _Last_prev->_Next       = _Before;
        const auto _Before_prev = _Before->_Prev;
        _Before_prev->_Next     = _First;

        // fixup the _Prev values
        _Before->_Prev = _Last_prev;
        _Last->_Prev   = _First_prev;
        _First->_Prev  = _Before_prev;

        return _Last;
    }

    static _Nodeptr _Unchecked_splice(const _Nodeptr _Before, const _Nodeptr _First) noexcept {
        // splice [_First, _First->_Next) before _Before; returns _First->_Next
        _STL_INTERNAL_CHECK(_Before != _First && _First->_Next != _Before);
        // still 3 reads and 6 writes, but 1 less read if the caller was going to get _First->_Next
        const auto _Last = _First->_Next;

        // fixup the _Next values
        const auto _First_prev = _First->_Prev;
        _First_prev->_Next     = _Last;
        // const auto _Last_prev = _First;
        _First->_Next           = _Before;
        const auto _Before_prev = _Before->_Prev;
        _Before_prev->_Next     = _First;

        // fixup the _Prev values
        _Before->_Prev = _First;
        _Last->_Prev   = _First_prev;
        _First->_Prev  = _Before_prev;

        return _Last;
    }

    template <class _Pr2>
    static _Nodeptr _Merge_same(_Nodeptr _First, _Nodeptr _Mid, const _Nodeptr _Last, _Pr2 _Pred) {
        // Merge the sorted ranges [_First, _Mid) and [_Mid, _Last)
        // Returns the new beginning of the range (which won't be _First if it was spliced elsewhere)
        _STL_INTERNAL_CHECK(_First != _Mid && _Mid != _Last);
        _Nodeptr _Newfirst;
        if (_DEBUG_LT_PRED(_Pred, _Mid->_Myval, _First->_Myval)) {
            // _Mid will be spliced to the front of the range
            _Newfirst = _Mid;
        } else {
            // Establish _Pred(_Mid->_Myval, _First->_Myval) by skipping over elements from the first
            // range already in position.
            _Newfirst = _First;
            do {
                _First = _First->_Next;
                if (_First == _Mid) {
                    return _Newfirst;
                }
            } while (!_DEBUG_LT_PRED(_Pred, _Mid->_Myval, _First->_Myval));
        }

        for (;;) { // process one run splice
            auto _Run_start = _Mid;
            do { // find the end of the "run" of elements we need to splice from the second range into the first
                _Mid = _Mid->_Next;
            } while (_Mid != _Last && _DEBUG_LT_PRED(_Pred, _Mid->_Myval, _First->_Myval));

            // [_Run_start, _Mid) goes before _First->_Myval
            _Unchecked_splice(_First, _Run_start, _Mid);
            if (_Mid == _Last) {
                return _Newfirst;
            }

            // Reestablish _Pred(_Mid->_Myval, _First->_Myval) by skipping over elements from the first
            // range already in position.
            do {
                _First = _First->_Next;
                if (_First == _Mid) {
                    return _Newfirst;
                }
            } while (!_DEBUG_LT_PRED(_Pred, _Mid->_Myval, _First->_Myval));
        }
    }

    template <class _Pr2>
    static _Nodeptr _Sort(_Nodeptr& _First, const size_type _Size, _Pr2 _Pred) {
        // order [_First, _Last), using _Pred, return _First + _Size
        switch (_Size) {
        case 0:
            return _First;
        case 1:
            return _First->_Next;
        default:
            break;
        }

        auto _Mid        = _Sort(_First, _Size / 2, _Pred);
        const auto _Last = _Sort(_Mid, _Size - _Size / 2, _Pred);
        _First           = _Merge_same(_First, _Mid, _Last, _Pred);
        return _Last;
    }

    _Nodeptr _Myhead; // pointer to head node
    size_type _Mysize; // number of elements
};

// STRUCT TEMPLATE _List_node_emplace_op2
template <class _Alnode>
struct _List_node_emplace_op2 : _Alloc_construct_ptr<_Alnode> {
    using _Alnode_traits = allocator_traits<_Alnode>;
    using pointer        = typename _Alnode_traits::pointer;

    template <class... _Valtys>
    explicit _List_node_emplace_op2(_Alnode& _Al_, _Valtys&&... _Vals) : _Alloc_construct_ptr<_Alnode>(_Al_) {
        this->_Allocate();
        _Alnode_traits::construct(this->_Al, _STD addressof(this->_Ptr->_Myval), _STD forward<_Valtys>(_Vals)...);
    }

    ~_List_node_emplace_op2() {
        if (this->_Ptr != pointer{}) {
            _Alnode_traits::destroy(this->_Al, _STD addressof(this->_Ptr->_Myval));
        }
    }

    _List_node_emplace_op2(const _List_node_emplace_op2&) = delete;
    _List_node_emplace_op2& operator=(const _List_node_emplace_op2&) = delete;

    pointer _Transfer_before(const pointer _Insert_before) noexcept {
        const pointer _Insert_after = _Insert_before->_Prev;
        _Construct_in_place(this->_Ptr->_Next, _Insert_before);
        _Construct_in_place(this->_Ptr->_Prev, _Insert_after);
        const auto _Result    = this->_Ptr;
        this->_Ptr            = pointer{};
        _Insert_before->_Prev = _Result;
        _Insert_after->_Next  = _Result;
        return _Result;
    }
};

// STRUCT TEMPLATE _List_node_insert_op2
template <class _Alnode>
struct _List_node_insert_op2 {
    // list insert operation which maintains exception safety
    using _Alnode_traits = allocator_traits<_Alnode>;
    using pointer        = typename _Alnode_traits::pointer;
    using size_type      = typename _Alnode_traits::size_type;
    using value_type     = typename _Alnode_traits::value_type;

    explicit _List_node_insert_op2(_Alnode& _Al_) : _Al(_Al_), _Added(0) {}

    _List_node_insert_op2(const _List_node_insert_op2&) = delete;
    _List_node_insert_op2& operator=(const _List_node_insert_op2&) = delete;

    template <class... _CArgT>
    void _Append_n(size_type _Count, const _CArgT&... _Carg) {
        // Append _Count elements constructed from _Carg
        if (_Count <= 0) {
            return;
        }

        _Alloc_construct_ptr<_Alnode> _Newnode(_Al);
        if (_Added == 0) {
            _Newnode._Allocate(); // throws
            _Alnode_traits::construct(_Al, _STD addressof(_Newnode._Ptr->_Myval), _Carg...); // throws
            _Head = _Newnode._Ptr;
            _Tail = _Newnode._Ptr;
            ++_Added;
            --_Count;
        }

        for (; 0 < _Count; --_Count) {
            _Newnode._Allocate(); // throws
            _Alnode_traits::construct(_Al, _STD addressof(_Newnode._Ptr->_Myval), _Carg...); // throws
            _Construct_in_place(_Tail->_Next, _Newnode._Ptr);
            _Construct_in_place(_Newnode._Ptr->_Prev, _Tail);
            _Tail = _Newnode._Ptr;
            ++_Added;
        }

        _Newnode._Ptr = pointer{};
    }

    template <class _InIt, class _Sentinel>
    void _Append_range_unchecked(_InIt _First, const _Sentinel _Last) {
        // Append the values in [_First, _Last)
        if (_First == _Last) { // throws
            return;
        }

        _Alloc_construct_ptr<_Alnode> _Newnode(_Al);
        if (_Added == 0) {
            _Newnode._Allocate(); // throws
            _Alnode_traits::construct(_Al, _STD addressof(_Newnode._Ptr->_Myval), *_First); // throws
            const auto _Newhead = _STD exchange(_Newnode._Ptr, pointer{});
            _Head               = _Newhead;
            _Tail               = _Newhead;
            ++_Added;
            ++_First; // throws
        }

        while (_First != _Last) { // throws
            _Newnode._Allocate(); // throws
            _Alnode_traits::construct(_Al, _STD addressof(_Newnode._Ptr->_Myval), *_First); // throws
            _Construct_in_place(_Tail->_Next, _Newnode._Ptr);
            _Construct_in_place(_Newnode._Ptr->_Prev, _Tail);
            _Tail = _STD exchange(_Newnode._Ptr, pointer{});
            ++_Added;
            ++_First; // throws
        }
    }

    template <class _Val_types>
    pointer _Attach_before(_List_val<_Val_types>& _List_data, const pointer _Insert_before) noexcept {
        // Attach the elements in *this before _Insert_before.
        // If *this is empty, returns _Insert_before; otherwise returns a pointer to the first inserted list node.
        // Resets *this to the default-initialized state.

        const auto _Local_added = _Added;
        if (_Local_added == 0) {
            return _Insert_before;
        }

        const auto _Local_head   = _Head;
        const auto _Local_tail   = _Tail;
        const auto _Insert_after = _Insert_before->_Prev;

        _Construct_in_place(_Local_head->_Prev, _Insert_after);
        _Insert_after->_Next = _Local_head;
        _Construct_in_place(_Local_tail->_Next, _Insert_before);
        _Insert_before->_Prev = _Local_tail;

        _List_data._Mysize += _Local_added;
        _Added = 0;
        return _Local_head;
    }

    template <class _Val_types>
    void _Attach_at_end(_List_val<_Val_types>& _List_data) noexcept {
        _Attach_before(_List_data, _List_data._Myhead);
    }

    template <class _Val_types>
    void _Attach_head(_List_val<_Val_types>& _List_data) {
        _Alloc_construct_ptr<_Alnode> _Newnode(_Al);
        _Newnode._Allocate(); // throws
        const auto _Local_added = _STD exchange(_Added, size_type{0});
        if (_Local_added == 0) {
            _Construct_in_place(_Newnode._Ptr->_Next, _Newnode._Ptr);
            _Construct_in_place(_Newnode._Ptr->_Prev, _Newnode._Ptr);
        } else {
            const auto _Local_head = _Head;
            const auto _Local_tail = _Tail;
            _Construct_in_place(_Newnode._Ptr->_Next, _Local_head);
            _Construct_in_place(_Newnode._Ptr->_Prev, _Local_tail);
            _Construct_in_place(_Local_head->_Prev, _Newnode._Ptr);
            _Construct_in_place(_Local_tail->_Next, _Newnode._Ptr);
        }

        _List_data._Mysize = _Local_added;
        _List_data._Myhead = _Newnode._Release();
    }

    ~_List_node_insert_op2() {
        if (_Added == 0) {
            return;
        }

        _Construct_in_place(_Head->_Prev, pointer{});
        _Construct_in_place(_Tail->_Next, pointer{});
        pointer _Subject = _Head;
        while (_Subject) {
            value_type::_Freenode(_Al, _STD exchange(_Subject, _Subject->_Next));
        }
    }

private:
    _Alnode& _Al;
    size_type _Added; // if 0, the values of _Head and _Tail are indeterminate
    pointer _Tail; // points to the most recently appended element; it doesn't have _Next constructed
    pointer _Head; // points to the first appended element; it doesn't have _Prev constructed
};

template <class _Traits>
class _Hash;

// CLASS TEMPLATE list
template <class _Ty, class _Alloc = allocator<_Ty>>
class list { // bidirectional linked list
private:
    template <class>
    friend class _Hash;
    template <class _Traits>
    friend bool _Hash_equal(const _Hash<_Traits>&, const _Hash<_Traits>&);
#if !_HAS_IF_CONSTEXPR
    template <class _Traits>
    friend bool _Hash_equal_elements(const _Hash<_Traits>& _Left, const _Hash<_Traits>& _Right, false_type);
#endif // _HAS_IF_CONSTEXPR
    // 对于list<int>, _Alty = allocator<int>
    using _Alty          = _Rebind_alloc_t<_Alloc, _Ty>;
    using _Alty_traits   = allocator_traits<_Alty>;
    // struct std::_List_node<int,void * __ptr64>
    // allocator_traits<_Alloc>::void_pointer = void*
    using _Node          = _List_node<_Ty, typename allocator_traits<_Alloc>::void_pointer>;  // 就是一个_Node类
    // _Alnode = allocator<_Node>
    using _Alnode        = _Rebind_alloc_t<_Alloc, _Node>;
    using _Alnode_traits = allocator_traits<_Alnode>;
    using _Nodeptr       = typename _Alnode_traits::pointer;

    using _Val_types = conditional_t<_Is_simple_alloc_v<_Alnode>, _List_simple_types<_Ty>,
        _List_iter_types<_Ty, typename _Alty_traits::size_type, typename _Alty_traits::difference_type,
            typename _Alty_traits::pointer, typename _Alty_traits::const_pointer, _Ty&, const _Ty&, _Nodeptr>>;

    using _Scary_val = _List_val<_Val_types>;

public:
    static_assert(!_ENFORCE_MATCHING_ALLOCATORS || is_same_v<_Ty, typename _Alloc::value_type>,
        _MISMATCHED_ALLOCATOR_MESSAGE("list<T, Allocator>", "T"));

    using value_type      = _Ty;
    using allocator_type  = _Alloc;
    using size_type       = typename _Alty_traits::size_type;
    using difference_type = typename _Alty_traits::difference_type;
    using pointer         = typename _Alty_traits::pointer;
    using const_pointer   = typename _Alty_traits::const_pointer;
    using reference       = value_type&;
    using const_reference = const value_type&;

    using iterator                  = _List_iterator<_Scary_val>;
    using const_iterator            = _List_const_iterator<_Scary_val>;
    using _Unchecked_iterator       = _List_unchecked_iterator<_Scary_val>;
    using _Unchecked_const_iterator = _List_unchecked_const_iterator<_Scary_val>;

    using reverse_iterator       = _STD reverse_iterator<iterator>;
    using const_reverse_iterator = _STD reverse_iterator<const_iterator>;

    list() : _Mypair(_Zero_then_variadic_args_t{}) {
        //cout << typeid(_Node).name() << endl;
        //cout << typeid(_Alnode).name() << endl;
        _Alloc_sentinel_and_proxy();
    }

    explicit list(const _Alloc& _Al) : _Mypair(_One_then_variadic_args_t{}, _Al) {
        _Alloc_sentinel_and_proxy();
    }

private:
    template <class _Any_alloc>
    explicit list(_Move_allocator_tag, _Any_alloc& _Al) : _Mypair(_One_then_variadic_args_t{}, _STD move(_Al)) {
        _Alloc_sentinel_and_proxy();
    }

    void _Construct_n(_CRT_GUARDOVERFLOW size_type _Count) {
        auto&& _Alproxy = _GET_PROXY_ALLOCATOR(_Alnode, _Getal());
        _Container_proxy_ptr<_Alty> _Proxy(_Alproxy, _Mypair._Myval2);
        _List_node_insert_op2<_Alnode> _Appended(_Getal());
        _Appended._Append_n(_Count);
        _Appended._Attach_head(_Mypair._Myval2);
        _Proxy._Release();
    }

public:
    explicit list(_CRT_GUARDOVERFLOW size_type _Count)
        : _Mypair(_Zero_then_variadic_args_t{}) { // construct list from _Count * _Ty()
        _Construct_n(_Count);
    }

    explicit list(_CRT_GUARDOVERFLOW size_type _Count, const _Alloc& _Al)
        : _Mypair(_One_then_variadic_args_t{}, _Al) { // construct list from _Count * _Ty(), with allocator
        _Construct_n(_Count);
    }

private:
    void _Construct_n(_CRT_GUARDOVERFLOW size_type _Count, const _Ty& _Val) {
        auto&& _Alproxy = _GET_PROXY_ALLOCATOR(_Alnode, _Getal());
        _Container_proxy_ptr<_Alty> _Proxy(_Alproxy, _Mypair._Myval2);
        _List_node_insert_op2<_Alnode> _Appended(_Getal());
        _Appended._Append_n(_Count, _Val);
        _Appended._Attach_head(_Mypair._Myval2);
        _Proxy._Release();
    }

public:
    list(_CRT_GUARDOVERFLOW size_type _Count, const _Ty& _Val)
        : _Mypair(_Zero_then_variadic_args_t{}) { // construct list from _Count * _Val
        _Construct_n(_Count, _Val);
    }

    list(_CRT_GUARDOVERFLOW size_type _Count, const _Ty& _Val, const _Alloc& _Al)
        : _Mypair(_One_then_variadic_args_t{}, _Al) { // construct list from _Count * _Val, allocator
        _Construct_n(_Count, _Val);
    }

private:
    template <class _Iter>
    void _Construct_range_unchecked(_Iter _First, _Iter _Last) {
        auto&& _Alproxy = _GET_PROXY_ALLOCATOR(_Alnode, _Getal());
        _Container_proxy_ptr<_Alty> _Proxy(_Alproxy, _Mypair._Myval2);
        _List_node_insert_op2<_Alnode> _Appended(_Getal());
        _Appended._Append_range_unchecked(_First, _Last);
        _Appended._Attach_head(_Mypair._Myval2);
        _Proxy._Release();
    }

public:
    list(const list& _Right)
        : _Mypair(_One_then_variadic_args_t{}, _Alnode_traits::select_on_container_copy_construction(_Right._Getal())) {
        _Construct_range_unchecked(_Right._Unchecked_begin(), _Right._Unchecked_end());
    }

    list(const list& _Right, const _Alloc& _Al) : _Mypair(_One_then_variadic_args_t{}, _Al) {
        _Construct_range_unchecked(_Right._Unchecked_begin(), _Right._Unchecked_end());
    }

    template <class _Iter, enable_if_t<_Is_iterator_v<_Iter>, int> = 0>
    list(_Iter _First, _Iter _Last) : _Mypair(_Zero_then_variadic_args_t{}) {
        _Adl_verify_range(_First, _Last);
        _Construct_range_unchecked(_Get_unwrapped(_First), _Get_unwrapped(_Last));
    }

    template <class _Iter, enable_if_t<_Is_iterator_v<_Iter>, int> = 0>
    list(_Iter _First, _Iter _Last, const _Alloc& _Al) : _Mypair(_One_then_variadic_args_t{}, _Al) {
        _Adl_verify_range(_First, _Last);
        _Construct_range_unchecked(_Get_unwrapped(_First), _Get_unwrapped(_Last));
    }

    list(list&& _Right) : _Mypair(_One_then_variadic_args_t{}, _STD move(_Right._Getal())) {
        _Alloc_sentinel_and_proxy();
        _Swap_val(_Right);
    }

    list(list&& _Right, const _Alloc& _Al) : _Mypair(_One_then_variadic_args_t{}, _Al) {
        if _CONSTEXPR_IF (!_Alnode_traits::is_always_equal::value) {
            if (_Getal() != _Right._Getal()) {
                _Construct_range_unchecked(_STD make_move_iterator(_Right._Unchecked_begin()),
                    _STD make_move_iterator(_Right._Unchecked_end()));
                return;
            }
        }

        _Alloc_sentinel_and_proxy();
        _Swap_val(_Right);
    }

private:
    void _Move_assign(list& _Right, _Equal_allocators) noexcept {
        clear();
        _Pocma(_Getal(), _Right._Getal());
        _Swap_val(_Right);
    }

    void _Move_assign(list& _Right, _Propagate_allocators) {
        auto& _Al       = _Getal();
        auto& _Right_al = _Right._Getal();
        if (_Al == _Right_al) {
            _Move_assign(_Right, _Equal_allocators{});
        } else {
            auto&& _Alproxy       = _GET_PROXY_ALLOCATOR(_Alnode, _Al);
            auto&& _Right_alproxy = _GET_PROXY_ALLOCATOR(_Alnode, _Right_al);
            _Container_proxy_ptr<_Alty> _Proxy(_Right_alproxy, _Leave_proxy_unbound{});
            auto& _My_data      = _Mypair._Myval2;
            auto& _Right_data   = _Right._Mypair._Myval2;
            const auto _Newhead = _STD exchange(_Right_data._Myhead, _Node::_Buyheadnode(_Right_al));
            const auto _Newsize = _STD exchange(_Right_data._Mysize, size_type{0});
            _Tidy();
            _Pocma(_Al, _Right_al);
            _My_data._Myhead = _Newhead;
            _My_data._Mysize = _Newsize;
            _Proxy._Bind(_Alproxy, _STD addressof(_My_data));
            _My_data._Swap_proxy_and_iterators(_Right_data);
        }
    }

    void _Move_assign(list& _Right, _No_propagate_allocators) {
        if (_Getal() == _Right._Getal()) {
            _Move_assign(_Right, _Equal_allocators{});
        } else {
            assign(
                _STD make_move_iterator(_Right._Unchecked_begin()), _STD make_move_iterator(_Right._Unchecked_end()));
        }
    }

public:
    list& operator=(list&& _Right) noexcept(
        noexcept(_Move_assign(_Right, _Choose_pocma<_Alnode>{}))) /* strengthened */ {
        if (this != _STD addressof(_Right)) {
            _Move_assign(_Right, _Choose_pocma<_Alnode>{});
        }

        return *this;
    }

private:
    void _Swap_val(list& _Right) noexcept { // swap with _Right, same allocator
        auto& _My_data    = _Mypair._Myval2;
        auto& _Right_data = _Right._Mypair._Myval2;
        _My_data._Swap_proxy_and_iterators(_Right_data);
        _Swap_adl(_My_data._Myhead, _Right_data._Myhead);
        _STD swap(_My_data._Mysize, _Right_data._Mysize);
    }

public:
    void push_front(_Ty&& _Val) { // insert element at beginning
        _Emplace(_Mypair._Myval2._Myhead->_Next, _STD move(_Val));
    }

    void push_back(_Ty&& _Val) { // insert element at end
        _Emplace(_Mypair._Myval2._Myhead, _STD move(_Val));
    }

    iterator insert(const_iterator _Where, _Ty&& _Val) { // insert _Val at _Where
        return emplace(_Where, _STD move(_Val));
    }

    template <class... _Valty>
    decltype(auto) emplace_front(_Valty&&... _Val) { // insert element at beginning
        reference _Result = _Emplace(_Mypair._Myval2._Myhead->_Next, _STD forward<_Valty>(_Val)...)->_Myval;

#if _HAS_CXX17
        return _Result;
#else // ^^^ _HAS_CXX17 // !_HAS_CXX17 vvv
        (void) _Result;
#endif // _HAS_CXX17
    }

    template <class... _Valty>
    decltype(auto) emplace_back(_Valty&&... _Val) { // insert element at end
        reference _Result = _Emplace(_Mypair._Myval2._Myhead, _STD forward<_Valty>(_Val)...)->_Myval;

#if _HAS_CXX17
        return _Result;
#else // ^^^ _HAS_CXX17 // !_HAS_CXX17 vvv
        (void) _Result;
#endif // _HAS_CXX17
    }

    template <class... _Valty>
    iterator emplace(const const_iterator _Where, _Valty&&... _Val) { // insert element at _Where
#if _ITERATOR_DEBUG_LEVEL == 2
        _STL_VERIFY(_Where._Getcont() == _STD addressof(_Mypair._Myval2), "list emplace iterator outside range");
#endif // _ITERATOR_DEBUG_LEVEL == 2

        return _Make_iter(_Emplace(_Where._Ptr, _STD forward<_Valty>(_Val)...));
    }

    template <class... _Valty>
    _Nodeptr _Emplace(const _Nodeptr _Where, _Valty&&... _Val) { // insert element at _Where
        size_type& _Mysize = _Mypair._Myval2._Mysize;
        if (_Mysize == max_size()) {
            _Xlength_error("list too long");
        }

        _List_node_emplace_op2<_Alnode> _Op{_Getal(), _STD forward<_Valty>(_Val)...};
        ++_Mysize;
        return _Op._Transfer_before(_Where);
    }

    list(initializer_list<_Ty> _Ilist) : _Mypair(_Zero_then_variadic_args_t{}) {
        _Construct_range_unchecked(_Ilist.begin(), _Ilist.end());
    }

    list(initializer_list<_Ty> _Ilist, const _Alloc& _Al) : _Mypair(_One_then_variadic_args_t{}, _Al) {
        _Construct_range_unchecked(_Ilist.begin(), _Ilist.end());
    }

    list& operator=(initializer_list<_Ty> _Ilist) {
        assign(_Ilist.begin(), _Ilist.end());
        return *this;
    }

    void assign(initializer_list<_Ty> _Ilist) {
        assign(_Ilist.begin(), _Ilist.end());
    }

    iterator insert(const_iterator _Where,
        initializer_list<_Ty> _Ilist) { // insert initializer_list
        return insert(_Where, _Ilist.begin(), _Ilist.end());
    }

    ~list() noexcept {
        _Tidy();
#if _ITERATOR_DEBUG_LEVEL != 0 // TRANSITION, ABI
        auto&& _Alproxy = _GET_PROXY_ALLOCATOR(_Alnode, _Getal());
        _Delete_plain_internal(_Alproxy, _Mypair._Myval2._Myproxy);
#endif // _ITERATOR_DEBUG_LEVEL != 0
    }

private:
    void _Reload_sentinel_and_proxy(const list& _Right) { // reload sentinel / proxy from unequal POCCA _Right
        auto& _Al             = _Getal();
        auto&& _Alproxy       = _GET_PROXY_ALLOCATOR(_Alnode, _Al);
        auto& _Right_al       = _Right._Getal();
        auto&& _Right_alproxy = _GET_PROXY_ALLOCATOR(_Alnode, _Right_al);
        _Container_proxy_ptr<_Alty> _Proxy(_Right_alproxy, _Leave_proxy_unbound{});
        auto _Right_al_non_const = _Right_al;
        auto _Newhead            = _Node::_Buyheadnode(_Right_al_non_const);
        _Tidy();
        _Pocca(_Al, _Right_al);
        _Mypair._Myval2._Myhead = _Newhead;
        _Mypair._Myval2._Mysize = 0;
        _Proxy._Bind(_Alproxy, _STD addressof(_Mypair._Myval2));
    }

    void _Copy_assign(const list& _Right, false_type) {
        _Pocca(_Getal(), _Right._Getal());
        assign(_Right._Unchecked_begin(), _Right._Unchecked_end());
    }

    void _Copy_assign(const list& _Right, true_type) {
        if (_Getal() != _Right._Getal()) {
            _Reload_sentinel_and_proxy(_Right);
        }

        assign(_Right._Unchecked_begin(), _Right._Unchecked_end());
    }

public:
    list& operator=(const list& _Right) {
        if (this != _STD addressof(_Right)) {
            _Copy_assign(_Right, _Choose_pocca<_Alnode>{});
        }

        return *this;
    }

    _NODISCARD iterator begin() noexcept {
        return iterator(_Mypair._Myval2._Myhead->_Next, _STD addressof(_Mypair._Myval2));
    }

    _NODISCARD const_iterator begin() const noexcept {
        return const_iterator(_Mypair._Myval2._Myhead->_Next, _STD addressof(_Mypair._Myval2));
    }

    _NODISCARD iterator end() noexcept {
        return iterator(_Mypair._Myval2._Myhead, _STD addressof(_Mypair._Myval2));
    }

    _NODISCARD const_iterator end() const noexcept {
        return const_iterator(_Mypair._Myval2._Myhead, _STD addressof(_Mypair._Myval2));
    }

    _Unchecked_iterator _Unchecked_begin() noexcept {
        return _Unchecked_iterator(_Mypair._Myval2._Myhead->_Next, nullptr);
    }

    _Unchecked_const_iterator _Unchecked_begin() const noexcept {
        return _Unchecked_const_iterator(_Mypair._Myval2._Myhead->_Next, nullptr);
    }

    _Unchecked_iterator _Unchecked_end() noexcept {
        return _Unchecked_iterator(_Mypair._Myval2._Myhead, nullptr);
    }

    _Unchecked_const_iterator _Unchecked_end() const noexcept {
        return _Unchecked_const_iterator(_Mypair._Myval2._Myhead, nullptr);
    }

    iterator _Make_iter(_Nodeptr _Where) const noexcept {
        return iterator(_Where, _STD addressof(_Mypair._Myval2));
    }

    const_iterator _Make_const_iter(_Nodeptr _Where) const noexcept {
        return const_iterator(_Where, _STD addressof(_Mypair._Myval2));
    }

    _NODISCARD reverse_iterator rbegin() noexcept {
        return reverse_iterator(end());
    }

    _NODISCARD const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(end());
    }

    _NODISCARD reverse_iterator rend() noexcept {
        return reverse_iterator(begin());
    }

    _NODISCARD const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(begin());
    }

    _NODISCARD const_iterator cbegin() const noexcept {
        return begin();
    }

    _NODISCARD const_iterator cend() const noexcept {
        return end();
    }

    _NODISCARD const_reverse_iterator crbegin() const noexcept {
        return rbegin();
    }

    _NODISCARD const_reverse_iterator crend() const noexcept {
        return rend();
    }

    void resize(_CRT_GUARDOVERFLOW size_type _Newsize) { // determine new length, padding with _Ty() elements as needed
        auto& _My_data = _Mypair._Myval2;
        if (_My_data._Mysize < _Newsize) { // pad to make larger
            _List_node_insert_op2<_Alnode> _Op(_Getal());
            _Op._Append_n(_Newsize - _My_data._Mysize);
            _Op._Attach_at_end(_My_data);
        } else {
            while (_Newsize < _My_data._Mysize) {
                pop_back();
            }
        }
    }

    void resize(_CRT_GUARDOVERFLOW size_type _Newsize, const _Ty& _Val) {
        // determine new length, padding with _Val elements as needed
        auto& _My_data = _Mypair._Myval2;
        if (_My_data._Mysize < _Newsize) { // pad to make larger
            _List_node_insert_op2<_Alnode> _Op(_Getal());
            _Op._Append_n(_Newsize - _My_data._Mysize, _Val);
            _Op._Attach_at_end(_My_data);
        } else {
            while (_Newsize < _My_data._Mysize) {
                pop_back();
            }
        }
    }

    _NODISCARD size_type size() const noexcept {
        return _Mypair._Myval2._Mysize;
    }

    _NODISCARD size_type max_size() const noexcept {
        return (_STD min)(
            static_cast<size_type>((numeric_limits<difference_type>::max)()), _Alnode_traits::max_size(_Getal()));
    }

    _NODISCARD bool empty() const noexcept {
        return _Mypair._Myval2._Mysize == 0;
    }

    _NODISCARD allocator_type get_allocator() const noexcept {
        return static_cast<allocator_type>(_Getal());
    }

    _NODISCARD reference front() noexcept /* strengthened */ {
#if _CONTAINER_DEBUG_LEVEL > 0
        _STL_VERIFY(_Mypair._Myval2._Mysize != 0, "front() called on empty list");
#endif // _CONTAINER_DEBUG_LEVEL > 0

        return _Mypair._Myval2._Myhead->_Next->_Myval;
    }

    _NODISCARD const_reference front() const noexcept /* strengthened */ {
#if _CONTAINER_DEBUG_LEVEL > 0
        _STL_VERIFY(_Mypair._Myval2._Mysize != 0, "front() called on empty list");
#endif // _CONTAINER_DEBUG_LEVEL > 0

        return _Mypair._Myval2._Myhead->_Next->_Myval;
    }

    _NODISCARD reference back() noexcept /* strengthened */ {
#if _CONTAINER_DEBUG_LEVEL > 0
        _STL_VERIFY(_Mypair._Myval2._Mysize != 0, "back() called on empty list");
#endif // _CONTAINER_DEBUG_LEVEL > 0

        return _Mypair._Myval2._Myhead->_Prev->_Myval;
    }

    _NODISCARD const_reference back() const noexcept /* strengthened */ {
#if _CONTAINER_DEBUG_LEVEL > 0
        _STL_VERIFY(_Mypair._Myval2._Mysize != 0, "back() called on empty list");
#endif // _CONTAINER_DEBUG_LEVEL > 0

        return _Mypair._Myval2._Myhead->_Prev->_Myval;
    }

    void push_front(const _Ty& _Val) {
        _Emplace(_Mypair._Myval2._Myhead->_Next, _Val);
    }

    void pop_front() noexcept /* strengthened */ {
#if _CONTAINER_DEBUG_LEVEL > 0
        _STL_VERIFY(_Mypair._Myval2._Mysize != 0, "pop_front called on empty list");
#endif // _CONTAINER_DEBUG_LEVEL > 0

        _Unchecked_erase(_Mypair._Myval2._Myhead->_Next);
    }

    void push_back(const _Ty& _Val) {
        _Emplace(_Mypair._Myval2._Myhead, _Val);
    }

    void pop_back() noexcept /* strengthened */ {
#if _CONTAINER_DEBUG_LEVEL > 0
        _STL_VERIFY(_Mypair._Myval2._Mysize != 0, "pop_back called on empty list");
#endif // _CONTAINER_DEBUG_LEVEL > 0

        _Unchecked_erase(_Mypair._Myval2._Myhead->_Prev);
    }

private:
    template <class _Target_ref, class _UIter>
    void _Assign_cast(_UIter _UFirst, const _UIter _ULast) {
        // assign [_UFirst, _ULast), casting existing nodes to _Target_ref
        const auto _Myend = _Mypair._Myval2._Myhead;
        auto _Old         = _Myend->_Next;
        for (;;) { // attempt to reuse a node
            if (_Old == _Myend) { // no more nodes to reuse, append the rest
                _List_node_insert_op2<_Alnode> _Op(_Getal());
                _Op._Append_range_unchecked(_UFirst, _ULast);
                _Op._Attach_at_end(_Mypair._Myval2);
                return;
            }

            if (_UFirst == _ULast) {
                // input sequence was shorter than existing list, destroy and deallocate what's left
                _Unchecked_erase(_Old, _Myend);
                return;
            }

            // reuse the node
            reinterpret_cast<_Target_ref>(_Old->_Myval) = *_UFirst;
            _Old                                        = _Old->_Next;
            ++_UFirst;
        }
    }

public:
    template <class _Iter, enable_if_t<_Is_iterator_v<_Iter>, int> = 0>
    void assign(_Iter _First, _Iter _Last) {
        _Assign_cast<reference>(_Get_unwrapped(_First), _Get_unwrapped(_Last));
    }

    void assign(_CRT_GUARDOVERFLOW size_type _Count, const _Ty& _Val) { // assign _Count * _Val
        const auto _Myend = _Mypair._Myval2._Myhead;
        auto _Old         = _Myend->_Next;
        for (;;) { // attempt to reuse a node
            if (_Old == _Myend) { // no more nodes to reuse, append the rest
                _List_node_insert_op2<_Alnode> _Op(_Getal());
                _Op._Append_n(_Count, _Val);
                _Op._Attach_at_end(_Mypair._Myval2);
                return;
            }

            if (_Count == 0) {
                // input sequence was shorter than existing list, destroy and deallocate what's left
                _Unchecked_erase(_Old, _Myend);
                return;
            }

            // reuse the node
            _Old->_Myval = _Val;
            _Old         = _Old->_Next;
            --_Count;
        }
    }

    iterator insert(const_iterator _Where, const _Ty& _Val) { // insert _Val at _Where
#if _ITERATOR_DEBUG_LEVEL == 2
        _STL_VERIFY(_Where._Getcont() == _STD addressof(_Mypair._Myval2), "list insert iterator outside range");
#endif // _ITERATOR_DEBUG_LEVEL == 2
        return _Make_iter(_Emplace(_Where._Ptr, _Val));
    }

    iterator insert(const_iterator _Where, _CRT_GUARDOVERFLOW size_type _Count,
        const _Ty& _Val) { // insert _Count * _Val before _Where
#if _ITERATOR_DEBUG_LEVEL == 2
        _STL_VERIFY(_Where._Getcont() == _STD addressof(_Mypair._Myval2), "list insert iterator outside range");
#endif // _ITERATOR_DEBUG_LEVEL == 2
        _List_node_insert_op2<_Alnode> _Op(_Getal());
        _Op._Append_n(_Count, _Val);
        return _Make_iter(_Op._Attach_before(_Mypair._Myval2, _Where._Ptr));
    }

    template <class _Iter, enable_if_t<_Is_iterator_v<_Iter>, int> = 0>
    iterator insert(const const_iterator _Where, _Iter _First, _Iter _Last) { // insert [_First, _Last) before _Where
#if _ITERATOR_DEBUG_LEVEL == 2
        _STL_VERIFY(_Where._Getcont() == _STD addressof(_Mypair._Myval2), "list insert iterator outside range");
#endif // _ITERATOR_DEBUG_LEVEL == 2
        _Adl_verify_range(_First, _Last);
        _List_node_insert_op2<_Alnode> _Op(_Getal());
        _Op._Append_range_unchecked(_Get_unwrapped(_First), _Get_unwrapped(_Last));
        return _Make_iter(_Op._Attach_before(_Mypair._Myval2, _Where._Ptr));
    }

public:
    iterator erase(const const_iterator _Where) noexcept /* strengthened */ {
#if _ITERATOR_DEBUG_LEVEL == 2
        _STL_VERIFY(_Where._Getcont() == _STD addressof(_Mypair._Myval2), "list erase iterator outside range");
#endif // _ITERATOR_DEBUG_LEVEL == 2
        const auto _Result = _Where._Ptr->_Next;
        _Node::_Freenode(_Getal(), _Mypair._Myval2._Unlinknode(_Where._Ptr));
        return _Make_iter(_Result);
    }

private:
    _Nodeptr _Unchecked_erase(const _Nodeptr _Pnode) noexcept { // erase element at _Pnode
        const auto _Result = _Pnode->_Next;
        _Mypair._Myval2._Orphan_ptr2(_Pnode);
        --_Mypair._Myval2._Mysize;
        _Pnode->_Prev->_Next = _Result;
        _Result->_Prev       = _Pnode->_Prev;
        _Node::_Freenode(_Getal(), _Pnode);
        return _Result;
    }

public:
    iterator erase(const const_iterator _First, const const_iterator _Last) noexcept /* strengthened */ {
        _Adl_verify_range(_First, _Last);
        return _Make_iter(_Unchecked_erase(_First._Ptr, _Last._Ptr));
    }

private:
    _Nodeptr _Unchecked_erase(_Nodeptr _First, const _Nodeptr _Last) noexcept { // erase [_First, _Last)
        if (_First == _Last) {
            return _Last;
        }

        const auto _Predecessor = _First->_Prev;
#if _ITERATOR_DEBUG_LEVEL == 2
        const auto _Head = _Mypair._Myval2._Myhead;
        if (_First == _Head->_Next && _Last == _Head) { // orphan all non-end iterators
            _Mypair._Myval2._Orphan_non_end();
        } else { // orphan erased iterators
            _Lockit _Lock(_LOCK_DEBUG);
            for (auto _Marked = _First; _Marked != _Last; _Marked = _Marked->_Next) { // mark erased nodes
                _Marked->_Prev = nullptr;
            }

            _Iterator_base12** _Pnext = &_Mypair._Myval2._Myproxy->_Myfirstiter;
            while (*_Pnext) {
                _Iterator_base12** _Pnextnext = &(*_Pnext)->_Mynextiter;
                if (static_cast<const_iterator&>(**_Pnext)._Ptr->_Prev) { // node still has a _Prev, skip
                    _Pnext = _Pnextnext;
                } else { // orphan the iterator
                    (*_Pnext)->_Myproxy = nullptr;
                    *_Pnext             = *_Pnextnext;
                }
            }

            // _Prev pointers not restored because we're about to delete the nodes of which they are a member anyway
        }
#endif // _ITERATOR_DEBUG_LEVEL == 2

        // snip out the removed range
        _Predecessor->_Next = _Last;
        _Last->_Prev        = _Predecessor;

        // count and deallocate the removed nodes
        auto& _Al           = _Getal();
        size_type _Erasures = 0;
        do {
            const auto _Next = _First->_Next;
            _Node::_Freenode(_Al, _First);
            _First = _Next;
            ++_Erasures;
        } while (_First != _Last);

        _Mypair._Myval2._Mysize -= _Erasures;
        return _Last;
    }

public:
    void clear() noexcept { // erase all
        auto& _My_data = _Mypair._Myval2;
        _My_data._Orphan_non_end();
        _Node::_Free_non_head(_Getal(), _My_data._Myhead);
        _My_data._Myhead->_Next = _My_data._Myhead;
        _My_data._Myhead->_Prev = _My_data._Myhead;
        _My_data._Mysize        = 0;
    }

private:
    void _Tidy() noexcept {
        auto& _Al      = _Getal();
        auto& _My_data = _Mypair._Myval2;
        _My_data._Orphan_all();
        _Node::_Free_non_head(_Al, _My_data._Myhead);
        _Node::_Freenode0(_Al, _My_data._Myhead);
    }

public:
    void swap(list& _Right) noexcept /* strengthened */ {
        if (this != _STD addressof(_Right)) {
            _Pocs(_Getal(), _Right._Getal());
            _Swap_val(_Right);
        }
    }

    void splice(const const_iterator _Where, list& _Right) { // splice all of _Right at _Where
        auto& _Right_data = _Right._Mypair._Myval2;
        if (this != _STD addressof(_Right) && _Right_data._Mysize != 0) { // worth splicing, do it
#if _ITERATOR_DEBUG_LEVEL == 2
            _STL_VERIFY(_Where._Getcont() == _STD addressof(_Mypair._Myval2), "list splice iterator outside range");
#endif // _ITERATOR_DEBUG_LEVEL == 2
            const auto _Right_head = _Right_data._Myhead;
            _Splice(_Where._Ptr, _Right, _Right_head->_Next, _Right_head, _Right_data._Mysize);
        }
    }

    void splice(const const_iterator _Where, list&& _Right) { // splice all of _Right at _Where
        splice(_Where, _Right);
    }

    void splice(const const_iterator _Where, list& _Right, const const_iterator _First) {
        // splice _Right [_First, _First + 1) at _Where
#if _ITERATOR_DEBUG_LEVEL == 2
        _STL_VERIFY(_Where._Getcont() == _STD addressof(_Mypair._Myval2)
                        && _First._Getcont() == _STD addressof(_Right._Mypair._Myval2),
            "list splice iterator outside range");
#endif // _ITERATOR_DEBUG_LEVEL == 2
        const auto _UWhere = _Where._Ptr;
        const auto _UFirst = _First._Ptr;

#if _ITERATOR_DEBUG_LEVEL == 2
        if (_UFirst == _Right._Mypair._Myval2._Myhead) {
            _STL_REPORT_ERROR("list splice iterator outside range");
        }
#endif // _ITERATOR_DEBUG_LEVEL == 2

        const auto _ULast = _UFirst->_Next;
        if (this != _STD addressof(_Right) || (_UWhere != _UFirst && _UWhere != _ULast)) {
            _Splice(_UWhere, _Right, _UFirst, _ULast, 1);
        }
    }

    void splice(const const_iterator _Where, list&& _Right, const const_iterator _First) {
        // splice _Right [_First, _First + 1) at _Where
        splice(_Where, _Right, _First);
    }

    void splice(const const_iterator _Where, list& _Right, const const_iterator _First, const const_iterator _Last) {
        // splice _Right [_First, _Last) at _Where
#if _ITERATOR_DEBUG_LEVEL == 2
        const auto _Right_data_ptr = _STD addressof(_Right._Mypair._Myval2);
        _STL_VERIFY(_Where._Getcont() == _STD addressof(_Mypair._Myval2) && _First._Getcont() == _Right_data_ptr
                        && _Last._Getcont() == _Right_data_ptr,
            "list splice iterator outside range");
#endif // _ITERATOR_DEBUG_LEVEL == 2

        const auto _UWhere = _Where._Ptr;
        const auto _UFirst = _First._Ptr;
        const auto _ULast  = _Last._Ptr;

        if (_UFirst != _ULast && (this != _STD addressof(_Right) || _UWhere != _ULast)) { // worth splicing, do it
            size_type _Count = 0;

            if (this != _STD addressof(_Right)) {
                const auto _Right_end = _Right._Mypair._Myval2._Myhead;
                if (_UFirst == _Right_end->_Next && _ULast == _Right_end) {
                    _Count = _Right._Mypair._Myval2._Mysize; // splice in whole list
                } else { // count nodes and check for knot
                    for (auto _To_check = _UFirst; _To_check != _ULast; _To_check = _To_check->_Next, (void) ++_Count) {
#if _ITERATOR_DEBUG_LEVEL != 0
                        _STL_VERIFY(_To_check != _Right_end, "list bad splice");
#endif // _ITERATOR_DEBUG_LEVEL != 0
                    }
                }
            }

            _Splice(_UWhere, _Right, _UFirst, _ULast, _Count);
        }
    }

    void splice(const const_iterator _Where, list&& _Right, const const_iterator _First, const const_iterator _Last) {
        // splice _Right [_First, _Last) at _Where
        splice(_Where, _Right, _First, _Last);
    }

    struct _List_node_remove_op {
        // tracks nodes pending removal in a remove operation, so that program-defined predicates may reference those
        // elements until the removal is complete.

        explicit _List_node_remove_op(list& _List_) noexcept : _List(_List_), _Head(), _Tail(_STD addressof(_Head)) {}

        _List_node_remove_op(const _List_node_remove_op&) = delete;
        _List_node_remove_op& operator=(const _List_node_remove_op&) = delete;

        _Nodeptr _Transfer_back(const _Nodeptr _Removed) noexcept {
            // extract _Removed from the list, and add it to the singly-linked list of nodes to destroy
            // returns the node after _Removed
            _STL_INTERNAL_CHECK(_List._Mypair._Myval2._Myhead != _Removed);

            // snip the node out
            --_List._Mypair._Myval2._Mysize;
            const auto _Next = _STD exchange(_Removed->_Next, _Nodeptr());
            const auto _Prev = _Removed->_Prev;
            _Prev->_Next     = _Next;
            _Next->_Prev     = _Prev;

#if _ITERATOR_DEBUG_LEVEL == 2
            // mark removed node for IDL to snip out later
            _Removed->_Prev = _Nodeptr();
#endif // _ITERATOR_DEBUG_LEVEL == 2

            *_Tail = _Removed;
            _Tail  = _STD addressof(_Removed->_Next);

            return _Next;
        }

        ~_List_node_remove_op() {
            auto& _Al = _List._Getal();

#if _ITERATOR_DEBUG_LEVEL == 2
            {
                _Lockit _Lock(_LOCK_DEBUG);
                _Iterator_base12** _Pnext = &_List._Mypair._Myval2._Myproxy->_Myfirstiter;
                while (*_Pnext) {
                    _Iterator_base12** _Pnextnext = &(*_Pnext)->_Mynextiter;
                    const auto _Pnextptr          = static_cast<const_iterator&>(**_Pnext)._Ptr;
                    if (_Pnextptr->_Prev) {
                        // iterator doesn't point to one of the elements we're removing
                        _Pnext = _Pnextnext;
                    } else { // orphan the iterator
                        (*_Pnext)->_Myproxy = nullptr;
                        *_Pnext             = *_Pnextnext;
                    }
                }
            }
#endif // _ITERATOR_DEBUG_LEVEL == 2

            auto _Target = _Head;
            while (_Target) {
                auto _Next = _Target->_Next;
                _List._Mypair._Myval2._Orphan_ptr2(_Target);
                _Alnode_traits::destroy(_Al, _STD addressof(_Target->_Next));
                _Alnode_traits::destroy(_Al, _STD addressof(_Target->_Prev));
                _Alnode_traits::destroy(_Al, _STD addressof(_Target->_Myval));
                _Al.deallocate(_Target, 1);
                _Target = _Next;
            }
        }

        list& _List;
        _Nodeptr _Head; // singly linked list of nodes to remove; their _Prev pointers set to _Nodeptr()
        _Nodeptr* _Tail;
    };

    auto remove(const _Ty& _Val) { // erase each element matching _Val
        return remove_if([&](const _Ty& _Other) { return _Other == _Val; });
    }

    template <class _Pr1>
    auto remove_if(_Pr1 _Pred) { // erase each element satisfying _Pred
        auto& _My_data = _Mypair._Myval2;
        _List_node_remove_op _Op(*this);
        const auto _Last         = _My_data._Myhead;
        const size_type _Oldsize = _My_data._Mysize;
        for (auto _First = _Last->_Next; _First != _Last;) {
            auto _Next = _First->_Next;
            if (_Pred(_First->_Myval)) {
                _Op._Transfer_back(_First);
            }

            _First = _Next;
        }

#if _HAS_CXX20
        return _Oldsize - _My_data._Mysize;
#else // _HAS_CXX20
        (void) _Oldsize;
#endif // _HAS_CXX20
    }

    auto unique() { // erase each element matching previous
        return unique(equal_to<>());
    }

    template <class _Pr2>
    auto unique(_Pr2 _Pred) { // erase each element satisfying _Pred with previous
        _List_node_remove_op _Op(*this);
        const _Nodeptr _Phead    = _Mypair._Myval2._Myhead;
        _Nodeptr _Pprev          = _Phead->_Next;
        _Nodeptr _Pnode          = _Pprev->_Next;
        const size_type _Oldsize = _Mypair._Myval2._Mysize;
        while (_Pnode != _Phead) {
            if (_Pred(_Pprev->_Myval, _Pnode->_Myval)) { // match, remove it
                _Pnode = _Op._Transfer_back(_Pnode);
            } else { // no match, advance
                _Pprev = _Pnode;
                _Pnode = _Pnode->_Next;
            }
        }

#if _HAS_CXX20
        return _Oldsize - _Mypair._Myval2._Mysize;
#else // _HAS_CXX20
        (void) _Oldsize;
#endif // _HAS_CXX20
    }

    void merge(list& _Right) { // merge in elements from _Right, both ordered by operator<
        _Merge1(_Right, less<>());
    }

    void merge(list&& _Right) { // merge in elements from _Right, both ordered by operator<
        _Merge1(_Right, less<>());
    }

    template <class _Pr2>
    void merge(list& _Right, _Pr2 _Pred) { // merge in elements from _Right, both ordered by _Pred
        _Merge1(_Right, _Pass_fn(_Pred));
    }

    template <class _Pr2>
    void merge(list&& _Right, _Pr2 _Pred) { // merge in elements from _Right, both ordered by _Pred
        _Merge1(_Right, _Pass_fn(_Pred));
    }

private:
    template <class _Pr2>
    void _Merge1(list& _Right, _Pr2 _Pred) { // merge in elements from _Right, both ordered by _Pred
#if _ITERATOR_DEBUG_LEVEL != 0
        _DEBUG_ORDER_UNWRAPPED(_Unchecked_begin(), _Unchecked_end(), _Pred);
#endif // _ITERATOR_DEBUG_LEVEL != 0
        if (this == _STD addressof(_Right)) {
            return;
        }

#if _ITERATOR_DEBUG_LEVEL != 0
        _DEBUG_ORDER_UNWRAPPED(_Right._Unchecked_begin(), _Right._Unchecked_end(), _Pred);
        if _CONSTEXPR_IF (!_Alnode_traits::is_always_equal::value) {
            _STL_VERIFY(_Getal() == _Right._Getal(), "list allocators incompatible for merge");
        }
#endif // _ITERATOR_DEBUG_LEVEL != 0

        const auto _Right_size = _Right._Mypair._Myval2._Mysize;
        if (_Right_size == 0) {
            // nothing to do
            return;
        }

        // splice all _Right's nodes on the end of *this
        const auto _Myhead     = _Mypair._Myval2._Myhead;
        const auto _Right_head = _Right._Mypair._Myval2._Myhead;
        const auto _Mid        = _Right_head->_Next;
        _Splice(_Myhead, _Right, _Mid, _Right_head, _Right_size);

        // if *this had any elements, run the merge op between the range we just spliced and the old elements
        if (_Myhead->_Next != _Mid) {
            _Scary_val::_Merge_same(_Myhead->_Next, _Mid, _Myhead, _Pred);
        }
    }

public:
    void sort() { // order sequence, using operator<
        sort(less<>());
    }

    template <class _Pr2>
    void sort(_Pr2 _Pred) { // order sequence, using _Pred
        auto& _My_data = _Mypair._Myval2;
        _Scary_val::_Sort(_My_data._Myhead->_Next, _My_data._Mysize, _Pass_fn(_Pred));
    }

    void reverse() noexcept { // reverse sequence
        const _Nodeptr _Phead = _Mypair._Myval2._Myhead;
        _Nodeptr _Pnode       = _Phead;

        for (;;) { // flip pointers in a node
            const _Nodeptr _Pnext = _Pnode->_Next;
            _Pnode->_Next         = _Pnode->_Prev;
            _Pnode->_Prev         = _Pnext;

            if (_Pnext == _Phead) {
                break;
            }

            _Pnode = _Pnext;
        }
    }

private:
    _Nodeptr _Splice(
        const _Nodeptr _Where, list& _Right, const _Nodeptr _First, const _Nodeptr _Last, const size_type _Count) {
        // splice _Right [_First, _Last) before _Where; returns _Last
        if (this != _STD addressof(_Right)) { // splicing from another list, adjust counts
#if _ITERATOR_DEBUG_LEVEL != 0
            if _CONSTEXPR_IF (!_Alnode_traits::is_always_equal::value) {
                _STL_VERIFY(_Getal() == _Right._Getal(), "list allocators incompatible for splice");
            }
#endif // _ITERATOR_DEBUG_LEVEL != 0

            auto& _My_data = _Mypair._Myval2;
            if (max_size() - _My_data._Mysize < _Count) {
                _Xlength_error("list too long");
            }

            auto& _Right_data = _Right._Mypair._Myval2;
#if _ITERATOR_DEBUG_LEVEL == 2
            // transfer ownership
            if (_Count == 1) {
                _My_data._Adopt_unique(_Right_data, _First);
            } else if (_Count == _Right_data._Mysize) {
                _My_data._Adopt_all(_Right_data);
            } else {
                _My_data._Adopt_range(_Right_data, _First, _Last);
            }
#endif // _ITERATOR_DEBUG_LEVEL == 2

            _My_data._Mysize += _Count;
            _Right_data._Mysize -= _Count;
        }

        return _Scary_val::_Unchecked_splice(_Where, _First, _Last);
    }

    void _Alloc_sentinel_and_proxy() {
        // _Alnode& _Getal(){}   _Alnode
        // _Alnode = allocator<_Node>
        //cout << _Is_default_allocator<_Alloc>::value << endl;
        //cout << _Is_default_allocator<_Alnode>::value << endl;
        // static_cast<_Rebind_alloc_t<_Alty, _Container_proxy>>(_Al)

        /////////////old//////////////////
        //auto&& _Alproxy = _GET_PROXY_ALLOCATOR(_Alnode, _Getal());      // std::allocator<std::_Container_proxy> &&
        /////////////new//////////////////
        using _Alproxy_cls = _Rebind_alloc_t<_Alnode, _Container_proxy>;
        auto _Alproxy_left = static_cast<_Alproxy_cls>(_Getal());
        auto&& _Alproxy = _Alproxy_left;
        //cout << typeid(_Alproxy_cls).name() << endl;                        // allocator<_Container_proxy>
        //cout << typeid(_Alproxy_left).name() << endl;                       // allocator<_Container_proxy>
        /////////////new//////////////////
        // 类型

        _Container_proxy_ptr<_Alty> _Proxy(_Alproxy, _Mypair._Myval2);
        //cout << typeid(_Proxy).name() << endl;                       // _Container_proxy_ptr12< allocator<_Container_proxy> >

        auto& _Al     = _Getal();
        auto _Newhead = _Al.allocate(1);
        _Construct_in_place(_Newhead->_Next, _Newhead);
        _Construct_in_place(_Newhead->_Prev, _Newhead);
        _Mypair._Myval2._Myhead = _Newhead;
        _Proxy._Release();
    }

    void _Orphan_all() noexcept {
        _Mypair._Myval2._Orphan_all();
    }

    _Alnode& _Getal() noexcept {
        return _Mypair._Get_first();
    }

    const _Alnode& _Getal() const noexcept {
        return _Mypair._Get_first();
    }

    _Compressed_pair<_Alnode, _Scary_val> _Mypair;
};

#if _HAS_CXX17
template <class _Iter, class _Alloc = allocator<_Iter_value_t<_Iter>>,
    enable_if_t<conjunction_v<_Is_iterator<_Iter>, _Is_allocator<_Alloc>>, int> = 0>
list(_Iter, _Iter, _Alloc = _Alloc()) -> list<_Iter_value_t<_Iter>, _Alloc>;
#endif // _HAS_CXX17

template <class _Ty, class _Alloc>
void swap(list<_Ty, _Alloc>& _Left, list<_Ty, _Alloc>& _Right) noexcept /* strengthened */ {
    _Left.swap(_Right);
}

template <class _Ty, class _Alloc>
_NODISCARD bool operator==(const list<_Ty, _Alloc>& _Left, const list<_Ty, _Alloc>& _Right) {
    return _Left.size() == _Right.size() && _STD equal(_Left.begin(), _Left.end(), _Right.begin());
}

template <class _Ty, class _Alloc>
_NODISCARD bool operator!=(const list<_Ty, _Alloc>& _Left, const list<_Ty, _Alloc>& _Right) {
    return !(_Left == _Right);
}

template <class _Ty, class _Alloc>
_NODISCARD bool operator<(const list<_Ty, _Alloc>& _Left, const list<_Ty, _Alloc>& _Right) {
    return _STD lexicographical_compare(_Left.begin(), _Left.end(), _Right.begin(), _Right.end());
}

template <class _Ty, class _Alloc>
_NODISCARD bool operator>(const list<_Ty, _Alloc>& _Left, const list<_Ty, _Alloc>& _Right) {
    return _Right < _Left;
}

template <class _Ty, class _Alloc>
_NODISCARD bool operator<=(const list<_Ty, _Alloc>& _Left, const list<_Ty, _Alloc>& _Right) {
    return !(_Right < _Left);
}

template <class _Ty, class _Alloc>
_NODISCARD bool operator>=(const list<_Ty, _Alloc>& _Left, const list<_Ty, _Alloc>& _Right) {
    return !(_Left < _Right);
}

#if _HAS_CXX20
template <class _Ty, class _Alloc, class _Uty>
typename list<_Ty, _Alloc>::size_type erase(list<_Ty, _Alloc>& _Cont, const _Uty& _Val) {
    return _Cont.remove_if([&](_Ty& _Elem) { return _Elem == _Val; });
}

template <class _Ty, class _Alloc, class _Pr>
typename list<_Ty, _Alloc>::size_type erase_if(list<_Ty, _Alloc>& _Cont, _Pr _Pred) {
    return _Cont.remove_if(_Pass_fn(_Pred));
}
#endif // _HAS_CXX20

#if _HAS_CXX17
namespace pmr {
    template <class _Ty>
    using list = _STD list<_Ty, polymorphic_allocator<_Ty>>;
} // namespace pmr
#endif // _HAS_CXX17
_STD_END
#pragma pop_macro("new")
_STL_RESTORE_CLANG_WARNINGS
#pragma warning(pop)
#pragma pack(pop)
#endif // _STL_COMPILER_PREPROCESSOR
#endif // _LIST_
