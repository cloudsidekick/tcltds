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

#ifndef __TOKENIZER_H__
#define __TOKENIZER_H__

#include <string>

class Tokenizer {
    public:
        Tokenizer(std::string input = "", char delimiter = ' ') {
            index = 0;
            delim = delimiter;
            if (input.length()) {
                data = input;
            }
            else {
                data = "";
            }
        }

        ~Tokenizer() {};

        bool hasNext() {
            if (index >= data.length())
                return false;

            while (data[index] == delim && index <= data.length())
                index++;

            return (index < data.length());
        }

        bool next(std::string &out) {
            if (index > data.length())
                return false;

            int begin;
            while (index <= data.length() && data[index] == delim)
                index++;
            begin = index;

            while (index < data.length() && data[index] != delim)
                index++;

            out = data.substr(begin, index - begin);

            while (index <= data.length() && data[index] == delim)
                index++;

            return true;
        }

        void append(std::string input) {
            data += input;
        }

        bool remaining(std::string &out) {
            out = data.substr(index);
        }

        void setDelimiter(char delimiter) {
            delim = delimiter;
        }

        void reset(std::string input = "") {
            if (input != "")
                data = input;
            index = 0;
        }

    private:
        int index;
        std::string data;
        char delim;
};

#endif // __TOKENIZER_H_
