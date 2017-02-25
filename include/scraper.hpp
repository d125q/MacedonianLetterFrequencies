#ifndef SCRAPER_H
#define SCRAPER_H

#include "http_client.hpp"

class scraper : public http_client {
public:
    using http_client::http_client;
    void process_content(const std::string& content);
};

#endif /* SCRAPER_H */
