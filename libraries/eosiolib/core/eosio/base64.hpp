/*
   base64 encoding and decoding with C++.
   More information at
     https://renenyffenegger.ch/notes/development/Base64/Encoding-and-decoding-base-64-with-cpp
     https://github.com/ReneNyffenegger/cpp-base64

   Version: 2.rc.09 (release candidate)

   Copyright (C) 2004-2017, 2020-2022 René Nyffenegger

   This source code is provided 'as-is', without any express or implied
   warranty. In no event will the author be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this source code must not be misrepresented; you must not
      claim that you wrote the original source code. If you use this source code
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original source code.

   3. This notice may not be removed or altered from any source distribution.

   René Nyffenegger rene.nyffenegger@adp-gmbh.ch

*/

#include "check.hpp"

#include <algorithm>
#include <stdexcept>
#if __cplusplus >= 201703L
#include <string_view>
#endif  // __cplusplus >= 201703L

namespace eosio {

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len);
inline std::string base64_encode(char const* bytes_to_encode, unsigned int in_len) { return base64_encode( (unsigned char const*)bytes_to_encode, in_len); }
std::string base64_encode( std::string_view enc );
std::string base64_decode( std::string_view encoded_string);

namespace detail {
std::string base64_encode     (std::string const& s, bool url = false);

std::string base64_decode(std::string const& s, bool remove_linebreaks = false);
std::string base64_encode(unsigned char const*, size_t len, bool url = false);

std::string base64_encode     (std::string_view s, bool url = false);

std::string base64_decode(std::string_view s, bool remove_linebreaks = false);

unsigned int pos_of_char(const unsigned char chr) {
    // Return the position of chr within base64_encode()
    const unsigned char from_base64_chars[256] = {
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 62, 64, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
    64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 63,
    64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
    };
    auto c = from_base64_chars[chr];
    eosio::check(c != 64, "encountered non-base64 character");

    return c;
}

std::string insert_linebreaks(std::string str, size_t distance) {
    // Provided by https://github.com/JomaCorpFX
    if (!str.length()) {
        return std::string{};
    }

    size_t pos = distance;

    while (pos < str.size()) {
        str.insert(pos, "\n");
        pos += distance + 1;
    }

    return str;
}

template <typename String, unsigned int line_length>
std::string encode_with_line_breaks(String s) {
  return insert_linebreaks(base64_encode(s, false), line_length);
}

template <typename String>
std::string encode_pem(String s) {
  return encode_with_line_breaks<String, 64>(s);
}

template <typename String>
std::string encode_mime(String s) {
  return encode_with_line_breaks<String, 76>(s);
}

template <typename String>
std::string encode(String s, bool url) {
  return base64_encode(reinterpret_cast<const unsigned char*>(s.data()), s.length(), url);
}

std::string base64_encode(unsigned char const* bytes_to_encode, size_t in_len, bool url) {

    const size_t len_encoded = (in_len +2) / 3 * 4;
    const unsigned char trailing_char = url ? '.' : '=';

    // Includes performance improvement from unmerged PR: https://github.com/ReneNyffenegger/cpp-base64/pull/27

    // Depending on the url parameter in base64_chars, one of
    // two sets of base64 characters needs to be chosen.
    // They differ in their last two characters.
    const char* base64_chars[2] = {
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789"
             "+/",
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789"
             "-_"};
    
    // Choose set of base64 characters. They differ
    // for the last two positions, depending on the url
    // parameter.
    // A bool (as is the parameter url) is guaranteed
    // to evaluate to either 0 or 1 in C++ therefore,
    // the correct character set is chosen by subscripting
    // base64_chars with url.
    const char* base64_chars_ = base64_chars[url];

    std::string ret;
    ret.reserve(len_encoded);

    unsigned int pos = 0;

    while (pos < in_len) {
        ret.push_back(base64_chars_[(bytes_to_encode[pos + 0] & 0xfc) >> 2]);

        if (pos+1 < in_len) {
           ret.push_back(base64_chars_[((bytes_to_encode[pos + 0] & 0x03) << 4) + ((bytes_to_encode[pos + 1] & 0xf0) >> 4)]);

           if (pos+2 < in_len) {
              ret.push_back(base64_chars_[((bytes_to_encode[pos + 1] & 0x0f) << 2) + ((bytes_to_encode[pos + 2] & 0xc0) >> 6)]);
              ret.push_back(base64_chars_[  bytes_to_encode[pos + 2] & 0x3f]);
           }
           else {
              ret.push_back(base64_chars_[(bytes_to_encode[pos + 1] & 0x0f) << 2]);
              ret.push_back(trailing_char);
           }
        }
        else {

            ret.push_back(base64_chars_[(bytes_to_encode[pos + 0] & 0x03) << 4]);
            ret.push_back(trailing_char);
            ret.push_back(trailing_char);
        }

        pos += 3;
    }


    return ret;
}

template <typename String>
static std::string decode(const String& encoded_string, bool remove_linebreaks) {
 //
 // decode(…) is templated so that it can be used with String = const std::string&
 // or std::string_view (requires at least C++17)
 //

    if (encoded_string.empty()) return std::string{};

    if (remove_linebreaks) {

       std::string copy(encoded_string);

       copy.erase(std::remove(copy.begin(), copy.end(), '\n'), copy.end());

       return base64_decode(copy, false);
    }

    size_t length_of_string = encoded_string.length();
    size_t pos = 0;

 //
 // The approximate length (bytes) of the decoded string might be one or
 // two bytes smaller, depending on the amount of trailing equal signs
 // in the encoded string. This approximation is needed to reserve
 // enough space in the string to be returned.
 //
    size_t approx_length_of_decoded_string = length_of_string / 4 * 3;
    std::string ret;
    ret.reserve(approx_length_of_decoded_string);

    while (pos < length_of_string) {
    //
    // Iterate over encoded input string in chunks. The size of all
    // chunks except the last one is 4 bytes.
    //
    // The last chunk might be padded with equal signs or dots
    // in order to make it 4 bytes in size as well, but this
    // is not required as per RFC 2045.
    //
    // All chunks except the last one produce three output bytes.
    //
    // The last chunk produces at least one and up to three bytes.
    //
       eosio::check(pos+1 < length_of_string, "wrong encoded string size");
       size_t pos_of_char_1 = pos_of_char(encoded_string.at(pos+1) );

    //
    // Emit the first output byte that is produced in each chunk:
    //
       ret.push_back(static_cast<std::string::value_type>( ( (pos_of_char(encoded_string.at(pos+0)) ) << 2 ) + ( (pos_of_char_1 & 0x30 ) >> 4)));

       if ( ( pos + 2 < length_of_string  )       &&  // Check for data that is not padded with equal signs (which is allowed by RFC 2045)
              encoded_string.at(pos+2) != '='     &&
              encoded_string.at(pos+2) != '.'         // accept URL-safe base 64 strings, too, so check for '.' also.
          )
       {
       //
       // Emit a chunk's second byte (which might not be produced in the last chunk).
       //
          unsigned int pos_of_char_2 = pos_of_char(encoded_string.at(pos+2) );
          ret.push_back(static_cast<std::string::value_type>( (( pos_of_char_1 & 0x0f) << 4) + (( pos_of_char_2 & 0x3c) >> 2)));

          if ( ( pos + 3 < length_of_string )     &&
                 encoded_string.at(pos+3) != '='  &&
                 encoded_string.at(pos+3) != '.'
             )
          {
          //
          // Emit a chunk's third byte (which might not be produced in the last chunk).
          //
             ret.push_back(static_cast<std::string::value_type>( ( (pos_of_char_2 & 0x03 ) << 6 ) + pos_of_char(encoded_string.at(pos+3))   ));
          }
       }

       pos += 4;
    }

    return ret;
}

std::string base64_decode(std::string const& s, bool remove_linebreaks) {
   return decode(s, remove_linebreaks);
}

std::string base64_encode(std::string const& s, bool url) {
   return encode(s, url);
}

std::string base64_encode(std::string_view s, bool url) {
   return encode(s, url);
}

std::string base64_decode(std::string_view s, bool remove_linebreaks) {
   return decode(s, remove_linebreaks);
}

} // anonymous namespace


std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
   return detail::base64_encode(bytes_to_encode, in_len, false);
}
std::string base64_encode(std::string_view enc) {
   return detail::base64_encode(enc, false);
}
std::string base64_decode(std::string_view encoded_string) {
   return detail::base64_decode(encoded_string, false);
}

std::string base64url_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
   return detail::base64_encode(bytes_to_encode, in_len, true);
}
std::string base64url_encode(std::string_view enc) {
   return detail::base64_encode(enc, true);
}
std::string base64url_decode(std::string_view encoded_string) {
   return detail::base64_decode(encoded_string, true);
}

} // namespace eosio
