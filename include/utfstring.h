//          Copyright Grayson Hooper, 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//         http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "utf.h"

// remove for release
#include <iostream>				// Operator<< overload on basic_ostream
#include <string>				// Deprecated 'assign' overload

namespace utf {

	// Move operators outside of class
	// Add more operator overloads
	// Change functions to make use of move semantics

	/*
		* utf::string is an immutable, unicode friendly string class.
		* Converting between different encodings is performed seamlessly and largely implicitly
		* All string operations, with the exception of assign and operator=, do not modify the
		* Internal text string in any way. Rather if a function "modifies" the string, it instead
		* Returns a new string that has had the desired modifications applied to it.
		*/

	template <typename ch>
	class string {				// consider changing name to string
		private:
			// alias for the templated struct used internally in 'utf.h'
			typedef typename impl::encoding_for_size<sizeof(ch)>::type chartype;

			// friend class to prevent errors when accessing members of a string when it has a different encoding (technically a different type)
			friend struct stringview<const ch*>;
			template <typename dchar> friend class string;

			// Returns the unicode encoding as a text string (Can I make these static?)
			template <typename coding>
			const char* priv_encode() { throw("Error: Incorrect coding specified"); return ""; }
			template <>	const char* priv_encode<utf8_t>() { return "UTF-8"; }
			template <>	const char* priv_encode<utf16_t>() { return "UTF-16"; }
			template <>	const char* priv_encode<utf32_t>() { return "UTF-32"; }

			// Determines whether the given indices describe a valid range (ie. |idx1| < |idx2|)
			bool is_valid_range(int idx1,int idx2) {
				return (view.codeidx(idx1) < view.codeidx(idx2));
			}

		protected:
			ch* text;
			stringview<const ch*> view;

			// Internal code chunk to initialize internal text string/stringview
			template <typename dest>
			void rawAssign(stringview<dest>& temp) {
				text = new ch[temp.codeunits<chartype>()];
				temp.to<chartype>(utf_encoder(text));						// fill text
				view.refocus(text, text + temp.codeunits<chartype>());
			}

			// Internal code chunk to handle the inserting of one string into another at a given index
			template <typename dest>
			string<ch> rawSplice(stringview<dest>& piece2,int idx_sp) {					// Perhaps change to string<ch>&&
				auto piece1 = make_stringview(text, text + view.codeidx(idx_sp));
				auto piece3 = make_stringview(text + view.codeidx(idx_sp), text + view.codeidx(0));
				// add the null bit onto the string

				ch* newText = new ch[piece1.codeunits() + piece2.codeunits<chartype>() + piece3.codeunits()];
				piece3.to<chartype>(piece2.to<chartype>(piece1.to<chartype>(utf_encoder(newText))));
	
				return string<ch>(newText, newText + piece1.codeunits() + piece2.codeunits<chartype>() + piece3.codeunits());
			}

		public:
			/*
			This class represents a mutable iterator over a c text string/array for use in conjungtion with stringview::to
			Notes: Does not store an end position. Currently possible to corrupt memory if the wrapped string is shorter than the encoding string
			*/
			class utf_encoder {
				private:
					ch* text;

				public:
					utf_encoder(ch* pos) : text(pos) { }
					~utf_encoder() { text = nullptr; }

					auto operator*() -> decltype(*text) {
						return *text;
					}

					auto operator++() -> decltype(*text) {
						++text;
						return *text;
					}

					auto operator++(int)-> decltype(*text) {
						auto tmp = *text;
						++text;
						return tmp;
					}

			};


			// constructors
			template <typename dchar,size_t N>
			string(const dchar(&_text)[N]) : view(0,0) {					// Text Literal (Not sure about handling custom literals)
				rawAssign(make_stringview(_text));
			}
			template <typename dchar>
			string(const dchar* _text, size_t N) : view(0,0) {				// C-style string
				rawAssign(make_stringview(_text,_text + N));
			}
			template <typename dchar>
			string(string<dchar>& str) : view(0,0) {						// Encoding converter
				rawAssign(str.view);
			}

			// Note: How to initialize with an std::string???

			// move constructors
			string(ch* start, ch* end) : text(start),view(start,end) {		// Char pointers
				start = nullptr;
				end = nullptr;
			}

			string(string<ch>&& str) : text(str.text),view(0,0) {			// string r-value (I don't think this is used to often
				view.refocus(str.view);
				str.text = nullptr;
			}

			// destructor
			~string() { delete[] text; }

			// string info functions

			// The number of characters in a string
			int length() { return view.codepoints(); }			// test to ensure that codepoints will give the right number

			const char* encoding() { return priv_encode<chartype>(); }

			// Gives the array size of the string in the specified encoding (Relies on an explicit template type)
			template <typename dchar = ch>
			size_t strsize() { return view.codeunits<typename impl::encoding_for_size<sizeof(dchar)>::type>(); }	// translates char -> utf8_t, char16_t -> utf16_t, etc.
			template <>	size_t strsize<utf8_t>() { return view.codeunits<utf8_t>(); }
			template <>	size_t strsize<utf16_t>() { return view.codeunits<utf16_t>(); }
			template <>	size_t strsize<utf32_t>() { return view.codeunits<utf32_t>(); }

			// char string conversions

			// Converts a copy of the string into a character array with the desired encoding
			// Note: Currently only "work" on dchar = { char, char16_t, char32_t }
			template <typename dchar>
			dchar* text_as() {
				typedef typename impl::encoding_for_size<sizeof(dchar)>::type dchartype;

				dchar* _text = new dchar[view.codeunits<dchartype>()];
				view.to<dchartype>(string<dchar>::utf_encoder(_text));
				return _text;
			}

			// Overload with a size parameter reference (compatibility with c-style string operations)
			template <typename dchar>
			dchar* text_as(size_t& N) {
				N = strsize<dchar>();
				return text_as<dchar>();
			}

			// deprecated: use string constructor
			template <typename dchar> string<dchar> to() {
				return string<dchar>(*this);
			}

			// mutation functions

			// Changes the internal text to the passed string
			template <typename dchar,size_t N>
			void assign(const dchar(&_text)[N]) {								// Text Literal
				delete[] text;
				rawAssign(make_stringview(_text));
			}

			template <typename dchar>
			void assign(const dchar* _text, size_t N) {							// C-style strings
				delete[] text;
				rawAssign(make_stringview(_text,_text + N));
			}

			template <typename dchar>
			void assign(string<dchar>& str) {									// Encoding converter
				delete[] text;
				rawAssign(str.view);
			}

			// Currently deprecated
			void assign(std::string& str) {
				delete[] text;
				// doesn't append ending character?
				rawAssign(make_stringview(str.begin(), str.end()));
			}

			// text manipulation functions

			/*
				* Creates a substring from the character index 'idx_b' to character index 'idx_e'
				* Note that strings are 1-indexed by default (the first character is indexed with 1, the second with 2, ...
				* This function also provides the ability to index from the back of the string by using a "negative" index
				* The function will throw an error if the actual indices given do not represent a valid substring
				*/
			string<ch> substr(int idx_b = 1,int idx_e = -1) {
				if(!is_valid_range(idx_b, idx_e)) throw("Error: invalid index range");

				auto sub = make_stringview(text + view.codeidx(idx_b), text + view.codeidx(idx_e) + (idx_e > 0));
				auto end = make_stringview(text + view.codeidx(-1), text + view.codeidx(0));

				ch* substr = new ch[sub.codeunits() + end.codeunits()];
				end.to<chartype>(sub.to<chartype>(utf_encoder(substr)));

				return string<ch>(substr, substr + sub.codeunits<chartype>() + end.codeunits<chartype>());
			}

			string<ch> copy() { return substr(); }				// Couldn't I just return *this; ???

	
			// Splices the given text substring into the string at the given position
			// General arguments: splice string, character index after split, splice substring begin, splice substring end

			// Splices (Do I really need all these overloads ???)
			template <typename dchar>
			string<ch> splice(dchar* _text, size_t N, int idx_sp = -1, int idx_b = 1, int idx_e = -1) {
				if(!is_valid_range(idx_b, idx_e)) throw("Error: invalid index range");

				auto temp = make_stringview(_text, _text + N);
				return rawSplice(make_stringview(_text + temp.codeidx(idx_b), _text + temp.codeidx(idx_e) + (idx_e > 0)), idx_sp);
			}

			template<typename dchar,size_t N>
			string<ch> splice(dchar(&_text)[N], int idx_sp = -1, int idx_b = 1, int idx_e = -1) {
				if(!is_valid_range(idx_b, idx_e)) throw("Error: invalid index range");

				auto temp = make_stringview(_text);
				return rawSplice(make_stringview(_text + temp.codeidx(idx_b), _text + temp.codeidx(idx_e) + (idx_e > 0)), idx_sp);
			}

			template <typename dchar>
			string<ch> splice(string<dchar>& str, int idx_sp = -1, int idx_b = 1, int idx_e = -1) {
				if(!is_valid_range(idx_b, idx_e)) throw("Error: invalid index range");

				return rawSplice(make_stringview(str.text + str.view.codeidx(idx_b),
					str.text + str.view.codeidx(idx_e) + (idx_e > 0)), idx_sp);
			}

			// Removes the specified substring
			string<ch> cut(int idx_b = 1,int idx_e = -1) {
				auto piece1 = make_stringview(text,text + view.codeidx(idx_b));
				auto piece2 = make_stringview(text + view.codeidx(idx_e) + (idx_e > 0),text + view.codeidx(0));

				ch* str = new ch[piece1.codeunits() + piece2.codeunits()];
				piece2.to<chartype>(piece1.to<chartype>(utf_encoder(str)));

				return string<ch>(str,str + piece1.codeunits() + piece2.codeunits());
			}

			// boolean operators

			// Performs an arraywise comparison of two strings. Performs any necessary conversions first
			template <typename dchar>
			bool operator==(string<dchar>& str) {
				if(str.strsize<ch>() == strsize()) {
					ch* comp = str.text_as<ch>();

					for(auto i = 0; i != strsize(); ++i)
						if(comp[i] != text[i])
							return false;

					return true;
				}

				return false;
			}

			template <typename dchar>
			bool operator!=(string<dchar>& str) {
				return !operator==(str);
			}

			// bool operator< ???

			// assignment operators

			template <typename dchar>
			string<ch>& operator=(string<dchar>& str) {				// Encoding converter 
				assign(str);
				return *this;
			}

			string<ch>& operator=(string<ch>&& str) {				// string r-value (I don't have any functions that can produce this though)
				text = str.text;
				view = str.view;
				str.text = nullptr;
				return *this;
			}

			template <typename dchar,size_t N>
			string<ch>& operator=(dchar(&str)[N]) {					// string literal
				assign(str);
				return *this;
			}

			// concatentation operators

			// Appends the given string to the current string (wraps splice)
			template <typename dchar>
			string<ch> operator+(string<dchar>& str) {
				return splice(str);
			}

			template <typename dchar, size_t N>
			string<ch> operator+(const dchar(&str)[N]) {				// Couldn't I just provide an overload for string/string&&
				return splice(str,N);
			}

			// other functions
				
			// Allows iteration over the string (Add const and c_begin, etc. ???)
			auto begin() -> decltype(view.begin()) { return view.begin(); }
			auto end() -> decltype(view.end()) { return view.end(); }

			// Cast operator to the internal text string (const qualified to prevent modifications)
			explicit operator const ch*() { return this->text; }
	};

	
}

// usage typedefs (move to utf namespace ???)
typedef utf::string<char> utf8;
typedef utf::string<char16_t> utf16;
typedef utf::string<char32_t> utf32;
typedef utf8 ansi;
typedef utf8 ascii;
typedef utf16 unicode;

namespace std {
	
	// external begin and end for use in ranged for loop
	template <typename ch>
	auto begin(utf::string<ch>& str) -> decltype(str.begin()) { return str.begin(); }
	template <typename ch>
	auto end(utf::string<ch>& str) -> decltype(str.end()) { return str.end(); }

	// Overloads the stream operator (for usage in cout, etc.)
	// Currently relies on explicit
	template<typename ch,typename dchar>
	basic_ostream<dchar>& operator<<(basic_ostream<dchar>& str, utf::string<ch>& text) {
		str << text.text_as<dchar>();
		return str;
	}

	//add overload for insertion and getline
}
