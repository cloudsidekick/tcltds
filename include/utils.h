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

#ifndef __UTILS_H__
#define __UTILS_H__

#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <string>

class to_lower {
    public:
        char operator() (char c) const {
            return tolower(c);
        }
};

class to_upper {
    public:
        char operator() (char c) const {
            return toupper(c);
        }
};

inline std::string trim_right(const std::string &source, const std::string &t = " ") {
    std::string str = source;
        return str.erase(str.find_last_not_of(t) + 1);
}

inline std::string trim_left(const std::string &source, const std::string &t = " ") {
    std::string str = source;
        return str.erase(0, str.find_first_not_of(t));
}

inline std::string trim(const std::string &source, const std::string &t = " ") {
    std::string str = source;
        return trim_left(trim_right(str, t), t);
}

std::string itoa(long long value, int base = 10);
#endif // #define __UTILS_H__
