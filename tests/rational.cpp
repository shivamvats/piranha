/***************************************************************************
 *   Copyright (C) 2009-2011 by Francesco Biscani                          *
 *   bluescarni@gmail.com                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "../src/rational.hpp"

#define BOOST_TEST_MODULE rational_test
#include <boost/test/unit_test.hpp>

#define FUSION_MAX_VECTOR_SIZE 20

// #include <algorithm>
#include <boost/fusion/algorithm.hpp>
#include <boost/fusion/include/algorithm.hpp>
#include <boost/fusion/include/sequence.hpp>
#include <boost/fusion/sequence.hpp>
#include <boost/integer_traits.hpp>
#include <boost/lexical_cast.hpp>
// #include <boost/numeric/conversion/bounds.hpp>
// #include <ctgmath>
#include <limits>
#include <memory>
// #include <numeric>
// #include <sstream>
#include <stdexcept>
#include <string>
// #include <type_traits>
// #include <unordered_set>
#include <vector>

#include "../src/integer.hpp"
// #include "../src/exceptions.hpp"

using namespace piranha;

const boost::fusion::vector<char,signed char,short,int,long,long long,unsigned char,unsigned short,unsigned,unsigned long,unsigned long long,float,double> arithmetic_values(
	(char)42,(signed char)42,(short)42,-42,42L,-42LL,
	(unsigned char)42,(unsigned short)42,42U,42UL,42ULL,
	23.456f,-23.456
);

const std::vector<std::string> invalid_strings{"-0","+0","01","+1","123f"," 123","123 ","123.56","123 / 4","212/","/332"};

static inline rational get_big_int()
{
	std::string tmp = boost::lexical_cast<std::string>(boost::integer_traits<unsigned long long>::const_max);
	tmp += "123456789";
	return rational(tmp);
}

struct check_arithmetic_construction
{
	template <typename T>
	void operator()(const T &value) const
	{
		BOOST_CHECK_EQUAL(static_cast<int>(value),static_cast<int>(rational(value)));
	}
};

BOOST_AUTO_TEST_CASE(rational_constructors_test)
{
	// Default construction.
	BOOST_CHECK_EQUAL(0,static_cast<int>(rational()));
	// Construction from arithmetic types.
	boost::fusion::for_each(arithmetic_values,check_arithmetic_construction());
	// Construction from integer.
	BOOST_CHECK_EQUAL(42,static_cast<int>(rational(integer(42))));
	BOOST_CHECK_EQUAL(-42,static_cast<int>(rational(integer(-42))));
	// Construction from string.
	BOOST_CHECK_EQUAL(123,static_cast<int>(rational("123")));
	BOOST_CHECK_EQUAL(-123,static_cast<int>(rational("-123")));
	BOOST_CHECK_EQUAL(128,static_cast<int>(rational("128/1")));
	BOOST_CHECK_EQUAL(-128,static_cast<int>(rational("128/-1")));
	BOOST_CHECK_EQUAL(128,static_cast<int>(rational("-128/-1")));
	BOOST_CHECK_EQUAL(128,static_cast<int>(rational("256/2")));
	BOOST_CHECK_EQUAL(-128,static_cast<int>(rational("256/-2")));
	// Construction from malformed strings.
	std::unique_ptr<rational> ptr;
	for (std::vector<std::string>::const_iterator it = invalid_strings.begin(); it != invalid_strings.end(); ++it) {
		BOOST_CHECK_THROW(ptr.reset(new rational(*it)),std::invalid_argument);
	}
	// Copy construction
	rational i("-30"), j(i);
	BOOST_CHECK_EQUAL(-30,static_cast<int>(j));
	// Large value.
// 	rational i2(get_big_int()), j2(i2);
// 	BOOST_CHECK_EQUAL(i2,j2);
	// Move construction.
	rational i3("-30"), j3(std::move(i3));
	BOOST_CHECK(static_cast<int>(j3) == -30);
// 	rational i4(get_big_int()), j4(std::move(i4));
// 	BOOST_CHECK(j4 == i2);
	// Construction with non-finite floating-point.
	if (std::numeric_limits<float>::has_infinity && std::numeric_limits<double>::has_infinity) {
		BOOST_CHECK_THROW(ptr.reset(new rational(std::numeric_limits<float>::infinity())),std::invalid_argument);
		BOOST_CHECK_THROW(ptr.reset(new rational(std::numeric_limits<double>::infinity())),std::invalid_argument);
	}
	if (std::numeric_limits<float>::has_quiet_NaN) {
		BOOST_CHECK_THROW(ptr.reset(new rational(std::numeric_limits<float>::quiet_NaN())),std::invalid_argument);
	}
	if (std::numeric_limits<double>::has_quiet_NaN) {
		BOOST_CHECK_THROW(ptr.reset(new rational(std::numeric_limits<double>::quiet_NaN())),std::invalid_argument);
	}
	// Construction from numerator and denominator.
	BOOST_CHECK(static_cast<int>(rational(9,3)) == 3);
	BOOST_CHECK(static_cast<int>(rational(-9,3)) == -3);
	BOOST_CHECK(static_cast<int>(rational(9,-3)) == -3);
	BOOST_CHECK(static_cast<int>(rational(-9,-3)) == 3);
	BOOST_CHECK(static_cast<unsigned>(rational(9u,3u)) == 3u);
	BOOST_CHECK(static_cast<unsigned long long>(rational(9ull,3ull)) == 3ull);
	BOOST_CHECK(static_cast<long long>(rational(9ll,-3ll)) == -3ll);
	BOOST_CHECK(static_cast<int>(rational(integer(-9),integer(3))) == -3);
}

struct check_arithmetic_assignment
{
	check_arithmetic_assignment(rational &i):m_i(i) {}
	template <typename T>
	void operator()(const T &value) const
	{
		m_i = value;
		BOOST_CHECK_EQUAL(static_cast<int>(value),static_cast<int>(m_i));
	}
	rational &m_i;
};

BOOST_AUTO_TEST_CASE(rational_assignment_test)
{
	rational i, j;
	// Assignment from arithmetic types.
	boost::fusion::for_each(arithmetic_values,check_arithmetic_assignment(i));
	// Assignment from string.
	i = "123";
	BOOST_CHECK_EQUAL(123,static_cast<int>(i));
	i = std::string("-123");
	BOOST_CHECK_EQUAL(-123,static_cast<int>(i));
	// Assignment from malformed strings.
	for (std::vector<std::string>::const_iterator it = invalid_strings.begin(); it != invalid_strings.end(); ++it) {
		BOOST_CHECK_THROW(i = *it,std::invalid_argument);
	}
	// Copy assignment.
	i = "30000/2";
	j = i;
	BOOST_CHECK_EQUAL(15000,static_cast<int>(j));
	// Assignment from non-finite floating-point.
	if (std::numeric_limits<float>::has_infinity && std::numeric_limits<double>::has_infinity) {
		BOOST_CHECK_THROW(j = -std::numeric_limits<float>::infinity(),std::invalid_argument);
		BOOST_CHECK_THROW(j = std::numeric_limits<double>::infinity(),std::invalid_argument);
	}
	if (std::numeric_limits<float>::has_quiet_NaN) {
		BOOST_CHECK_THROW(j = std::numeric_limits<float>::quiet_NaN(),std::invalid_argument);
	}
	if (std::numeric_limits<double>::has_quiet_NaN) {
		BOOST_CHECK_THROW(j = std::numeric_limits<double>::quiet_NaN(),std::invalid_argument);
	}
	// Assignment from integer.
	i = integer(100);
	BOOST_CHECK_EQUAL(100,static_cast<int>(i));
}

struct check_arithmetic_move_construction
{
	template <typename T>
	void operator()(const T &value) const
	{
		rational i(value), j(std::move(i));
		BOOST_CHECK_EQUAL(static_cast<int>(value),static_cast<int>(j));
		// Revive i.
		i = value;
		BOOST_CHECK_EQUAL(static_cast<int>(value),static_cast<int>(i));
	}
};

struct check_arithmetic_move_assignment
{
	template <typename T>
	void operator()(const T &value) const
	{
		rational i(value), j;
		j = std::move(i);
		BOOST_CHECK_EQUAL(static_cast<int>(value),static_cast<int>(j));
		// Revive i.
		i = value;
		BOOST_CHECK_EQUAL(static_cast<int>(value),static_cast<int>(i));
	}
};

BOOST_AUTO_TEST_CASE(rational_move_semantics_test)
{
	boost::fusion::for_each(arithmetic_values,check_arithmetic_move_construction());
	boost::fusion::for_each(arithmetic_values,check_arithmetic_move_assignment());
	// Revive via plain assignment.
	{
		rational i(42), j, k(43);
		j = std::move(i);
		j = k;
		BOOST_CHECK_EQUAL(43,static_cast<int>(j));
	}
	// Revive via move assignment.
	{
		rational i(42), j, k(43);
		j = std::move(i);
		j = std::move(k);
		BOOST_CHECK_EQUAL(43,static_cast<int>(j));
	}
	// Revive via string assignment.
	{
		rational i(42), j;
		j = std::move(i);
		j = "42";
		BOOST_CHECK_EQUAL(42,static_cast<int>(j));
	}
}

BOOST_AUTO_TEST_CASE(rational_swap_test)
{
	rational i(42), j(43), k(10,3);
	i.swap(j);
	BOOST_CHECK_EQUAL(43,static_cast<int>(i));
	i.swap(k);
	BOOST_CHECK_EQUAL(3,static_cast<int>(i));
	k = get_big_int();
	i.swap(k);
	BOOST_CHECK_EQUAL(3,static_cast<int>(k));
	k.swap(i);
	BOOST_CHECK_EQUAL(3,static_cast<int>(i));
// 	piranha::integer l(get_big_int() + 1);
// 	l.swap(k);
// 	BOOST_CHECK_EQUAL(get_big_int(),l);
}
