LeekPP: A Bloom Filter Library
==============================

A header-only library is a set of data structures and functions for dealing with
 [Bloom filters](https://en.wikipedia.org/wiki/Bloom_filter).
It is hosted on [GitHub](https://github.com/tgockel/leekpp) and has online
 [Doxygen documentation](http://tgockel.github.io/leekpp/) (which you might be looking at right now).

Usage
-----

    #include <leekpp/bloom_filter.hpp>

    #include <iostream>
    #include <string>

    int main()
    {
        // Create an "ideally sized" Bloom filter with a 5% FPR and 1000 items
        auto filter = leekpp::bloom_filter<std::string>::create_ideal(0.05, 1000);

        // Get items from the command line and insert them into the filter until we hit a blank
        std::string ss;
        while (std::cin >> ss && !ss.empty())
            filter.insert(ss);

        // Test items from the command line until we EOF
        while (std::cin >> ss)
            std::cout << "Is '" << ss << "' in the filter? "
                      << (filter.count(ss) ? "maybe" : "no")
                      << std::endl;
    }

F.A.Q.
------

### Is it thread-safe?

If you use `thread_safe_bloom_filter`.

### Why is this called `leekpp`?

The library is all about the Bloom filter, so I thought of the disgusting culinary masterpiece of the Bloomin' Onion.
Since Bloom filters have the property of false positives, one might consider them as a "leaky" data structure (a perfect
 filter would not let anything unspecified through).
A leek is a type of onion _and_ is a homophone of "leak."

License
-------

> Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with
> the License. You may obtain a copy of the License at
>
>  [http://www.apache.org/licenses/LICENSE-2.0](http://www.apache.org/licenses/LICENSE-2.0)
>
> Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
> an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
> specific language governing permissions and limitations under the License.
