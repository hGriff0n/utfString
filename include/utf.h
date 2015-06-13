//          Copyright Jesper Dam 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//         http://www.boost.org/LICENSE_1_0.txt)

//	Copied and Edited: 2013, Grayson Andrew Hooper
//	Source from original library "Jalfd/utf.hpp": 
//	  found at https://github.com/jalfd/utf.hpp
//	Name changes and feature additions (~33 new lines)
//	Used and edited for use in custom utfString library

// improve behavior of codeidx
// add documentation

#pragma once

#include "utf_impl.h"
#include <iterator>

#include <iostream>

namespace utf {

	template <typename It>
	class utf_iterator
		: public std::iterator<std::input_iterator_tag,const codepoint_type,ptrdiff_t,const codepoint_type*,const codepoint_type&> {
			typedef typename std::iterator_traits<It>::value_type codeunit_type;
			typedef typename impl::native_encoding<codeunit_type>::type encoding;
			typedef impl::code_traits<encoding> traits_type;
			codepoint_type val;
			It pos;

	public:
		explicit utf_iterator() : val(),pos() { }
		explicit utf_iterator(It pos) : val(),pos(pos) { }
		utf_iterator(const utf_iterator& it) : val(it.val),pos(it.pos) { }

		typename std::iterator_traits<utf_iterator>::reference operator*() {
			val = traits_type::decode(pos);
			return val;
		}
		typename std::iterator_traits<utf_iterator>::pointer operator->() const { return (&**this); }
		utf_iterator& operator++() {
			It next = pos + traits_type::read_length(*pos);
			pos = next;
			return *this;
		}
		utf_iterator operator++(int) {
			codepoint_type tmp = *this;
			++(*this);
			return tmp;
		}
		friend bool operator != (utf_iterator lhs,utf_iterator rhs) { return lhs.pos != rhs.pos; }
		friend bool operator == (utf_iterator lhs,utf_iterator rhs) { return !(lhs != rhs); }
	};

	// template <typename E, typename Iter = const typename impl::code_traits<E>::codeunit_type*>
	template <typename Iter,typename E = typename impl::native_encoding<typename std::iterator_traits<Iter>::value_type>::type>
	struct stringview {
		typedef typename std::iterator_traits<Iter>::value_type codeunit_type;

		stringview(const Iter first,const Iter last) : first(first),last(last) { }
		/*
		stringview(const stringview<Iter>& view) {
			refocus(view.first,view.last);
		} // can't copy std:string Iterators*/

		// returns iterators for on-the-fly decoding of a string
		// from its current encoding to Unicode code points
		utf_iterator<Iter> begin() const { return utf_iterator<Iter>(first); }
		utf_iterator<Iter> end() const { return utf_iterator<Iter>(last); }
		utf_iterator<Iter> at(int pos) const { return utf_iterator<Iter>(first + pos); }

		// moves the stringview to focus on a different string
		void refocus(const stringview<Iter>& view) {
			refocus(view.first,view.last);
		}
		void refocus(Iter start,Iter end) {
			const_cast<Iter>(this->first) = start;
			const_cast<Iter>(this->last) = end;
		}

		// check string's validity under it's current encoding
		bool validate() const {
			typedef impl::code_traits<E> traits_t;
			for(Iter it = first;  it < last;) {
				size_t len = traits_t::read_length(*it);
				if(last - it < static_cast<ptrdiff_t>(len)) {
					return false;
				}
				if(!traits_t::validate(it,it + len)) {
					return false;
				}
				codepoint_type cp = traits_t::decode(it);
				if(!impl::validate_codepoint(cp)) {
					return false;
				}
				it += len;
			}
			return true;
		}

		// number of code points in current encoding
		size_t codepoints() const {
			return std::distance(begin(),end());
		}

		// byte size of string in current encoding
		size_t bytes() const {
			return codeunits() * sizeof(typename impl::code_traits<E>::codeunit_type);
		}

		// byte size of string in future encoding EDest
		template <typename EDest>
		size_t bytes() const {
			return codeunits<EDest>() * sizeof(typename impl::code_traits<EDest>::codeunit_type);
		}

		// string length in current encoding
		size_t codeunits() const { return last - first; }

		// string length in future encoding
		template <typename EDest>
		size_t codeunits() const {
			size_t cus = 0;
			for(auto it = this->begin(); it != this->end(); ++it)
				cus += impl::code_traits<EDest>::write_length(*it);
			return cus;
		}

		// Encode the string in EDest
		template <typename EDest,typename OutIt>
		OutIt to(OutIt dest) const {
			//if(this->codeunits<EDest>() != 0)
			for(auto it : *this)
				dest = impl::code_traits<EDest>::encode(it,dest);
			return dest;
		}

		// Returns the array position of the idx character of the string
		// Returns location of the ending null bit if idx is 0
		// clean up code
		int codeidx(int idx) {
			if(idx == 0 || idx > (int)codeunits() || idx < 0 - (int)codeunits()) return this->codeunits();
			auto marker = 0;
			auto pointer = this->begin();
			idx = (idx < 0) ? this->codeunits() + idx : idx - 1;
			while(idx--) {
				auto plus = impl::code_traits<E>::read_length(*pointer);
				marker += plus;
				for(; plus; --plus)
					++pointer;
			}
			return marker;
		}

	private:
		const Iter first;
		const Iter last;
	};

	// convenience stuff
	template <typename T,size_t N>
	stringview<const T*> make_stringview(T(&arr)[N]) {
		return stringview<const T*>(arr,arr + N);
	}

	template <typename Iter>
	stringview<Iter> make_stringview(Iter first,Iter last) {
		return stringview<Iter>(first,last);
	}

}

namespace std {

	// 
	template <typename Iter, typename E>
	auto begin(utf::stringview<Iter,E>& str) -> decltype(str.begin()) {
		return str.begin();
	}

	template <typename Iter, typename E>
	auto end(utf::stringview<Iter,E>& str) -> decltype(str.begin()) {
		return str.end();
	}

}
