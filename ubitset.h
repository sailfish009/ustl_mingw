// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#pragma once
#include "ustring.h"
#include "ufunction.h"

namespace ustl {

class istringstream;

typedef uint32_t	bitset_value_type;

void convert_to_bitstring (const bitset_value_type* v, size_t n, string& buf) noexcept;
void convert_from_bitstring (const string& buf, bitset_value_type* v, size_t n) noexcept;

/// \class bitset ubitset.h ustl.h
/// \ingroup Sequences
///
/// \brief bitset is a fixed-size block of memory with addressable bits.
///
/// Normally used for state flags; allows setting and unsetting of individual
/// bits as well as bitwise operations on the entire set. The interface is
/// most like that of unsigned integers, and is intended to be used as such.
/// If you were using begin() and end() functions in STL's bitset, you would
/// not be able to do the same thing here, because those functions return
/// host type iterators, not bits.
///
template <size_t Size>
class bitset {
public:
    typedef bitset_value_type	value_type;
    typedef value_type*		pointer;
    typedef const value_type*	const_pointer;
    typedef pointer		iterator;
    typedef const_pointer	const_iterator;
    typedef size_t		difference_type;
    typedef size_t		size_type;
    typedef const bitset<Size>&	rcself_t;
private:
    static const size_t s_WordBits	= BitsInType (value_type);
    static const size_t	s_nWords	= Size / s_WordBits + ((Size % s_WordBits) != 0);
    static const size_t	s_nBits		= s_nWords * s_WordBits;
private:
    constexpr value_type&	BitRef (uoff_t n)	{ assert (n < Size); return _bits [n / s_WordBits]; }
    constexpr value_type	BitRef (uoff_t n) const	{ assert (n < Size); return _bits [n / s_WordBits]; }
    constexpr value_type	Mask (uoff_t n) const	{ assert (n < Size); return 1 << (n % s_WordBits); }
public:
#if HAVE_CPP11
    constexpr		bitset (value_type v = 0)	:_bits{v}{}
#else
    inline		bitset (value_type v = 0)	{ for (size_t i = 0; i < VectorSize(_bits); ++i) _bits[i] = 0; _bits[0] = v; }
#endif
    inline		bitset (const string& buf)	{ convert_from_bitstring (buf, _bits, s_nWords); }
    constexpr void	flip (uoff_t n)			{ BitRef(n) ^= Mask(n); }
    constexpr void	reset (void)			{ fill_n (_bits, s_nWords, 0); }
    constexpr void	clear (void)			{ fill_n (_bits, s_nWords, 0); }
    constexpr void	set (void)			{ fill_n (_bits, s_nWords, -1); }
    constexpr bitset	operator~ (void) const		{ bitset rv (*this); rv.flip(); return rv; }
    constexpr size_type	size (void) const		{ return Size; }
    constexpr size_type	capacity (void) const		{ return s_nBits; }
    constexpr bool	test (uoff_t n) const		{ return BitRef(n) & Mask(n); }
    constexpr bool	operator[] (uoff_t n) const	{ return test(n); }
    constexpr const_iterator	begin (void) const		{ return _bits; }
    constexpr iterator	begin (void)			{ return _bits; }
    constexpr const_iterator	end (void) const		{ return _bits + s_nWords; }
    constexpr iterator	end (void)			{ return _bits + s_nWords; }
 			/// Returns the value_type with the equivalent bits. If size() > 1, you'll get only the first BitsInType(value_type) bits.
    constexpr value_type to_value (void) const		{ return _bits[0]; }
    			/// Flips all the bits in the set.
    constexpr void	flip (void) { transform (begin(), end(), begin(), bitwise_not<value_type>()); }
			/// Sets or clears bit \p n.
    constexpr void	set (uoff_t n, bool val = true)
			{
			    value_type& br (BitRef (n));
			    const value_type mask (Mask (n));
			    const value_type bOn (br | mask), bOff (br & ~mask);
			    br = val ? bOn : bOff;
			}
			// Sets the value of the bitrange \p first through \p last to the equivalent number of bits from \p v.
    constexpr void	set (uoff_t first, uoff_t DebugArg(last), value_type v)
			{
			    assert (size_t (distance (first, last)) <= s_WordBits && "Bit ranges must be 32 bits or smaller");
			    assert (first / s_WordBits == last / s_WordBits && "Bit ranges can not cross dword (4 byte) boundary");
			    assert ((v & BitMask(value_type,distance(first,last))) == v && "The value is too large to fit in the given bit range");
			    BitRef(first) |= v << (first % s_WordBits);
			}
    			/// Clears the bit \p n.
    constexpr void	reset (uoff_t n)		{ set (n, false); }
			/// Returns a string with bits MSB "001101001..." LSB.
    inline string	to_string (void) const
			{
			    string rv (Size, '0');
			    convert_to_bitstring (_bits, s_nWords, rv);
			    return rv;
			}
    constexpr value_type at (uoff_t n) const		{ return test(n); }
			/// Returns the value in bits \p first through \p last.
    constexpr value_type at (uoff_t first, uoff_t last) const
			{
			    assert (size_t (distance (first, last)) <= s_WordBits && "Bit ranges must be 32 bits or smaller");
			    assert (first / s_WordBits == last / s_WordBits && "Bit ranges can not cross dword (4 byte) boundary");
			    return (BitRef(first) >> (first % s_WordBits)) & BitMask(value_type,distance(first, last));
			}
    constexpr bool	any (void) const	{ value_type sum = 0; foreach (const_iterator, i, *this) sum |= *i; return sum; }
    constexpr bool	none (void) const	{ return !any(); }
    constexpr size_t	count (void) const	{ size_t sum = 0; foreach (const_iterator, i, *this) sum += popcount(*i); return sum; }
    constexpr bool	operator== (rcself_t v) const
			    { return s_nWords == 1 ? (_bits[0] == v._bits[0]) : equal (begin(), end(), v.begin()); }
    constexpr bitset	operator& (rcself_t v) const
			    { bitset<Size> result; transform (begin(), end(), v.begin(), result.begin(), bitwise_and<value_type>()); return result; }
    constexpr bitset	operator| (rcself_t v) const
			    { bitset<Size> result; transform (begin(), end(), v.begin(), result.begin(), bitwise_or<value_type>()); return result; }
    constexpr bitset	operator^ (rcself_t v) const
			    { bitset<Size> result; transform (begin(), end(), v.begin(), result.begin(), bitwise_xor<value_type>()); return result; }
    constexpr rcself_t	operator&= (rcself_t v)
			    { transform (begin(), end(), v.begin(), begin(), bitwise_and<value_type>()); return *this; }
    constexpr rcself_t	operator|= (rcself_t v)
			    { transform (begin(), end(), v.begin(), begin(), bitwise_or<value_type>()); return *this; }
    constexpr rcself_t	operator^= (rcself_t v)
			    { transform (begin(), end(), v.begin(), begin(), bitwise_xor<value_type>()); return *this; }
    inline void		read (istream& is)			{ nr_container_read (is, *this); }
    inline void		write (ostream& os) const		{ nr_container_write (os, *this); }
    inline void		text_write (ostringstream& os) const	{ os << to_string(); }
    void		text_read (istringstream& is);
    constexpr size_t	stream_size (void) const		{ return sizeof(_bits); }
private:
    value_type		_bits [s_nWords];
};

} // namespace ustl
