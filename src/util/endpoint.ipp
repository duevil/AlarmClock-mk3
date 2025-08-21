#ifndef ENDPOINT_IPP
#define ENDPOINT_IPP


// ReSharper disable CppDFAMemoryLeak


namespace endpoint
{
    /*-- Endpoint member functions definitions --*/

    inline Endpoint::Endpoint(Handler* handler): _handler(handler)
    {
        if (_handler)
        {
            // hook the deleted flag to the handler and add it to the server
            _handler->deleted = &_deleted;
        }
    }

    inline auto& Endpoint::getUri() const
    {
        return _handler && !_deleted ? _handler->getUri() : emptyString;
    }

    inline auto Endpoint::getMethod() const
    {
        return _handler && !_deleted ? _handler->getMethod() : WebRequestMethodComposite{};
    }

    inline void Endpoint::add_to_server(AsyncWebServer& server) const
    {
        if (_handler) server.addHandler(_handler);
    }

    inline void Endpoint::remove_from_server(AsyncWebServer& server)
    {
        // calling server.removeHandler will invoke the handler's destructor,
        // ensuring that the deleted flag will be set to true
        if (_handler && server.removeHandler(_handler)) _handler = nullptr;
    }


    /*-- Endpoint HandlerWrapper member functions definitions --*/

    template <typename T>
    bool Endpoint::HandlerWrapper<T>::canHandle(AsyncWebServerRequest* request) const
    {
        return _handler.canHandle(request);
    }

    template <typename T>
    void Endpoint::HandlerWrapper<T>::handleRequest(AsyncWebServerRequest* request)
    {
        _handler.handleRequest(request);
    }

    template <typename T>
    void Endpoint::HandlerWrapper<T>::handleUpload(AsyncWebServerRequest* request, const String& filename, size_t index,
                                                   uint8_t* data, size_t len, bool final)
    {
        _handler.handleUpload(request, filename, index, data, len, final);
    }

    template <typename T>
    void Endpoint::HandlerWrapper<T>::handleBody(AsyncWebServerRequest* request, uint8_t* data, size_t len,
                                                 size_t index, size_t total)
    {
        _handler.handleBody(request, data, len, index, total);
    }

    template <typename T>
    bool Endpoint::HandlerWrapper<T>::isRequestHandlerTrivial() const
    {
        return _handler.isRequestHandlerTrivial();
    }


    /*-- Endpoint Builder member functions definitions --*/

    inline Endpoint::Builder& Endpoint::Builder::withMethod(WebRequestMethod method)
    {
        _method |= method;
        return *this;
    }

    template <typename Arg> requires is_valid_endpoint_arg<Endpoint::Request, Endpoint::JRequest, Arg>
    Endpoint Endpoint::Builder::build(Arg&& arg) const
    {
        if constexpr (is_plain<Arg>)
        {
            // the created handler will be owned by the server, which will handle deletion
            auto handler = new HandlerWrapper<AsyncCallbackWebHandler>();
            (*handler)->setUri(_uri);
            (*handler)->setMethod(_method);
            (*handler)->onRequest(std::forward<Arg>(arg));
            return handler;
        }
        else if constexpr (is_json<Arg>)
        {
            // the created handler will be owned by the server, which will handle deletion
            auto handler = new HandlerWrapper<AsyncCallbackJsonWebHandler>(_uri);
            (*handler)->setMethod(_method);
            (*handler)->onRequest(std::forward<Arg>(arg));
            return handler;
        }
        else if constexpr (is_func<Request, Arg>)
        {
            return build([arg](AsyncWebServerRequest* r)
            {
                Request request{r};
                arg(request);
            });
        }
        else if constexpr (is_func<JRequest, Arg>)
        {
            return build([arg](AsyncWebServerRequest* r, JsonVariant& json)
            {
                JRequest request{r, json};
                arg(request);
            });
        }
        else if constexpr (is_getter<Arg>)
        {
            return build([&arg](AsyncWebServerRequest* r)
            {
                Request{r}.jr().set(arg);
            });
        }
        else if constexpr (is_setter<Arg>)
        {
            return build([&arg](AsyncWebServerRequest* r, JsonVariant& json)
            {
                using T = std::remove_reference_t<Arg>;
                if (json.is<T>())
                {
                    arg = json.as<T>();
                    Request{r}.jr().set(arg);
                }
                else
                {
                    r->send(400 /*Bad Request*/);
                }
            });
        }

        return nullptr;
    }

    inline Endpoint Endpoint::Builder::file(const char* path, FS& fs, const char* cache_control) const
    {
        return new HandlerWrapper<AsyncStaticWebHandler>(_uri.c_str(), fs, path, cache_control);
    }

    inline Endpoint Endpoint::Builder::dir(const char* path, FS& fs, const char* cache_control) const
    {
        auto handler = new HandlerWrapper<AsyncStaticWebHandler>(_uri.c_str(), fs, path, cache_control);
        (*handler)->setIsDir(true);
        return handler;
    }


    /*-- Endpoint request member functions definitions --*/

    inline Endpoint::Request::~Request()
    {
        if (_request && !_request->isSent())
        {
            if (_response)
            {
                _response->setLength();
                _request->send(_response);
            }
            else if (_text)
            {
                _request->send(200, asyncsrv::T_text_plain, *_text);
            }
            else
            {
                _request->send(204 /*No Content*/);
            }
        }
    }

    inline String& Endpoint::Request::text()
    {
        _text = std::make_unique<String>();
        return *_text;
    }

    inline JsonVariant& Endpoint::Request::jr()
    {
        if (!_response)
            // using new because the response will be deleted by the response handler
            _response = new AsyncJsonResponse(true);

        return _response->getRoot();
    }

    inline JsonArray Endpoint::Request::jrArr()
    {
        if (!_response)
            // using new because the response will be deleted by the response handler
            _response = new AsyncJsonResponse(true);

        return _response->getRoot().as<JsonArray>();
    }
}


#endif //ENDPOINT_IPP
