// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2016 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#pragma once
#if HAVE_CPP11
#include "uratio.h"
#include <time.h>

namespace ustl {
namespace chrono {

//{{{ duration_cast ----------------------------------------------------

template <typename Rep, typename Period> struct duration;

template <typename ToDuration, typename Rep, typename Period>
struct __duration_cast_impl {
    using to_rep	= typename ToDuration::rep;
    using conv_ratio	= typename ratio_divide<Period, typename ToDuration::period>::type;
    static inline ToDuration cast (const duration<Rep, Period>& d)
	{ return ToDuration (static_cast<to_rep>(d.count()) * conv_ratio::num / conv_ratio::den); }
};

/// Converts durations of different periods or representations
template <typename ToDuration, typename Rep, typename Period>
ToDuration duration_cast (const duration<Rep, Period>& d)
    { return __duration_cast_impl<ToDuration,Rep,Period>::cast (d); }

//}}}-------------------------------------------------------------------
//{{{ treat_as_floating_point

template <typename Rep>
struct treat_as_floating_point : public is_floating_point<Rep> {};

//}}}-------------------------------------------------------------------
//{{{ duration

/// Stores a time duration
template <typename Rep, typename Period = ratio<1>>
struct duration {
    using rep		= Rep;
    using period	= Period;

    // Automatic conversion ctors enabled if lossless
    template <typename Rep2, typename = typename
		enable_if<is_convertible<Rep2, rep>::value
			&& (treat_as_floating_point<rep>::value
			    || numeric_limits<Rep2>::is_integer)>::type>
    inline constexpr explicit duration (const Rep2& v)
	  : _r (v) {}
    template <typename Rep2, typename Period2, typename = typename
		enable_if<treat_as_floating_point<rep>::value
			|| (ratio_divide<Period2, period>::den == 1
			    && numeric_limits<Rep2>::is_integer)>::type>
    inline constexpr duration (const duration<Rep2, Period2>& v)
	: _r (duration_cast<duration>(v).count()) {}

    constexpr			duration (void) = default;
    constexpr			duration (const duration& d) = default;
				~duration (void) = default;
    duration&			operator= (const duration&) = default;
    inline constexpr const rep&	count (void) const		{ return _r; }
    inline constexpr duration	operator+ (void) const		{ return *this; }
    inline constexpr duration	operator- (void) const		{ return duration(-count()); }
    inline duration&		operator++ (void)		{ ++_r; return *this; }
    inline duration		operator++ (int)		{ return duration(_r++); }
    inline duration&		operator-- (void)		{ --_r; return *this; }
    inline duration		operator-- (int)		{ return duration(_r--); }
    inline duration&		operator+= (const duration& v)	{ _r += v.count(); return *this; }
    inline duration&		operator-= (const duration& v)	{ _r -= v.count(); return *this; }
    inline duration&		operator*= (const rep& v)	{ _r *= v; return *this; }
    inline duration&		operator/= (const rep& v)	{ _r /= v; return *this; }
    inline duration		operator+ (const duration& v) const	{ return duration (_r + v.count()); }
    inline duration		operator- (const duration& v) const	{ return duration (_r - v.count()); }

    // Mod allowed only for integers
    template <typename Rep2 = rep> typename enable_if<numeric_limits<Rep2>::is_integer,
    duration&>::type		operator%= (const rep& v)	{ _r %= v; return *this; }
    template <typename Rep2 = rep> typename enable_if<numeric_limits<Rep2>::is_integer,
    duration&>::type		operator%= (const duration& v)	{ _r %= v.count(); return *this; }

    inline constexpr bool	operator== (const duration& v) const	{ return count() == v.count(); }
    inline constexpr bool	operator< (const duration& v) const	{ return count() < v.count(); }

    // Comparisons with different duration types are allowed when implicitly convertible
    template <typename Rep2, typename Period2>
    inline constexpr typename enable_if<
		ratio_greater<ratio_divide<period,Period2>,ratio<1>>::value
		    && is_convertible<duration,duration<Rep2,Period2>>::value,
    bool>::type			operator== (const duration<rep,Period2>& v) const
				    { return duration<Rep2,Period2>(*this) == v; }
    template <typename Rep2, typename Period2>
    inline constexpr typename enable_if<
		ratio_less_equal<ratio_divide<period,Period2>,ratio<1>>::value
		    && is_convertible<duration,duration<Rep2,Period2>>::value,
    bool>::type			operator== (const duration<rep,Period2>& v) const
				    { return operator== (duration(v)); }
    template <typename Rep2, typename Period2>
    inline constexpr typename enable_if<
		ratio_greater<ratio_divide<period,Period2>,ratio<1>>::value
		    && is_convertible<duration,duration<Rep2,Period2>>::value,
    bool>::type			operator< (const duration<rep,Period2>& v) const
				    { return duration<Rep2,Period2>(*this) < v; }
    template <typename Rep2, typename Period2>
    inline constexpr typename enable_if<
		ratio_less_equal<ratio_divide<period,Period2>,ratio<1>>::value
		    && is_convertible<duration,duration<Rep2,Period2>>::value,
    bool>::type			operator< (const duration<rep,Period2>& v) const
				    { return operator< (duration(v)); }

    // Named ctors for special values
    static constexpr duration	zero (void)	{ return duration (0); }
    static constexpr duration	min (void)	{ return duration (numeric_limits<rep>::lowest()); }
    static constexpr duration	max (void)	{ return duration (numeric_limits<rep>::max()); }
private:
    rep	_r;
};

//}}}-------------------------------------------------------------------
//{{{ Standard durations

using nanoseconds	= duration<intmax_t, nano>;
using microseconds	= duration<intmax_t, micro>;
using milliseconds	= duration<intmax_t, milli>;
using seconds		= duration<intmax_t>;
using minutes		= duration<intmax_t, ratio<60>>;
using hours		= duration<intmax_t, ratio<minutes::period::num*60>>;
using days		= duration<intmax_t, ratio<hours::period::num*24>>;
using weeks		= duration<intmax_t, ratio<days::period::num*7>>;
using years		= duration<intmax_t, ratio<365*days::period::num+days::period::num/4>>;
using months		= duration<intmax_t, ratio<years::period::num/12>>;
using centuries		= duration<intmax_t, ratio<years::period::num*100>>;
using millenia		= duration<intmax_t, ratio<years::period::num*1000>>;

//}}}-------------------------------------------------------------------
//{{{ hrtime_t

// Extends struct timespec, used by libc time functions, as an arithmetic type
struct hrtime_t : public timespec {
    inline constexpr		hrtime_t (void)			: timespec{}{}
    inline constexpr		hrtime_t (const hrtime_t&) = default;
    inline constexpr hrtime_t&	operator= (const hrtime_t&) = default;
    inline explicit constexpr	hrtime_t (const timespec& v)	: timespec{v}{}
    inline constexpr		hrtime_t (time_t s, long ns)	: timespec{s,ns}{}
    inline explicit constexpr	hrtime_t (intmax_t v)		: timespec{v/nano::den,v%nano::den}{}
    inline explicit constexpr	hrtime_t (float v)		: timespec{time_t(v),time_t((v-time_t(v))*nano::den)}{}
    inline explicit constexpr	hrtime_t (double v)		: timespec{time_t(v),time_t((v-time_t(v))*nano::den)}{}
    inline constexpr		operator intmax_t (void) const	{ return intmax_t(tv_sec)*nano::den + tv_nsec; }
    inline constexpr		operator float (void) const	{ return float(tv_nsec)/nano::den + tv_sec; }
    inline constexpr		operator double (void) const	{ return double(tv_nsec)/nano::den + tv_sec; }
    inline constexpr hrtime_t	operator+ (void) const		{ return *this; }
    inline constexpr hrtime_t	operator- (void) const		{ return hrtime_t(-tv_sec,-tv_nsec); }
    hrtime_t&			operator++ (void) {
				    if (++tv_nsec > nano::den) {
					tv_nsec -= nano::den;
					++tv_sec;
				    }
				    return *this;
				}
    hrtime_t&			operator-- (void) {
				    if (!tv_nsec) {
					tv_nsec = nano::den;
					--tv_sec;
				    }
				    --tv_nsec;
				    return *this;
				}
    inline hrtime_t		operator++ (int)		{ hrtime_t r(*this); operator++(); return r; }
    inline hrtime_t		operator-- (int)		{ hrtime_t r(*this); operator--(); return r; }
    hrtime_t&			operator+= (const hrtime_t& v) {
				    if (nano::den <= (tv_nsec += v.tv_nsec)) {
					tv_nsec -= nano::den;
					++tv_sec;
				    }
				    tv_sec += v.tv_sec;
				    return *this;
				}
    hrtime_t&			operator-= (const hrtime_t& v) {
				    if (0 > (tv_nsec -= v.tv_nsec)) {
					tv_nsec += nano::den;
					--tv_sec;
				    }
				    tv_sec -= v.tv_sec;
				    return *this;
				}
    hrtime_t&			operator*= (intmax_t v) {
				    tv_sec *= v;
				    if (nano::den <= (tv_nsec *= v)) {
					tv_sec += tv_nsec / nano::den;
					tv_nsec %= nano::den;
				    }
				    return *this;
				}
    hrtime_t&			operator/= (intmax_t v)	{
				    tv_nsec /= v;
				    tv_nsec += (tv_sec % v) * nano::den;
				    tv_sec /= v;
				    return *this;
				}
    inline hrtime_t		operator+ (const hrtime_t& v) const	{ hrtime_t r(*this); return r += v; }
    inline hrtime_t		operator- (const hrtime_t& v) const	{ hrtime_t r(*this); return r -= v; }
    inline hrtime_t		operator* (intmax_t v) const		{ hrtime_t r(*this); return r *= v; }
    inline hrtime_t		operator/ (intmax_t v) const		{ hrtime_t r(*this); return r /= v; }
    inline constexpr bool	operator== (const hrtime_t& v) const	{ return tv_sec == v.tv_sec && tv_nsec == v.tv_nsec; }
    inline constexpr bool	operator< (const hrtime_t& v) const	{ return tv_sec < v.tv_sec || (tv_sec == v.tv_sec && tv_nsec < v.tv_nsec); }
};

// Override duration_cast to avoid intermediate conversion to intmax_t nanoseconds
template <>
template <typename ToDuration, typename Period>
struct __duration_cast_impl<ToDuration, hrtime_t, Period> {
    using to_rep	= typename ToDuration::rep;
    using ns_conv_ratio	= typename ratio_divide<Period, typename ToDuration::period>::type;
    using s_conv_ratio	= typename ratio_divide<ns_conv_ratio, Period>::type;
    static inline ToDuration cast (const duration<hrtime_t, Period>& d) {
	auto& t = d.count();
	return ToDuration (
		static_cast<to_rep>(t.tv_sec) * s_conv_ratio::num / s_conv_ratio::den
		+ static_cast<to_rep>(t.tv_nsec) * ns_conv_ratio::num / ns_conv_ratio::den);
    }
};

// timespec has a very high resolution, so is convertible as float
template <>
struct treat_as_floating_point<hrtime_t> : public true_type {};

//}}}-------------------------------------------------------------------
//{{{ time_point

template <typename Clock, typename Duration> struct time_point;

template <typename ToDuration, typename Clock, typename FromDuration>
constexpr time_point<Clock, ToDuration> time_point_cast (const time_point<Clock, FromDuration>& tp)
    { return time_point<Clock,ToDuration> (duration_cast<ToDuration> (tp.time_since_epoch())); }

template <typename Clock, typename Duration = typename Clock::duration>
struct time_point {
    using clock		= Clock;
    using duration	= Duration;
    using rep		= typename duration::rep;
    using period	= typename duration::period;

    constexpr			time_point (void) = default;
    constexpr explicit		time_point (const duration& t)	: _t(t) {}
    template <typename Duration2, typename = typename enable_if<is_convertible<Duration2,duration>::value>::type>
    inline constexpr		time_point (const time_point<clock, Duration2>& t)
				    :_t (t.time_since_epoch()) {}
    constexpr const duration&	time_since_epoch (void) const	{ return _t; }
    inline time_point&		operator+= (const duration& t)	{ _t += t; return *this; }
    inline time_point&		operator-= (const duration& t)	{ _t -= t; return *this; }
    inline time_point		operator- (const duration& t) const { return time_point(_t - t); }
    inline time_point		operator+ (const duration& t) const { return time_point(_t + t); }
    template <typename Duration2 = duration>
    inline duration		operator- (const time_point<clock,Duration2>& t) const
				    { return _t - t.time_since_epoch(); }
    template <typename Duration2 = duration>
    inline constexpr bool	operator== (const time_point<clock,Duration2>& t) const
				    { return time_since_epoch() == t.time_since_epoch(); }
    template <typename Duration2 = duration>
    inline constexpr bool	operator< (const time_point<clock,Duration2>& t) const
				    { return time_since_epoch() < t.time_since_epoch(); }
    static constexpr time_point	min (void)	{ return time_point (duration::min()); }
    static constexpr time_point	max (void)	{ return time_point (duration::max()); }
private:
    duration	_t;
};

//}}}-------------------------------------------------------------------
//{{{ system_clock

struct system_clock {
    using duration	= seconds;
    using rep		= typename duration::rep;
    using period	= typename duration::period;
    using time_point	= ::ustl::chrono::time_point<system_clock, duration>;

    static inline time_t	to_time_t (const time_point& tp) noexcept
				    { return static_cast<time_t>(tp.time_since_epoch().count()); }
    static inline time_point	from_time_t (time_t t) noexcept
				    { return time_point (duration (t)); }
    static inline time_point	now (void) noexcept
				    { return from_time_t (time (nullptr)); }
    static constexpr bool	is_steady = false;
};

//}}}-------------------------------------------------------------------
//{{{ high_resolution_clock

struct high_resolution_clock {
    using duration	= ::ustl::chrono::duration<hrtime_t, nano>;
    using rep		= typename duration::rep;
    using period	= typename duration::period;
    using time_point	= ::ustl::chrono::time_point<high_resolution_clock, duration>;
protected:
    static inline rep		rep_now (clockid_t clkid) noexcept
				    { rep ts; clock_gettime (clkid, &ts); return ts; }
public:
    static time_point		now (void) noexcept
				    { return time_point (duration (rep_now (CLOCK_REALTIME))); }
    static constexpr bool	is_steady = false;
};

//}}}-------------------------------------------------------------------
//{{{ steady_clock

struct steady_clock : public high_resolution_clock {
    using time_point		= ::ustl::chrono::time_point<steady_clock, duration>;
    static time_point		now (void) noexcept
				    { return time_point (duration (rep_now (CLOCK_MONOTONIC))); }
    static constexpr bool	is_steady = true;
};

//}}}-------------------------------------------------------------------
//{{{ system_clock_hr

// High resolution version of system_clock, storing a single intmax_t instead of timespec
template <typename Duration = milliseconds>
struct system_clock_hr : public high_resolution_clock {
    using duration	= Duration;
    using rep		= typename duration::rep;
    using period	= typename duration::period;
    using time_point	= ::ustl::chrono::time_point<system_clock_hr, duration>;
public:
    static time_point now (void) noexcept {
	return time_point (
		duration_cast<duration>(
		    high_resolution_clock::duration(
			rep_now (CLOCK_REALTIME))));
    }
};

// Specific resolution aliases for getting time_points at given resolution.
// This simplifies the very common use of timing blocks of code.
using system_clock_ms	= system_clock_hr<milliseconds>;
using system_clock_us	= system_clock_hr<microseconds>;
using system_clock_ns	= system_clock_hr<nanoseconds>;

//}}}-------------------------------------------------------------------

} // namespace chrono
} // namespace ustl
#endif // HAVE_CPP11
