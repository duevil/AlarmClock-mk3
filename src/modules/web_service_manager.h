#ifndef WEB_SERVICE_MANAGER_H
#define WEB_SERVICE_MANAGER_H

#include "util/boot_process.hpp"
#include "util/endpoint.hpp"


class WebServiceManager final : BootProcess
{
public:
    explicit WebServiceManager(auto port, auto&&... endpoints)
        : BootProcess("Web services initialized"), m_web_server{port}
    {
        m_endpoints.reserve(sizeof...(endpoints));
        (m_endpoints.emplace_back(std::forward<decltype(endpoints)>(endpoints)), ...);
    }

private:
    void runBootProcess() override;

    AsyncWebServer m_web_server;
    std::vector<endpoint::Endpoint> m_endpoints;
};


#endif //WEB_SERVICE_MANAGER_H
