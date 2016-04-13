#ifndef PARAMETER_PACK_H
#define PARAMETER_PACK_H

/// \brief Store a parameter pack as a single type to be applied later to another type.
/// \tparam Args The type to store in the parameter pack.
template <typename ...Args>
struct parameter_pack_type {

    /// \brief Apply the stored types of parameter_pack_type to \p T
    /// \tparam T The type to which to apply the parameter pack.
    template <template<typename ...> class T>
    struct apply {
        using type = T<Args...>;
    };
    /// \brief Shorthand for apply<T>::type.
    /// \tparam T The type to which to apply the parameter pack.
    template <template<typename ...> class T>
    using apply_t = typename apply<T>::type;
};

/// \brief Store a parameter pack as a single type to be applied later to another type.
/// \tparam FirstArg The type of the first argument stored in the parameter pack.
/// \tparam Args The type of the rest of the arguments in the parameter pack.
template <typename FirstArg, typename ...Args>
struct parameter_pack_type<FirstArg, Args...> {
    /// \brief Apply the stored types of parameter_pack_type to \p T
    /// \tparam T The type to which to apply the parameter pack.
    template <template<typename ...> class T>
    struct apply {
        using type = T<FirstArg, Args...>;
    };
    /// \brief Shorthand for apply<T>::type.
    /// \tparam T The type to which to apply the parameter pack.
    template <template<typename ...> class T>
    using apply_t = typename apply<T>::type;

    /// \brief Apply the stored types of parameter_pack_type as a function pointer to \p T.
    ///
    /// Takes the stored types Arg1, Arg2, ..., ArgN and create the type 'Arg1(Arg2, ..., ArgN)'.
    using fn_type = FirstArg(Args...);
};

#endif
