#include "web_service_manager.h"


void WebServiceManager::runBootProcess()
{
    for (const auto& endpoint : m_endpoints)
    {
        endpoint.add_to_server(m_web_server);
    }

    //m_web_server.begin();
}
