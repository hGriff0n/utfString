utfstring
=========

A small unicode string library

Usage
=====

The library consists of three header files with all functions and classes under the `utf` namespace. To use in a project just include `utfstring.h`. The library consists of two main classes, `stringview` and `utfstring`

stringview
===

The `stringview` class is used internally by `utfstring` primarily to convert between encodings. 
`stringview` relies on iterators and raw pointers to the string sequence and never takes ownership of memory, creates strings, etc.  

For convenience, a `make_stringview` function is provided that generates a `stringview` from the passed arguments

      char* text = "Hello, World!"	// size is 14
      std::vector<char16_t> u16data;
      auto literal = make_stringview("This is a string literal");
      auto pointer = make_stringview(text,text+14);
      auto iterator = make_stringview(u16data.begin(),u16data.end());	// stringview over UTF-16 data  

To convert between encoding using `stringview`, call `to<type>` with a non-const iterator  

     std::string utf8str;
     iterator.to<utf8_t>(std::back_inserter(utf8str));'  

It is possible to get the size of any text string in a different encoding
by calling `codeunits<type>` (type defaults to the current encoding)

     auto utf8size = iterate.codeunits<utf8_t>();
     auto utf16size = iterate.codeunits();  

You can of course iterate over the contents of the passed string

     for (auto it : pointer)
         std::cout << (char)(*it);			// outputs "Hello, World!"


utfstring
===

The `utfstring` class is an immutable, small unicode-friendly string. Unlike `stringview`, `utfstring` owns it's text range. However any function that would modify the internal text range, instead a new string with the requested modifications.

Several typedefs are provided that specify `utfstring` on specific encodings:

     utf8,ansi,ascii on utfstring<char>

     utf16,unicode on utfstring<char16_t>

     utf32 on utfstring<char32_t>

The creation of any `utfstring` is trivial regardless of the encoding of the original string as all conversions are done implicitly

    utf8 hello("Hello,");

    utf16 utf16_hello(hello);

To explicitly convert to a specific encoding, the `to` member is provided. It is also possible to gain a copy of the internal text array in any encoding through the `text_as` member.

Various common string operations, such as `substr`, `splice`, `cut`, and `+` are also provided. By default, these operations return a string of the same encoding as the parent. All operations work on character and not array indices. By default these operations are 1-indexed. All operations also support reverse indexing.

    utf16 world(" World");  // Conversion from char string to utf16

    utf32 cruel(" Cruel");

    utf8 hello_world = hello + world;

    utf16 sad = hello_world.splice(cruel, 7);  // Hello, Cruel World

    utf32 cynic = sad.cut(8);  // Cruel World
	

todo
====

add overloads that will accept std::strings,std::vectors,etc. to utfstring  
add `make_utfstring` function that creates a string with the encoding of the args ?