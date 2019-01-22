#pragma once
#include<cmath>
#include<type_traits>
#include<iostream>

/**
* Real allows the user to find numerical code sections in which the error of using
* single precision over double precision floating point is unbearable, without any need
* for code change. Real can also be used either as a single precision or double
* precision floating point variable, without performing background comparison, and without
* any extra run time / memory cost over the usage of a standard float or double type variables,
* when using O3 optimization.
*
* Usage is very simple:
* > to be used as a standard float or double variable: 
*   "FP::Real<FP::Precision::Single> a;" is the equivalent of "float a".
*   "FP::Real<FP::Precision::Double> a;" is the equivalent of "double a".
* > to be used for accuracy analysis, declare:
*   FP::Real<FP::Precision::Single, FP::Compare::Yes> a;
*   and 'a' will include both a single and double precision variables, whose error can be examined
*   any time by either writing the variable to the console (using << operator) or using the error() method.
*
* Dan Israel Malta
**/
namespace FP {

    // floating point variable precision to be used in calculations
    enum class Precision : bool {
        Single = true,      // single precision (32bit)
        Double = false      // double precision (64bit)
    };

    // should a comparison between single and double precision be made?
    enum class Compare : bool {
        Yes = true,         // do not compare double to single
        No  = false         // perform comparison between double & single precision
    };

    /**
    * \brief an object which can be either float or double, and if it is a float - can be used for
    *        numerical accuracy estimation.
    *
    * @param {Precision, in} precision of 'used' type (either single or double; single by default)
    * @param {Compare,   in} should an accuracy comparison be made between single & double precision? (no by default)
    **/
    template<Precision PRECISION = Precision::Single, Compare COMPARE = Compare::No> class Real {

        // internal helpers
        private:

            // extract an enum class underlying type
            template<typename ENUM> static constexpr typename std::underlying_type<ENUM>::type ToType(ENUM xi_enum) { 
                return static_cast<typename std::underlying_type<ENUM>::type>(xi_enum);
            }

        // aliases needed outside this class
        public:
            // alias for underlying type
            using TYPE = typename std::conditional<ToType(PRECISION), float, double>::type;

        // internal structure
        private:

            // structure holding a floating point variable to be used regularly
            struct regularStruct {
                // properties
                TYPE _value;    // used value

                // constructor
                constexpr regularStruct() = default;
                constexpr regularStruct(TYPE v)                     : _value(v)                   {}
                constexpr regularStruct(const regularStruct& c)     : _value(c._value)            {}
                constexpr regularStruct(regularStruct&& c) noexcept : _value(std::move(c._value)) {}

                template<typename U, typename = std::enable_if_t<std::is_arithmetic<U>::value>>
                constexpr regularStruct(const U v) : _value(static_cast<double>(v)) {}

                // assignment
                constexpr regularStruct& operator=(const regularStruct& c) {
                    _value = c._value;
                    return *this;
                }
                constexpr regularStruct& operator=(regularStruct&& c) noexcept {
                    _value = std::move(c._value);
                    return *this;
                }

                template<typename U, typename = std::enable_if_t<std::is_arithmetic<U>::value>>
                constexpr regularStruct& operator=(const U v) {
                    _value = static_cast<TYPE>(v);
                    return *this;
                }
            };

            // structure holding both single and double floating point variables for comparison
            struct compareStruct {
                // properties
                TYPE _value;        // used value
                double _exact;      // 'exact' value (for comparison)

                // constructor
                constexpr compareStruct() = default;
                constexpr compareStruct(const TYPE v)               : _value(v),                   _exact(static_cast<double>(v)) {}         
                constexpr compareStruct(const compareStruct& c)     : _value(c._value),            _exact(c._exact)               {}
                constexpr compareStruct(compareStruct&& c) noexcept : _value(std::move(c._value)), _exact(std::move(c._exact))    {}

                template<typename U, typename = std::enable_if_t<std::is_arithmetic<U>::value>>
                constexpr compareStruct(const TYPE v, const U e) : _value(v), _exact(static_cast<double>(e)) {}

                // assignment
                constexpr compareStruct& operator=(const compareStruct& c) {
                    _value = c._value;
                    _exact = c._exact;
                    return *this;
                }
                constexpr compareStruct& operator=(compareStruct&& c) noexcept {
                    _value = std::move(c._value);
                    _exact = std::move(c._exact);
                    return *this;
                }

                template<typename U, typename = std::enable_if_t<std::is_arithmetic<U>::value>>
                constexpr compareStruct& operator=(const U v) {
                    _value = static_cast<TYPE>(v);
                    _exact = static_cast<double>(v);
                    return *this;
                }
            };

        // properties
        private:

            // alias for functional type
            using PROPERTY_TYPE = typename std::conditional<ToType(COMPARE), compareStruct, regularStruct>::type;

            // used value
            PROPERTY_TYPE m_value;

        // constructor
        public:

            // constructor for non comparison analysis
            constexpr Real()       : m_value()  {}
            constexpr Real(TYPE v) : m_value(v) {}

            // constructor for comparison analysis
            template<typename U, Compare C = COMPARE, typename = std::enable_if_t<(C == Compare::Yes) && std::is_arithmetic<U>::value>>
            constexpr Real(const TYPE v, const U e) : m_value(v, e) {}

            // copy/move semantics
            constexpr Real(const Real& r)     : m_value(r.m_value)            {}
            constexpr Real(Real&& r) noexcept : m_value(std::move(r.m_value)) {}

            // assignment
            constexpr Real& operator=(const Real& r) {
                m_value = r.m_value;
                return *this;
            }

            constexpr Real& operator=(Real&& r) noexcept {
                m_value = std::move(r.m_value);
                return *this;
            }

            template<typename U, typename = std::enable_if_t<std::is_arithmetic<U>::value>>
            constexpr Real& operator=(const U v) {
                m_value = static_cast<TYPE>(v);
                return *this;
            }

        // getters
        public:

            // return underlying struct
            constexpr TYPE value() const noexcept { return m_value._value; }

            // return 'exact' value
            template<Compare T = COMPARE, typename = std::enable_if_t<T == Compare::Yes>>
            constexpr double exact() const noexcept { return m_value._exact; }

            // return error between double and single precision calculations
            template<Compare T = COMPARE, typename = std::enable_if<T == Compare::Yes>>
            constexpr double error() const noexcept { return (exact() - static_cast<double>(value())); }

        // stream
        public:

            friend std::ostream& operator<<(std::ostream& xio_stream, const Real& real) {
                if constexpr (COMPARE == Compare::Yes) {
                    return xio_stream << "{value = " << std::to_string(real.value())
                                      << ", exact value = " << std::to_string(real.exact())
                                      << ", error = " << std::to_string(real.error()) << "}";
                }
                else {
                    return xio_stream << std::to_string(real.value());
                }
            }

        // numerical assignment operator overloading
        public:

#define M_ASSIGNMENT_OPERATOR(OP)                                     \
            constexpr Real& operator OP (const TYPE xi_value) {       \
                m_value._value OP xi_value;                           \
                                                                      \
                if constexpr (COMPARE == Compare::Yes) {              \
                    m_value._exact OP static_cast<double>(xi_value);  \
                }                                                     \
                                                                      \
                return *this;                                         \
            }                                                         \
            constexpr Real& operator OP (const Real& xi_value) {      \
                m_value._value OP xi_value.value();                   \
                                                                      \
                if constexpr (COMPARE == Compare::Yes) {              \
                    m_value._exact OP xi_value.exact();               \
                }                                                     \
                                                                      \
                return *this;                                         \
            }                                                         \
            constexpr Real& operator OP (Real&& xi_value) {           \
                m_value._value OP std::move(xi_value.value());        \
                                                                      \
                if constexpr (COMPARE == Compare::Yes) {              \
                    m_value._exact OP std::move(xi_value.exact());    \
                }                                                     \
                                                                      \
                return *this;                                         \
            }

            M_ASSIGNMENT_OPERATOR(+=);
            M_ASSIGNMENT_OPERATOR(-=);
            M_ASSIGNMENT_OPERATOR(*=);
            M_ASSIGNMENT_OPERATOR(/=);

#undef M_ASSIGNMENT_OPERATOR
    };
    
    // --- unary numerical operator overload ---
    template<Precision P, Compare C> constexpr inline Real<P, C> operator - (const Real<P, C>& r) {
        if constexpr (C == Compare::Yes) {
            return Real<P, C>(-r.value(), -r.exact());
        }
        else {
            return Real<P, C>(-r.value());
        }
    }

    // --- binary numerical operator overload ---
#define M_NUMERICAL_OPERATOR(OP)                                                                                                                     \
    template<Precision P, Compare C> constexpr inline Real<P, C> operator OP (const Real<P, C>& xi_lhs, const typename Real<P, C>::TYPE xi_rhs) {    \
        if constexpr (C == Compare::Yes) {                                                                                                           \
            return Real<P, C>(xi_lhs.value() OP xi_rhs, xi_lhs.exact() OP static_cast<double>(xi_rhs));                                              \
        }                                                                                                                                            \
        else {                                                                                                                                       \
            return Real<P, C>(xi_lhs.value() OP xi_rhs);                                                                                             \
        }                                                                                                                                            \
    }                                                                                                                                                \
    template<Precision P, Compare C> constexpr inline Real<P, C> operator OP (const typename Real<P, C>::TYPE xi_lhs, const Real<P, C>& xi_rhs) {    \
        if constexpr (C == Compare::Yes) {                                                                                                           \
            return Real<P, C>(xi_rhs.value() OP xi_lhs, xi_rhs.exact() OP static_cast<double>(xi_lhs));                                              \
        }                                                                                                                                            \
        else {                                                                                                                                       \
            return Real<P, C>(xi_rhs.value() OP xi_lhs);                                                                                             \
        }                                                                                                                                            \
    }                                                                                                                                                \
    template<Precision P, Compare C> constexpr inline Real<P, C> operator OP (const Real<P, C>& xi_lhs, const Real<P, C>& xi_rhs) {                  \
        if constexpr (C == Compare::Yes) {                                                                                                           \
            return Real<P, C>(xi_lhs.value() OP xi_rhs.value(), xi_lhs.exact() OP xi_rhs.exact());                                                   \
        }                                                                                                                                            \
        else {                                                                                                                                       \
            return Real<P, C>(xi_lhs.value() OP xi_rhs.value());                                                                                     \
        }                                                                                                                                            \
    }
    
    M_NUMERICAL_OPERATOR(+);
    M_NUMERICAL_OPERATOR(-);
    M_NUMERICAL_OPERATOR(*);
    M_NUMERICAL_OPERATOR(/);

#undef M_NUMERICAL_OPERATOR

    // --- relational operator overload ---

#define M_RELATIONAL_OPERATOR(OP)                                                                                                                  \
    template<Precision P, Compare C> constexpr inline bool operator OP (const Real<P, C>& xi_lhs, const Real<P, C>& xi_rhs) {                      \
        return (xi_lhs.value() OP xi_rhs.value());                                                                                                 \
    }                                                                                                                                              \
    template<Precision P, Compare C> constexpr inline bool operator OP (const Real<P, C>& xi_lhs, const typename Real<P, C>::TYPE xi_rhs) {        \
        return (xi_lhs.value() OP xi_rhs);                                                                                                         \
    }                                                                                                                                              \
    template<Precision P, Compare C> constexpr inline bool operator OP (const typename Real<P, C>::TYPE xi_lhs, const Real<P, C>& xi_rhs) {        \
        return (xi_lhs OP xi_rhs.value());                                                                                                         \
    }

    M_RELATIONAL_OPERATOR(==);
    M_RELATIONAL_OPERATOR(!=);
    M_RELATIONAL_OPERATOR(>);
    M_RELATIONAL_OPERATOR(>=);
    M_RELATIONAL_OPERATOR(<);
    M_RELATIONAL_OPERATOR(<=);

#undef M_RELATIONAL_OPERATOR

    // --- standard unary numerical functions ---
#define M_UNARY_FUNCTION(NAME, OPERATION)                                                                             \
    template<Precision P, Compare C> constexpr inline Real<P, C> NAME(const Real<P, C>& r) noexcept {                 \
        if constexpr (C == Compare::Yes) {                                                                            \
            return Real<P, C>(OPERATION(r.value()), OPERATION(r.exact()));                                            \
        }                                                                                                             \
        else {                                                                                                        \
            return Real<P, C>(OPERATION(r.value()));                                                                  \
        }                                                                                                             \
    }                                                                                                                 \
    template<Precision P, Compare C> constexpr inline Real<P, C> NAME(const typename Real<P, C>::TYPE r) noexcept {   \
        if constexpr (C == Compare::Yes) {                                                                            \
            return Real<P, C>(OPERATION(r), OPERATION(static_cast<double>(r)));                                       \
        }                                                                                                             \
        else {                                                                                                        \
            return Real<P, C>(OPERATION(r));                                                                          \
        }                                                                                                             \
    }
    M_UNARY_FUNCTION(abs, std::abs);
    M_UNARY_FUNCTION(floor, std::floor);
    M_UNARY_FUNCTION(ceil, std::ceil);
    M_UNARY_FUNCTION(round, std::round);
    M_UNARY_FUNCTION(rint, std::rint);
    M_UNARY_FUNCTION(trunc, std::trunc);
    M_UNARY_FUNCTION(sqrt, std::sqrt);
    M_UNARY_FUNCTION(cbrt, std::cbrt);
    M_UNARY_FUNCTION(exp, std::exp);
    M_UNARY_FUNCTION(exp2, std::exp2);
    M_UNARY_FUNCTION(log, std::log);
    M_UNARY_FUNCTION(log2, std::log2);
    M_UNARY_FUNCTION(log10, std::log10);
    M_UNARY_FUNCTION(log1p, std::log1p);
    M_UNARY_FUNCTION(sin, std::sin);
    M_UNARY_FUNCTION(cos, std::cos);
    M_UNARY_FUNCTION(tan, std::tan);
    M_UNARY_FUNCTION(asin, std::asin);
    M_UNARY_FUNCTION(acos, std::acos);
    M_UNARY_FUNCTION(atan, std::atan);
    M_UNARY_FUNCTION(sinh, std::sinh);
    M_UNARY_FUNCTION(cosh, std::cosh);
    M_UNARY_FUNCTION(tanh, std::tanh);
    M_UNARY_FUNCTION(asinh, std::asinh);
    M_UNARY_FUNCTION(acosh, std::acosh);
    M_UNARY_FUNCTION(atanh, std::atanh);
    M_UNARY_FUNCTION(erf, std::erf);
    M_UNARY_FUNCTION(erfc, std::erfc);
    M_UNARY_FUNCTION(tgamma, std::tgamma);
    M_UNARY_FUNCTION(lgamma, std::lgamma);

#undef M_UNARY_FUNCTION

    // --- binary Real-TYPE numerical functions ---
    template<Precision P, Compare C> constexpr inline Real<P, C> pow(const Real<P, C>& r, const typename Real<P, C>::TYPE p) noexcept {
        if constexpr (C == Compare::Yes) {
            return Real<P, C>(std::pow(r.value(), p), std::pow(r.exact(), static_cast<double>(p)));
        }
        else {
            return Real<P, C>(std::pow(r.value(), p));
        }
    }

    // --- standard binary Real-Real & Real-TYPE numerical functions ---
#define M_BINARY_FUNCTION(NAME, OPERATION)                                                                                                              \
    template<Precision P, Compare C> constexpr inline Real<P, C> NAME(const Real<P, C>& l, const Real<P, C>& r) noexcept {                              \
        if constexpr (C == Compare::Yes) {                                                                                                              \
            return Real<P, C>(OPERATION(l.value(), r.value()), OPERATION(l.exact(), r.exact()));                                                        \
        }                                                                                                                                               \
        else {                                                                                                                                          \
            return Real<P, C>(OPERATION(l.value(), r.value()));                                                                                         \
        }                                                                                                                                               \
    }                                                                                                                                                   \
    template<Precision P, Compare C> constexpr inline Real<P, C> NAME(const Real<P, C>& l, const typename Real<P, C>::TYPE p) noexcept {                \
        if constexpr (C == Compare::Yes) {                                                                                                              \
            return Real<P, C>(OPERATION(l.value(), p), OPERATION(l.exact(), static_cast<double>(p)));                                                   \
        }                                                                                                                                               \
        else {                                                                                                                                          \
            return Real<P, C>(OPERATION(l.value(), p));                                                                                                 \
        }                                                                                                                                               \
    }                                                                                                                                                   \
    template<Precision P, Compare C> constexpr inline Real<P, C> NAME(const typename Real<P, C>::TYPE p, const Real<P, C>& r) noexcept {                \
        if constexpr (C == Compare::Yes) {                                                                                                              \
            return Real<P, C>(OPERATION(p, r.value()), OPERATION(static_cast<double>(p), r.exact()));                                                   \
        }                                                                                                                                               \
        else {                                                                                                                                          \
            return Real<P, C>(OPERATION(p, r.value()));                                                                                                 \
        }                                                                                                                                               \
    }                                                                                                                                                   \
    template<Precision P, Compare C> constexpr inline Real<P, C> NAME(const typename Real<P, C>::TYPE p, const typename Real<P, C>::TYPE r) noexcept {  \
        if constexpr (C == Compare::Yes) {                                                                                                              \
            return Real<P, C>(OPERATION(p, r), OPERATION(static_cast<double>(p), static_cast<double>(r)));                                              \
        }                                                                                                                                               \
        else {                                                                                                                                          \
            return Real<P, C>(OPERATION(p, r));                                                                                                         \
        }                                                                                                                                               \
    }

    M_BINARY_FUNCTION(hypot, std::hypot);
    M_BINARY_FUNCTION(atan2, std::atan2);
    M_BINARY_FUNCTION(reminder, std::reminder);
    M_BINARY_FUNCTION(fmod, std::fmod);

#undef M_BINARY_FUNCTION

    // --- non standard numeric operations ---

    // return the sign of a Real
    template<Precision P, Compare C> constexpr inline Real<P, C> sign(const Real<P, C>& r) noexcept {
        if constexpr (C == Compare::Yes) {
            typename Real<P, C>::TYPE v{ r.value() < static_cast<typename Real<P, C>::TYPE>(0.0) ? 
                                         static_cast<typename Real<P, C>::TYPE>(-1.0) : 
                                         static_cast<typename Real<P, C>::TYPE>(1.0) };
            return Real<P, C>(v, static_cast<double>(v));
        }
        else {
            return Real<P, C>(r.value() < static_cast<typename Real<P, C>::TYPE>(0.0) ? 
                              static_cast<typename Real<P, C>::TYPE>(-1.0) : 
                              static_cast<typename Real<P, C>::TYPE>(1.0));
        }
    }

    /**
    * \brief return the minimal value amongst all input argument
    * 
    * @param {Real/TYPE, in}  a pack of Real or TYPE values
    * @param {Real/TYPE, in}  ....
    * @param {Real,      out} minimal value among all values in the pack
    **/
    template<Precision P, Compare C> constexpr inline Real<P, C> min(const Real<P, C>& r) noexcept {
        return r;
    }
    template<Precision P, Compare C> constexpr inline Real<P, C> min(const typename Real<P, C>::TYPE p) noexcept {
        return Real<P, C>(p);
    }

    template<Precision P, Compare C> constexpr inline Real<P, C> min(const Real<P, C>& l, const Real<P, C>& r) noexcept {
        if constexpr (C == Compare::Yes) {
            return Real<P, C>(std::fmin(l.value(), r.value()), std::fmin(l.exact(), r.exact()));
        }
        else {
            return Real<P, C>(std::fmin(l.value(), r.value()));
        }
    }
    template<Precision P, Compare C> constexpr inline Real<P, C> min(const typename Real<P, C>::TYPE p, const Real<P, C>& r) noexcept {
        if constexpr (C == Compare::Yes) {
            return Real<P, C>(std::fmin(p, r.value()), std::fmin(static_cast<double>(p), r.exact()));
        }
        else {
            return Real<P, C>(std::fmin(p, r.value()));
        }
    }
    template<Precision P, Compare C> constexpr inline Real<P, C> min(const Real<P, C>& l, const typename Real<P, C>::TYPE p) noexcept {
        if constexpr (C == Compare::Yes) {
            return Real<P, C>(std::fmin(l.value(), p), std::fmin(l.exact(), static_cast<double>(p)));
        }
        else {
            return Real<P, C>(std::fmin(l.value(), p));
        }
    }
    template<Precision P, Compare C> constexpr inline typename Real<P, C>::TYPE min(const typename Real<P, C>::TYPE l, const typename Real<P, C>::TYPE p) noexcept {
        return std::fmin(l, p);
    }


    template<Precision P, Compare C, typename... TS> constexpr inline Real<P, C> min(const Real<P, C>& l, const Real<P, C>& r, const TS... args) noexcept {
        return min(min(l, r), args...);
    }
    template<Precision P, Compare C, typename... TS> constexpr inline Real<P, C> min(const typename Real<P, C>::TYPE p, const Real<P, C>& r, const TS... args) noexcept {
        return min(min(p, r), args...);
    }
    template<Precision P, Compare C, typename... TS> constexpr inline Real<P, C> min(const Real<P, C>& l, const typename Real<P, C>::TYPE p, const TS... args) noexcept {
        return min(min(l, p), args...);
    }
    template<Precision P, Compare C, typename... TS> constexpr inline typename Real<P, C>::TYPE min(const typename Real<P, C>::TYPE l, const typename Real<P, C>::TYPE p, const TS... args) noexcept {
        return min(min(l, p), args...);
    }

    /**
    * \brief return the maximal value amongst all input argument
    * 
    * @param {Real/TYPE, in}  a pack of Real or TYPE values
    * @param {Real/TYPE, in}  ....
    * @param {Real,      out} maximal value among all values in the pack
    **/
    template<Precision P, Compare C> constexpr inline Real<P, C> max(const Real<P, C>& r) noexcept {
        return r;
    }
    template<Precision P, Compare C> constexpr inline Real<P, C> max(const typename Real<P, C>::TYPE p) noexcept {
        return Real<P, C>(p);
    }

    template<Precision P, Compare C> constexpr inline Real<P, C> max(const Real<P, C>& l, const Real<P, C>& r) noexcept {
        if constexpr (C == Compare::Yes) {
            return Real<P, C>(std::fmax(l.value(), r.value()), std::fmin(l.exact(), r.exact()));
        }
        else {
            return Real<P, C>(std::fmax(l.value(), r.value()));
        }
    }
    template<Precision P, Compare C> constexpr inline Real<P, C> max(const typename Real<P, C>::TYPE p, const Real<P, C>& r) noexcept {
        if constexpr (C == Compare::Yes) {
            return Real<P, C>(std::fmax(p, r.value()), std::fmax(static_cast<double>(p), r.exact()));
        }
        else {
            return Real<P, C>(std::fmax(p, r.value()));
        }
    }
    template<Precision P, Compare C> constexpr inline Real<P, C> max(const Real<P, C>& l, const typename Real<P, C>::TYPE p) noexcept {
        if constexpr (C == Compare::Yes) {
            return Real<P, C>(std::fmax(l.value(), p), std::fmax(l.exact(), static_cast<double>(p)));
        }
        else {
            return Real<P, C>(std::fmax(l.value(), p));
        }
    }
    template<Precision P, Compare C> constexpr inline typename Real<P, C>::TYPE max(const typename Real<P, C>::TYP& l, const typename Real<P, C>::TYPE p) noexcept {
        return std::fmax(l, p);
    }

    template<Precision P, Compare C, typename... TS> constexpr inline Real<P, C> max(const Real<P, C>& l, const Real<P, C>& r, const TS... args) noexcept {
        return max(max(l, r), args...);
    }
    template<Precision P, Compare C, typename... TS> constexpr inline Real<P, C> max(const typename Real<P, C>::TYPE p, const Real<P, C>& r, const TS... args) noexcept {
        return max(max(p, r), args...);
    }
    template<Precision P, Compare C, typename... TS> constexpr inline Real<P, C> max(const Real<P, C>& l, const typename Real<P, C>::TYPE p, const TS... args) noexcept {
        return max(max(l, p), args...);
    }
    template<Precision P, Compare C, typename... TS> constexpr inline typename Real<P, C>::TYPE max(const typename Real<P, C>::TYPE l, const typename Real<P, C>::TYPE p, const TS... args) noexcept {
        return max(max(l, p), args...);
    }
    
    /**
    * \brief return the input argument clamped into a given region
    * 
    * @param {Real/TYPE, in}  value to be clamped
    * @param {Real/TYPE, in}  clamping region lower value
    * @param {Real/TYPE, in}  clamping region upper value
    * @param {Real,      out} clamped value
    **/
    template<Precision P, Compare C> constexpr inline Real<P, C> clamp(const Real<P, C>& r, const typename Real<P, C>::TYPE lo, const typename Real<P, C>::TYPE hi) noexcept {
        return min(lo, max(r, hi));
    }
    template<Precision P, Compare C> constexpr inline Real<P, C> clamp(const Real<P, C>& r, const Real<P, C>& lo, const typename Real<P, C>::TYPE hi) noexcept {
        return min(lo, max(r, hi));
    }
    template<Precision P, Compare C> constexpr inline Real<P, C> clamp(const Real<P, C>& r, const typename Real<P, C>::TYPE lo, const Real<P, C>& hi) noexcept {
        return min(lo, max(r, hi));
    }
    template<Precision P, Compare C> constexpr inline Real<P, C> clamp(const Real<P, C>& r, const Real<P, C>& lo, const Real<P, C>& hi) noexcept {
        return min(lo, max(r, hi));
    }
};