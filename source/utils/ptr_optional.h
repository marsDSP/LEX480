#pragma once

#ifndef LEX480_PTR_OPT_H
#define LEX480_PTR_OPT_H

#include <memory>
#include <cstdint>
#include <utility>
#include <type_traits>
#include <JuceHeader.h>

#include "ptr_packed.h"

namespace MarsDSP::Utils
{
    template <typename T>
    struct OptionalPointer
    {
        using element_type = T;

        OptionalPointer() = default;

        explicit OptionalPointer (T* ptr, bool shouldOwn = true)
            : pointer (ptr, shouldOwn ? Owning : NonOwning)
        {
        }

        template <typename Arg1, typename... Args, typename = std::enable_if_t<! (std::is_pointer_v<Arg1> && std::is_base_of_v<T, std::remove_pointer_t<Arg1>>)>>
        explicit OptionalPointer (Arg1&& arg1, Args&&... args)
            : pointer (new T { std::forward<Arg1> (arg1), std::forward<Args> (args)... }, Owning)
        {
        }

        void setOwning (T* ptr)
        {
            invalidate();
            pointer.set (ptr, Owning);
        }

        void setOwning (std::unique_ptr<T>&& ptr)
        {
            invalidate();
            pointer.set (ptr.release(), Owning);
        }

        void setNonOwning (T* ptr)
        {
            pointer.set (ptr, NonOwning);
        }

        OptionalPointer (OptionalPointer&& other) noexcept
        {
            invalidate();
            pointer.swap (other.pointer);
        }

        OptionalPointer& operator= (OptionalPointer&& other) noexcept
        {
            invalidate();
            pointer.swap (other.pointer);
            return *this;
        }

        OptionalPointer (const OptionalPointer&) = delete;
        OptionalPointer& operator= (const OptionalPointer&) = delete;

        ~OptionalPointer()
        {
            invalidate();
        }

        [[nodiscard]] bool isOwner() const noexcept { return pointer.get_flags() == Owning; }

        [[nodiscard]] T* release()
        {
            jassert (isOwner());
            pointer.set_flags (NonOwning);
            return pointer.get_ptr();
        }

        void invalidate()
        {
            if (isOwner())
                delete pointer.get_ptr();
            pointer.set (nullptr);
        }

        [[nodiscard]] T* get() { return pointer.get_ptr(); }
        [[nodiscard]] const T* get() const { return pointer.get_ptr(); }

        [[nodiscard]] operator T&() { return *pointer; }
        [[nodiscard]] T* operator->() { return pointer.get_ptr(); }
        [[nodiscard]] const T* operator->() const { return pointer.get_ptr(); }
        [[nodiscard]] T& operator*() { return *pointer; }
        [[nodiscard]] const T& operator*() const { return *pointer; }

    private:
        enum : uint8_t
        {
            NonOwning = 0,
            Owning = 1,
        };

        PackedPointer<T> pointer {};
    };

    template <typename T>
    bool operator== (const OptionalPointer<T>& p1, const OptionalPointer<T>& p2)
    {
        return p1.get() == p2.get();
    }

    template <typename T>
    bool operator!= (const OptionalPointer<T>& p1, const OptionalPointer<T>& p2)
    {
        return p1.get() != p2.get();
    }

    template <typename T>
    bool operator== (const OptionalPointer<T>& p1, const std::unique_ptr<T>& p2)
    {
        return p1.get() == p2.get();
    }

    template <typename T>
    bool operator!= (const OptionalPointer<T>& p1, const std::unique_ptr<T>& p2)
    {
        return p1.get() != p2.get();
    }

    template <typename T>
    bool operator== (const std::unique_ptr<T>& p1, const OptionalPointer<T>& p2)
    {
        return p1.get() == p2.get();
    }

    template <typename T>
    bool operator!= (const std::unique_ptr<T>& p1, const OptionalPointer<T>& p2)
    {
        return p1.get() != p2.get();
    }

    template <typename T>
    bool operator== (const OptionalPointer<T>& p1, std::nullptr_t)
    {
        return p1.get() == nullptr;
    }

    template <typename T>
    bool operator!= (const OptionalPointer<T>& p1, std::nullptr_t)
    {
        return p1.get() != nullptr;
    }

    template <typename T>
    bool operator== (std::nullptr_t, const OptionalPointer<T>& p2)
    {
        return nullptr == p2.get();
    }

    template <typename T>
    bool operator!= (std::nullptr_t, const OptionalPointer<T>& p2)
    {
        return nullptr != p2.get();
    }

    #ifndef DOXYGEN
    template <class T>
    struct IsOptionalPointerType : std::false_type
    {
    };

    template <class T>
    struct IsOptionalPointerType<OptionalPointer<T>> : std::true_type
    {
    };
    #endif

    /*
    template <typename T>
    static constexpr bool IsOptionalPointer = is_specialization_of_v<T, OptionalPointer>;
    */
}
#endif