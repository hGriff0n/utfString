

#include "string.h"

#include <iostream>
#include <cassert>

using namespace utf;
using namespace std;

int main(int argc,const char* argv[]) {

	utf8 hello("Hello,");			// Create a string in UTF_8 encoding
	utf16 world(" World!");			// Create a string in UTF_16 encoding
	utf32 cruel(" Cruel");			// Create a string in UTF_32 encoding

	utf8 hello_world = hello + world;		// Create a string in UTF_8 encoding
											// Initialize with the string formed by
											// Combining hello and world (utf8+utf16 -> utf8 => utf8)

	utf16 sad = hello_world.splice(cruel,7);		// Create a string in UTF_16 encoding
													// Initialize with the string formed by the
													// Splicing of sad into hello_world at character 7
													// automatic conversion from utf32 to utf8 to utf16 encoding

	utf8 truth = sad.substr(8);		// Create a string in UTF_8 encoding.
									// Initialize with the string formed by
									// A substring of cruel from the eighth('C') character onwards
									// Automatic conversion from utf16 to utf8 encoding

	// output demonstrations

	cout << hello.encoding() << " hello: " << hello << endl;
	cout << world.encoding() << " world: " << world.to<char>() << endl;		// necessary since cout is tied to type char
	cout << cruel.encoding() << " cruel: " << cruel.to<char>() << endl;		// would otherwise output a string of numbers
	cout << hello_world.encoding() << " hello_world: " << hello_world << endl;
	cout << sad.encoding() << " sad: " << sad.to<char>() << endl;
	cout << truth.encoding() << " truth: " << truth << endl;

	cin.get();
}