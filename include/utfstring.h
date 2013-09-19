//          Copyright Grayson Hooper, 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//         http://www.boost.org/LICENSE_1_0.txt)

/*
file export overview

utfstring -
	utf_encoder: class

	constructors: copy,move,literal,text,range
	strlength: int
	encoding: char*
	strsize<encoding>: size_t
	text_as<dchar>: size_t&		->	dchar*
	text_as<dhcar>: dchar*
	assign<dchar,size_t>: &dchar[],void
	assign<dchar>: dchar*,size_t -> void
	assign<dchar>: utfstring<dchar>& -> void
	substr: int,int -> utfstring<ch>
	copy: utfstring<ch>
	splice<dchar,size_t>: &dchar[],int,int,int -> utfstring<ch>
	splice<dchar>: dchar*,size_t,int,int,int -> utfstring<ch>
	splice<dchar>: utfstring<dchar>&,int,int,int -> utfstring<ch>
	cut: int,int -> utfstring<ch>
	operator==<dchar>: utfstring<dchar>& -> bool
	operator!=<dchar>: utfstring<dchar>& -> bool
	operator=<dchar,size_t>: &dchar[] -> utfstring<ch>&
	operator=: utfstring<ch>&& -> utfstring<ch>&
	operator=<dchar>: utfstring<dchar>& -> utfstring<ch>&
	operator+<dchar>: utfstring<dchar>& -> utfstring<ch>
	operator+<dchar,size_t>: &dchar[] -> utfstring<ch>
	begin: auto	"decltype(view.begin())"
	end: auto "decltype(view.end())"
	(const ch*): const ch*

std:
	begin<ch>: utfstring<ch>& -> auto "decltype(utfstring.begin())"
	end<ch>: utfstring<ch>& -> auto "decltype(utfstring.end())"
	operator<<[<stream,ch>]: stream&,utfstring<ch>& -> stream&
*/

#pragma once

#include "utf.h"

#include <iostream>

namespace utf {
	
	// Move operators outside of class
	// Add more operator overloads
	// Change functions to make use of move semantics

	/*--class--
	utfstring:
	pass: copy : literal string : char string,size of string : move : char range{begin,end}
	template: type of the internal text string
	action: unicode friendly string class
	action: freely converts between unicode encodings
	notes: string is immutable outside of calls to operator= and assign.
			all text manipulation is performed on a copy of the internal text string
	notes: member functions that operate on the string expect character indexes and not array indexes
	*/
	template <typename ch>
	class utfstring {
			/*--typedef--
			chartype:		typename impl::encoding_for_size<sizeof(ch)>::type chartype
			action: alias for template parameters to generate the struct used internally in 'utf.h'
			*/
			typedef typename impl::encoding_for_size<sizeof(ch)>::type chartype;

			/*--friend--
			stringview: 
			utfstring<dchar>: prevent errors when accessing members of a utfstring<dchar> (technically a different type than utfstring<ch>)
			*/
			friend struct stringview<const ch*>;
			template <typename dchar> friend class utfstring;
		private:
			/*--function--
			priv_encode:	return - const char*
			template: template { encoding }
			action: returns a text string with the strings encoding
			throws: template type is not overloaded below
			*/
			template <typename coding>
			const char* priv_encode() { throw("Error: Incorrect coding specified"); return ""; }
			template <>	const char* priv_encode<utf8_t>() { return "UTF-8"; }
			template <>	const char* priv_encode<utf16_t>() { return "UTF-16"; }
			template <>	const char* priv_encode<utf32_t>() { return "UTF-32"; }

			/*--function--
			!is_valid_range:	return - bool
			action: determine if the passed indexs are valid as a range
					 ie. if idx1 points to a character after idx2 it is invalid
			*/
			bool is_valid_range(int idx1,int idx2) {
				return (view.codeidx(idx1) < view.codeidx(idx2));
			}

		protected:
			ch* text;
			stringview<const ch*> view;

			/*--function--
			rawAssign:		return - void
			pass: stringview to the assign string
			template: type of stringview(inferred)
			action: common code to initialize internal text string with the text string observed by temp
			notes: also reloacates the internal stringview to point to the new text
			*/
			template <typename dest>
			void rawAssign(stringview<dest>& temp) {
				text = new ch[temp.codeunits<chartype>()];
				temp.to<chartype>(utf_encoder(text));
				view.refocus(text,text + temp.codeunits<chartype>());
			}
			/*--function--
			rawSplice:		return - string<ch>
			pass: stringview to the central piece,index of
			template: type of stringview(inferred)
			action: common code to create a new string by inserting the text observed by piece2
						into a copy of text at position idx_sp
			notes: guaranteed to add delimiting characters onto the end of the new text string
			plan: change return type to utfstring<ch>&&
			*/
			template <typename dest>
			utfstring<ch> rawSplice(utfstringview<dest>& piece2,int idx_sp) {
				auto piece1 = make_stringview(text,text + view.codeidx(idx_sp));
				auto piece3 = make_stringview(text + view.codeidx(idx_sp),text + view.codeidx(0));
				// add the null bit onto the string

				ch* newText = new ch[piece1.codeunits() + piece2.codeunits<chartype>() + piece3.codeunits()];
				piece3.to<chartype>(piece2.to<chartype>(piece1.to<chartype>(utf_encoder(newText))));

				return utfstring<ch>(newText,newText + piece1.codeunits() + piece2.codeunits<chartype>() + piece3.codeunits());

			}

		public:

			/*--class--
			utf_encoder:
			pass: pointer to the beginning of the text string
			action: mutable iterator over a text string for use in stringview::to
					enables encoding conversion directly to a text string
			notes: does not store an end position. possible to corrupt memory
					if the wrapped text string is shorter than the values its initialized with
			notes: derives internal type from string's type
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
			utfstring(const dchar(&_text)[N]) : view(0,0) {
				rawAssign(make_stringview(_text));
			}
			template <typename dchar>
			utfstring(const dchar* _text,size_t N) : view(0,0) {
				rawAssign(make_stringview(_text,_text+N));
			}
			template <typename dchar>
			utfstring(utfstring<dchar>& str) : view(0,0) {
				rawAssign(str.view);
			}

			// move constructors
			utfstring(ch* start,ch* end) : text(start),view(start,end) { 
				start = nullptr;
				end = nullptr;
			}
			utfstring(utfstring<ch>&& str) : text(str.text),view(0,0) { 
				view.refocus(str.view);
				str.text = nullptr;
			}

			// destructor
			~utfstring() { delete[] text; }

			// string info functions

			/*--function--
			strlength:		return - int
			pass: void
			action: returns the number of characters in the string
			notes: untested (not sure if codepoints will give the desired results)
			*/
			int strlength() { return view.codepoints(); }
			/*--function--
			encoding:		return - const char*
			pass: void
			action: returns a text string describing the encoding of the string
			notes: calls priv_encode<chartype> to prevent return of incorrect encodings (ie. "UTF-16" on a utf8 string)
					encoding does not have a template argument that could be overrode by a user
			*/
			const char* encoding() { return priv_encode<chartype>(); }
			/*--function--
			strsize:		return - size_t
			template: encoding
			action: returns the array size of the internal string in the specified encoding 
			notes: compile-time error if not called with template types { char,char16_t,char32_t,utf8_t,utf16_t,utf32_t }
					error thrown by utf::impl functions/classes
			notes: specializations are to prevent mappings from being evaluated exactly like keys
					char maps to utf8_t , char16_t to utf16_t , etc.	{ key -> mapping }
			*/
			template <typename dchar =ch>
			size_t strsize() { return view.codeunits<typename impl::encoding_for_size<sizeof(dchar)>::type>(); }
			template <>	size_t strsize<utf8_t>() { return view.codeunits<utf8_t>(); }
			template <>	size_t strsize<utf16_t>() { return view.codeunits<utf16_t>(); }
			template <>	size_t strsize<utf32_t>() { return view.codeunits<utf32_t>(); }

			// char string conversions

			/*--function--
			text_as:		return - dchar*
			pass: { size_t& }
			template: type of string to return
			action: returns a copy of the internal text string in the specified encoding
			notes: relies on an explicit template type. helps with code readability
			notes: does not have a similar specialization pattern to priv_encode
					compile-time error if not called with template type { char, char16_t, char32_t }
			notes: overload provided with size reference parameter
			*/
			template <typename dchar>
			dchar* text_as() {
				typedef typename impl::encoding_for_size<sizeof(dchar)>::type dchartype;

				dchar* _text = new dchar[view.codeunits<dchartype>()];
				view.to<dchartype>(utfstring<dchar>::utf_encoder(_text));
				return _text;
			}
			template <typename dchar>
			dchar* text_as(size_t& N) {
				N = strsize<dchar>();
				return text_as<dchar>();
			}
			template <typename dchar>
			utfstring<dchar> to() {
				size_t N=0;
				dchar* _text = text_as<dchar>(N);
				return utfstring<dchar>(_text,_text + N);
			}

			// mutation functions

			/*--function--
			assign:			return - void
			pass: string to copy over
			template: encoding of string
			action: initialize internal text string with a passed text string
			notes: deletes the internal text string first
			*/
			template <typename dchar,size_t N>
			void assign(const dchar(&_text)[N]) {
				delete[] text;
				rawAssign(make_stringview(_text));
			}
			template <typename dchar>
			void assign(const dchar* _text,size_t N) {
				delete[] text;
				rawAssign(make_stringview(_text,_text + N));
			}
			template <typename dchar>
			void assign(utfstring<dchar>& str) {
				delete[] text;
				rawAssign(str.view);
			}

			// text manipulation functions

			/*--function--
			substr:			return - utfstring<ch>
			pass: beginning character of new string{1}, end character of the new string{-1}
			action: returns a substring taken from the internal text string from character idx_b to character idx_e
			notes: index 1 corresponds to the first character in the string and -1 to the last
					index 2 to the second character and -2 to the next to last etc.
			notes: guaranteed to add delimiting characters to the end of the string
			throws: character idx_b lies after idx_e in the text string
			*/
			utfstring<ch> substr(int idx_b = 1,int idx_e = -1) {
				if(!is_valid_range(idx_b,idx_e)) throw("Error: invalid index range");

				auto sub = make_stringview(text + view.codeidx(idx_b),text + view.codeidx(idx_e) + (idx_e>0));
				auto end = make_stringview(text + view.codeidx(-1), text + view.codeidx(0));

				ch* substr = new ch[sub.codeunits()+end.codeunits()];

				end.to<chartype>(sub.to<chartype>(utf_encoder(substr)));

				return utfstring<ch>(substr,substr + sub.codeunits<chartype>() + end.codeunits<chartype>());
			}
			/*--function--
			copy:			return - utfstring<ch>
			pass: void
			action: returns a copy of the string
			notes: wrapper for substr with default arguments
			*/
			utfstring<ch> copy() { return substr(); }
			/*--function--
			splice:			return - utfstring<ch>
			pass: text string to splice in,{ size },character after split{-1},beginning of substring{1}, end of substring{-1}
			template: encoding of string
			action: create a new string by inserting the passed text substring into the internal text string at character idx_sp
			notes: guaranteed to add delimiting characters to the end of the string
			throws: character idx_b lies after idx_e in the text string
			*/
			template <typename dchar>
			utfstring<ch> splice(dchar* _text,size_t N,int idx_sp = -1,int idx_b = 1,int idx_e = -1) {
				if(!is_valid_range(idx_b,idx_e)) throw("Error: invalid index range");

				auto temp = make_stringview(_text,_text + N);
				return rawSplice(make_stringview(_text + temp.codeidx(idx_b),_text + temp.codeidx(idx_e) + (idx_e > 0)),idx_sp);
			}			
			template<typename dchar,size_t N>
			utfstring<ch> splice(dchar(&_text)[N],int idx_sp = -1,int idx_b = 1,int idx_e = -1) {
				if(!is_valid_range(idx_b,idx_e)) throw("Error: invalid index range");

				auto temp = make_stringview(_text);
				return rawSplice(make_stringview(_text + temp.codeidx(idx_b),_text + temp.codeidx(idx_e) + (idx_e > 0)),idx_sp);
			}
			template <typename dchar>
			utfstring<ch> splice(utfstring<dchar>& str,int idx_sp = -1,int idx_b = 1,int idx_e = -1) {
				if(!is_valid_range(idx_b,idx_e)) throw("Error: invalid index range");

				return rawSplice(make_stringview(str.text + str.view.codeidx(idx_b),
					str.text + str.view.codeidx(idx_e) + (idx_e > 0)),idx_sp);
			}
			/*--function--
			cut:			return - utfstring<ch>
			pass: beginning of substr{1},end of substring{-1}
			action: return a copy of the internal text string with the specified substr removed
			notes: guaranteed to add delimiting characters to the end of the string
			*/
			utfstring<ch> cut(int idx_b = 1,int idx_e = -1) {
				auto piece1 = make_stringview(text,text+view.codeidx(idx_b));
				auto piece2 = make_stringview(text + view.codeidx(idx_e) + (idx_e > 0),text + view.codeidx(0));
				
				ch* str = new ch[piece1.codeunits() + piece2.codeunits()];
				piece2.to<chartype>(piece1.to<chartype>(utf_encoder(str)));

				return utfstring<ch>(str,str + piece1.codeunits() + piece2.codeunits());
			}

			// boolean operators

			/*--function--
			operator==:		return - bool
			pass: string to compare
			template: encoding of string(inferred)
			action: Compares two strings for equality
			notes: Does an arraywise comparison. Two string are equal if
					and only if their internal text strings are exactly equal
			notes: Since it does an arraywise comparison, it can't do a case independent comparison
			*/
			template <typename dchar>
			bool operator==(utfstring<dchar>& str) {
				if(str.strsize<ch>() == strsize()) {
					ch* comp = str.text_as<ch>();
					for(auto i = 0; i != strsize(); ++i)
						if(comp[i] != text[i])
							return false;
					return true;
				}
				return false;
			}
			/*--function--
			pass: string to compare
			template: encoding of string(inferred)
			operator!=:		return - bool
			action: opposite of operator==
			*/
			template <typename dchar>
			bool operator!=(utfstring<dchar>& str) {
				return !operator==(str);
			}

			// assignment operators

			/*--function--
			operator=:		return - string<ch>&
			pass: string to use in assignment
			template: encoding of string(inferred)
			action: initializes the internal text string with a passed text string
			notes: wrapper for assign
			*/
			template <typename dchar>
			utfstring<ch>& operator=(utfstring<dchar>& str) {
				assign(str);
				return *this;
			}
			utfstring<ch>& operator=(utfstring<ch>&& str) {
				text = str.text;
				view = str.view;
				str.text = nullptr;
				return *this;
			}
			template <typename dchar,size_t N>
			utfstring<ch>& operator=(dchar (&str) [N]) {
				assign(str);
				return *this;
			}

			// concatentation operators

			/*--function--
			operator+:		return - utfstring<ch>
			pass: string to append
			template: encoding of string(inferred)
			action: appends the passed text string to the internal text string into a new text string
			notes: wrapper for splice with default arguments
			*/
			template <typename dchar>
			utfstring<ch> operator+(utfstring<dchar>& str) {
				return splice(str);
			}
			template <typename dchar,size_t N>
			utfstring<ch> operator+(const dchar (&str) [N]) {
				return splice(str,N);
			}

			// other functions

			/*--function--
			begin:			return - decltype(view.begin())
			pass: void
			action: returns a non-mutable iterator over the internal text string
			notes: wrapper for view.begin()
			*/
			auto begin() -> decltype(view.begin()) { return view.begin(); }
			/*--function--
			end:			return - decltype(view.end())
			pass: void
			action: returns a non-mutable iterator at the end of the internal text string
			notes: wrapper for view.end()
			*/
			auto end() -> decltype(view.end()) { return view.end(); }
			/*--function--
			const ch*:		return - const ch*
			pass: (const ch*)<string>
			action: cast operator to the internal text string
			*/
			explicit operator const ch*() { return this->text; }
	};

	// use typedefs
	typedef utfstring<char> utf8;
	typedef utfstring<char> ansi;
	typedef utfstring<char> ascii;
	typedef utfstring<char16_t> utf16;
	typedef utfstring<char16_t> unicode;
	typedef utfstring<char32_t> utf32;
}

namespace std {
	
	// external begin and end for use in ranged for loop
	template <typename ch>
	auto begin(utf::utfstring<ch>& str) -> decltype(str.begin()) { return str.begin(); }
	template <typename ch>
	auto end(utf::utfstring<ch>& str) -> decltype(str.end()) { return str.end(); }

	// overloaded input stream operator
	/*--function--
	operator<<[stream,ch]:		returns - stream&
	pass: stream& to input to, utfstring<ch>& to input from
	action: iterates over the text of the string pushing the individual chars into the stream
	
	possible to edit so that text is automatically converted into the type of stream ???
	*/
	template <class stream,typename ch>
	stream& operator<<(stream& str,utf::utfstring<ch>& text) {
		for(auto c : text)
			str << (ch)c;
		return str;
	}
	/*
	template <template<typename> class stream,typename ch>
	stream<ch>& operator<<(stream<ch>& str,utf::utfstring<ch>& text) {

	}
	*/

}
