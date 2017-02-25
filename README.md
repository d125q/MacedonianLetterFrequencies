# Introduction
This is a small utility written in C++11 which scrapes texsts from http://www.anarhisticka-biblioteka.org/library/ and calculates the frequencies of Macedonian letters.  As far as I'm aware, there are no such statistics for the Macedonian language on the WWW.

# Building from source
Clone the repository, and run
```sh
$ cmake .
$ make
```
You will need a C++11-compliant compiler, the [Boost C++ Libraries](http://www.boost.org/), and the [International Components for Unicode](http://site.icu-project.org/).

A result of the execution can be found in the file `frequencies.csv`.
