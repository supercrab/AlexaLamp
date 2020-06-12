#ifndef STRING_STREAM_H_
#define STRING_STREAM_H_

#include <Stream.h>

// StringStream class / Library for Arduino
//
// This peace of code is usable when you need Stream object which has to store/load data to/from Arduino String object.
// 
// https://gist.github.com/arcao/3252bb6e5e52493f03726ec32e61395c
//
class StringStream : public Stream{
	
	public:
		StringStream(String &s) : string(s), position(0) { }

		// Stream methods
		virtual int available() { return string.length() - position; }
		virtual int read() { return position < string.length() ? string[position++] : -1; }
		virtual int peek() { return position < string.length() ? string[position] : -1; }
		virtual void flush() { };

		// Print methods
		virtual size_t write(uint8_t c) { string += (char)c; return 1;};

	private:
		String &string;
		unsigned int length;
		unsigned int position;
};

#endif 