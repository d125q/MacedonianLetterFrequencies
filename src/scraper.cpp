#include <iostream>
#include <fstream>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <unicode/unistr.h>
#include <unicode/regex.h>
#include <unicode/ustdio.h>
#include "scraper.hpp"
#include "counter.hpp"

#define ALPHABET_REGEX (u8"а|б|в|г|д|ѓ|е|ж|з|ѕ|и|ј|к|л|љ|м|н|њ|о|п|р|с|т|ќ|у|ф|х|ц|ч|џ|ш")

counter<UChar> letter_counter;

void scraper::process_content(const std::string& content)
{
    UErrorCode status{U_ZERO_ERROR};
    icu::UnicodeString unicode_content{icu::UnicodeString::fromUTF8(content).toLower()};
    icu::RegexMatcher matcher(ALPHABET_REGEX, unicode_content,
			      0, status);

    if (U_FAILURE(status)) {
	std::cerr << "Error constructing icu::RegexMatcher\n";
	return;
    }

    int32_t offset{-1};

    while (matcher.find(offset + 1, status)) {
	offset = matcher.start(status);
	UChar matched_letter{unicode_content.charAt(offset)};
	letter_counter.increment(matched_letter);
    }
}

int main(int argc, char* argv[])
{
    try {
	if (argc != 2) {
	    std::cerr << "Usage: " << argv[0] << " <filename>\n";
	    std::cerr << "Example:\n";
	    std::cerr << "    " << argv[0] << " data/library.csv\n";
	    return -1;
	}
	
	std::ifstream library(argv[1]);
	std::string line;
	std::vector<scraper*> scrapers;
	boost::asio::io_service io_service;

	while (std::getline(library, line)) {
	    std::vector<std::string> tokens;
	    boost::split(tokens, line, boost::is_any_of(","));
	    assert(tokens.size() == 2);
	    scrapers.push_back(new scraper(io_service, tokens[0], tokens[1]));
	}

	library.close();
	io_service.run();
	
	letter_counter.for_each([&](UChar letter, unsigned long long count)
				{
				    double frequency = static_cast<double>(count) / letter_counter.size();
				    u_printf("%C,%f\n", letter, frequency);
				});

	for (auto it = scrapers.begin(); it != scrapers.end(); ++it)
	    delete *it;
    }
    catch (std::exception& e) {
	std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
