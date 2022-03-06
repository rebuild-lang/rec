#include "CoRoutine.h"
// #include <experimental/coroutine>

#if not defined(__cpp_lib_coroutine)

#    if defined(__GNUC__)
using procedure_t = void (*)(void*);
#    else
using procedure_t = void(__cdecl*)(void*);
#    endif

constexpr auto align_req_v = sizeof(void*) * 2;
template<typename P> constexpr auto aligned_size_v = (sizeof(P) + align_req_v - 1u) & ~(align_req_v - 1u);

/**
 * @brief Simply calculate aligned size of the type. It is multiplier of 16
 */
constexpr ptrdiff_t portable_aligned_size(size_t _TypeSize) {
    return (_TypeSize + align_req_v - 1u) & ~(align_req_v - 1u);
}

/**
 * @brief Clang coroutine frame's prefix
 * @details
 * The layout is like the following
 * ```
 * +------------------+------------+---+--------------------+
 * | Frame Prefix(16) | Promise(?) | ? | Local variables(?) |
 * +------------------+------------+---+--------------------+
 * ```
 */
struct clang_frame_prefix {
    procedure_t _Factivate;
    procedure_t _Fdestroy;
};
// static_assert(aligned_size_v<clang_frame_prefix> == 16);

/**
 * @brief GCC coroutine frame's prefix
 * @details
 * The layout is unknown
 */
using gcc_frame_prefix = clang_frame_prefix;

// - Note
//      MSVC coroutine frame's prefix
//      Reference <experimental/resumable> for the detail
// - Layout

/**
 * @brief MSVC coroutine frame's prefix
 * @details
 * The layout is like the following
 * ```
 * +------------+------------------+--------------------+
 * | Promise(?) | Frame Prefix(16) | Local variables(?) |
 * +------------+------------------+--------------------+
 * ```
 * @see Reference <experimental/resumable>
 */
struct msvc_frame_prefix {
    procedure_t _Factivate;
    uint16_t _Index;
    uint16_t _Flag;
};
// static_assert(aligned_size_v<msvc_frame_prefix> == 16);

//
// intrinsic: MSVC
//
extern "C" {
size_t _coro_resume(void*);
void _coro_destroy(void*);
size_t _coro_done(void*);
}
//
// intrinsic: Clang/GCC
//
extern "C" {
bool __builtin_coro_done(void*);
void __builtin_coro_resume(void*);
void __builtin_coro_destroy(void*);
// void* __builtin_coro_promise(void* ptr, int align, bool p);
}

bool _coro_finished(portable_coro_prefix* _Handle);

#    if defined(__clang__)
static constexpr auto is_clang = true;
static constexpr auto is_msvc = !is_clang;

struct portable_coro_prefix final : public clang_frame_prefix {};

#    elif defined(_MSC_VER)
static constexpr auto is_msvc = true;
static constexpr auto is_clang = !is_msvc;

#        pragma intrinsic(_coro_resume)
#        pragma intrinsic(_coro_destroy)
#        pragma intrinsic(_coro_done)

struct portable_coro_prefix final : public msvc_frame_prefix {};

inline bool _coro_finished(portable_coro_prefix* _Handle) { return _Handle->_Index == 0; }

#    elif defined(__GNUC__)
// For now, work like a clang
static constexpr auto is_clang = true;
static constexpr auto is_msvc = !is_clang;

struct portable_coro_prefix final : public clang_frame_prefix {};

extern "C" {
bool __builtin_coro_is_suspended(void*);
}

#    endif // __clang__ || _MSC_VER || __GNUC__

// replacement of the `_coro_done`
bool portable_coro_done(portable_coro_prefix* _Handle) {
    if constexpr (is_msvc) {
        return _coro_finished(_Handle);
    }
    else if constexpr (is_clang) {
        return __builtin_coro_done(_Handle);
    }
    return false; // follow `noop_coroutine`
}

void portable_coro_resume(portable_coro_prefix* _Handle) {
    if constexpr (is_msvc) {
        _coro_resume(_Handle);
    }
    else if constexpr (is_clang) {
        __builtin_coro_resume(_Handle);
    }
}

void portable_coro_destroy(portable_coro_prefix* _Handle) {
    if constexpr (is_msvc) {
        _coro_destroy(_Handle);
    }
    else if constexpr (is_clang) {
        __builtin_coro_destroy(_Handle);
    }
}

/**
 * @brief 'get_promise' from the frame prefix
 */
void* portable_coro_get_promise(portable_coro_prefix* _Handle, ptrdiff_t _PromSize) {
    // location of the promise object
    void* _PromAddr = nullptr;

    if constexpr (is_clang) {
        // for Clang, promise is placed just after frame prefix
        // see also: `__builtin_coro_promise`
        _PromAddr = reinterpret_cast<std::byte*>(_Handle) + aligned_size_v<clang_frame_prefix>;
    }
    else if constexpr (is_msvc) {
        // for MSVC, promise is placed before frame prefix
        _PromAddr = reinterpret_cast<std::byte*>(_Handle) - portable_aligned_size(_PromSize);
    }
    return _PromAddr;
}

/**
 * @brief Get the frame prefix 'from_promise'
 */
portable_coro_prefix* portable_coro_from_promise(void* _PromAddr, ptrdiff_t _PromSize) {
    // location of the frame prefix
    void* _Handle = nullptr;

    if constexpr (is_clang) {
        _Handle = reinterpret_cast<std::byte*>(_PromAddr) - aligned_size_v<clang_frame_prefix>;
    }
    else if constexpr (is_msvc) {
        _Handle = reinterpret_cast<std::byte*>(_PromAddr) + portable_aligned_size(_PromSize);
    }
    return reinterpret_cast<portable_coro_prefix*>(_Handle);
}

#endif
