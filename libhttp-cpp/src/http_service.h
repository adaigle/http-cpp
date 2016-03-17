#ifndef HTTP_SERVICE_H
#define HTTP_SERVICE_H

#include <atomic>
#include <string>

class http_service
{
public:
    http_service(const std::string& name, const std::string& path);

    std::string execute(std::string& request);
private:

    const std::string name;
    std::string path;
};

#endif
