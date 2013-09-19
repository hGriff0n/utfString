utfString
=========

small custom unicode string library

usage
=====

the library consists of three `.h` files
to use in a project just include `utfstring.h`
all functions and classes are exported under the `bast` namespace

This imports two main classes:
`utfstring` and `stringview`

the `stringview` class is used internally by `utfstring` to convert between unicode encodings.
`stringview` relies on iterators and raw pointers to the string sequence
and never takes ownership of memory,creates strings,etc.

for convenience, a `make_stringview` function is provided that generates a `stringview` from the passed args:

`
char* text = "Hello, World!"	// size is 14
std::vector<char16_t> u16data;
auto literal = make_stringview("This is a string literal");
auto pointer = make_stringview(text,text+14);
auto iterate = make_stringview(u16data.begin(),u16data.end());	// stringview over a UTF-16 data
`

To convert between encoding, call the `to<type>` member function with a non-const iterator:
`
>>>>>>> readme update
std::string utf8str;
iterate.to<utf8_t>(std::back_inserter(utf8str));
// moves any UTF-16 text stored in u16data into utf8str as UTF-8 text
// std::string is used here as an example

`
It is possible to get the size of any text string in a different encoding
through the `codeunits<type>` member function(type defaults to the current encoding):
`
auto utf8size = iterate.codeunits<utf8_t>();
auto utf16size = iterate.codeunits();
`
You can also iterate over the contents of the passed string like an stl container
`
for (auto it : pointer)
	std::cout << (char)(*it);			// outputs "Hello, World!"
`

the `utfstring` class is an immutable, small unicode-friendly string
unlike `stringview`, `utfstring` owns it's text range
any member function that involves the internal text returns on a new string
a process that sometimes involves copying the entire string over (except for the const ch* operator)

several typedefs are defined on the utfstring class over specific encodings:
	`utf8,ansi,ascii <- utfstring<char>
	utf16,unicode <- utfstring<char16_t>
	utf32 <- utfstring<char32_t>`
	`
	
constructors: copy,move,literal,text,range
	text_as<dchar>: size_t&
	assign<dchar,size_t>: &dchar[],void
	substr: int,int -> string<ch>
	copy: string<ch>
	splice<dchar,size_t>: &dchar[],int,int,int -> string<ch>
	cut: int,int -> string<ch>
	operator==<dchar>: string<dchar>& -> bool
	operator!=<dchar>: string<dchar>& -> bool
	operator=<dchar,size_t>: &dchar[] -> string<ch>&
	operator+<dchar,size_t>: &dchar[] -> string<ch>
	begin: auto	"decltype(view.begin())"
	end: auto "decltype(view.end())"

todo
====

add overloads that will accept std::strings,std::vectors,etc. to utfstring

add make_utfstring function that creates a string with the encoding of the args ?

create a large_string class that allows direct manipulation of the internal text string