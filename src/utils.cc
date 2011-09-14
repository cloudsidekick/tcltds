//########################################################################
// Copyright 2011 Cloud Sidekick
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//########################################################################

#include "utils.h"
#include <cstdlib>
#include <climits>
#include <algorithm>
using namespace std;

// I am TIRED of not being able to easily convert an integer to a standard
// string, so I wrote this function.  The name is taken from a standard
// function that I used to use in Borland C++

string 
itoa(long long value, int base) {
    const int MaxDigits = 35;
    string buf;

    buf.reserve(MaxDigits);

    if (base < 2 || base > 16)
        return buf;

    int quotient = value;

    do {
        buf += "0123456789abcdef"[abs(quotient % base)];
        quotient /= base;
    } while (quotient);

    if (value < 0 && base == 10)
        buf += '-';

    reverse(buf.begin(), buf.end());

    return buf;
}
