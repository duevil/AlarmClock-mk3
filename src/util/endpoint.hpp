#ifndef ENDPOINT_HPP
#define ENDPOINT_HPP

#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>


namespace endpoint
{
    template <typename Request>
    using HandlerFunc = std::function<void(Request&)>;

    template <typename Request, typename T>
    concept is_func = std::is_constructible_v<HandlerFunc<Request>, T>;

    template <typename T>
    concept is_plain = std::is_constructible_v<ArRequestHandlerFunction, T>;

    template <typename T>
    concept is_json = std::is_constructible_v<ArJsonRequestHandlerFunction, T>;

    template <typename T>
    concept is_getter = std::is_reference_v<T> && std::is_const_v<std::remove_reference_t<T>>;

    template <typename T>
    concept is_setter = std::is_reference_v<T> && !std::is_const_v<std::remove_reference_t<T>>;

    template <typename Request, typename JRequest, typename T>
    concept is_valid_endpoint_arg =
        is_plain<T> || is_json<T> ||
        is_func<Request, T> || is_func<JRequest, T> ||
        is_getter<T> || is_setter<T>;

    /**
     * Class representing a server endpoint;
     * endpoints can be created using a builder pattern:
     * <code>
     *     auto endpoint = Endpoint::at("/path").get([](const Request &r){
     *         r.text() = "Hello World";
     *     });
     * </code>
     */
    class Endpoint
    {
    public:
        class Request;
        class JRequest;

    private:
        /**
         * Interface extending the AsyncWebHandler by getter for the handler's URI string and method composite
         */
        class Handler : public AsyncWebHandler
        {
        public:
            // the server might delete a handler;
            // to prevent usage of dangling pointers, we set a flag when being deleted
            bool* deleted{};

            ~Handler() override { if (deleted) *deleted = true; }

            [[nodiscard]] virtual const String& getUri() const = 0;
            [[nodiscard]] virtual WebRequestMethodComposite getMethod() const = 0;

            bool canHandle(AsyncWebServerRequest* request) const override = 0;
            void handleRequest(AsyncWebServerRequest* request) override = 0;
            void handleUpload(AsyncWebServerRequest* request, const String& filename, size_t index, uint8_t* data,
                              size_t len, bool final) override = 0;
            void handleBody(AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index,
                            size_t total) override = 0;
            [[nodiscard]] bool isRequestHandlerTrivial() const override = 0;
        };


        /**
         * Implements the handler interface by wrapping an existing handler type
         * @tparam T The type of the handler to wrap
         */
        template <typename T>
        class HandlerWrapper final : public Handler
        {
            // anonymous class instance allowing access to common, protected fields of different handler types
            // and used for calling final override of the base class using delegation

            class Static : public T
            {
                friend class HandlerWrapper;
                using T::T;
                using T::_uri;
            };

            class Dynamic : public T
            {
                friend class HandlerWrapper;
                using T::T;
                using T::_uri;
                using T::_method;
                using T::onRequest;
            };

            std::conditional_t<
                std::is_same_v<T, AsyncCallbackWebHandler> || std::is_same_v<T, AsyncCallbackJsonWebHandler>,
                Dynamic, Static
            > _handler;

        public:
            template <typename... Args>
            explicit HandlerWrapper(Args&&... args) : _handler(std::forward<Args>(args)...) {}

            T* operator->() { return &_handler; }

            [[nodiscard]] const String& getUri() const override { return _handler._uri; }

            [[nodiscard]] WebRequestMethodComposite getMethod() const override
            {
                if constexpr (std::is_same_v<decltype(_handler), Dynamic>)
                    return _handler._method;
                else
                    return HTTP_GET;
            }


            bool canHandle(AsyncWebServerRequest* request) const override;
            void handleRequest(AsyncWebServerRequest* request) override;
            void handleUpload(AsyncWebServerRequest* request, const String& filename, size_t index, uint8_t* data,
                              size_t len, bool final) override;
            void handleBody(AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index,
                            size_t total) override;
            [[nodiscard]] bool isRequestHandlerTrivial() const override;
        };


        /**
         * Builder class to easily create endpoints using method chaining and templates
         */
        class Builder
        {
            // endpoint needs access to builder constructor
            friend class Endpoint;
            WebRequestMethodComposite _method{};
            String _uri{};

            // explicitly allow implicit construction in return statements of the builder factory
            template <typename... Args>
            explicit(false) Builder(Args&&... args) : _uri(std::forward<Args>(args)...) {}

        public:
            /**
             * Add a method to the endpoint
             * @param method The method to add to this endpoint
             * @return A reference to this builder
             */
            Builder& withMethod(WebRequestMethod method);

            /**
             * Builds the endpoint with the method and URI set in the builder and the handler arg;
             * the type of the endpoint handler depends on the arg type:
             *  - if the arg is a PlainHandlerFunc or ArRequestHandlerFunction, a AsyncCallbackWebHandler is created
             *  - if the arg is a JsonHandlerFunc or ArJsonRequestHandlerFunction, a AsyncCallbackJsonWebHandler is created
             *  - if the arg is a const reference, a JSON getter is created
             *  - if the arg is a non-const reference, a JSON setter is created
             * @tparam Arg The argument type; must be either the callback type for a
             * AsyncCallbackWebHandler or AsyncCallbackJsonWebHandler or a non-const or const reference to a variable;
             * the variable type must implement a bidirectional JSON converter
             * @param arg The handler argument to construct the endpoint handler from
             * @return The created Endpoint instance
             */
            template <typename Arg> requires is_valid_endpoint_arg<Request, JRequest, Arg>
            Endpoint build(Arg&& arg) const;

            /**
             * Builds an endpoint serving a static file with the URI set in the builder
             * @param path The path in the given filesystem to the file to be served statically
             * @param fs The filesystem the file is stored in
             * @param cache_control ???
             * @return The created Endpoint instance
             */
            Endpoint file(const char* path, FS& fs, const char* cache_control = nullptr) const;

            /**
             * Builds an endpoint serving static files from a directory with the URI set in the builder
             * @param path The path in the given filesystem to the directory to be served statically
             * @param fs The filesystem the directory is stored in
             * @param cache_control ???
             * @return The created Endpoint instance
             */
            Endpoint dir(const char* path, FS& fs, const char* cache_control = nullptr) const;

            auto get(auto&& arg) { return withMethod(HTTP_GET).build(std::forward<decltype(arg)>(arg)); }
            auto post(auto&& arg) { return withMethod(HTTP_POST).build(std::forward<decltype(arg)>(arg)); }
            auto put(auto&& arg) { return withMethod(HTTP_PUT).build(std::forward<decltype(arg)>(arg)); }
            auto del(auto&& arg) { return withMethod(HTTP_DELETE).build(std::forward<decltype(arg)>(arg)); }
        };


        // builder needs access to endpoint constructor
        friend class Builder;
        // Pointer only holds the address to the handler owned by the server
        Handler* _handler;
        // deleted is attached to the handler instance and will be set to true
        // when the handler's destructor is invoked (i.e., the handler is erased from the server's storage);
        // the flag indicates if the handler pointer is still valid or might be dangling
        // and must be used before accessing the pointer values
        bool _deleted{};

        // explicitly allow implicit construction in return statements of the builder build function
        explicit(false) Endpoint(Handler* handler);

    public:
        /**
         * Get the URI this endpoint is located at
         * @return The endpoint's URI string
         */
        [[nodiscard]] auto& getUri() const;

        /**
         * Get the method composite accepted by this endpoint
         * @return This endpoint's method
         */
        [[nodiscard]] auto getMethod() const;

        /**
         * Adds this endpoint's handler to the given server
         * @param server Reference to the server this endpoint should be added to
         */
        void add_to_server(AsyncWebServer& server) const;

        /**
         * Removes this endpoint's handler from the given server
         * @param server Reference to the server this endpoint should be removed from
         */
        void remove_from_server(AsyncWebServer& server);

        /**
         * Creates a new endpoint builder with the URI constructed by the given arguments
         * @tparam Args Argument types
         * @param args Arguments to construct the endpoint URI from
         */
        template <typename... Args>
        static Builder at(Args&&... args) { return {std::forward<Args>(args)...}; }


        /**
         * Helper class wrapping the default AsyncWebServerRequest;
         * will automatically send the response when being deleted
         */
        class Request
        {
            AsyncWebServerRequest* _request;
            std::unique_ptr<String> _text{};
            AsyncJsonResponse* _response{};

        public:
            explicit Request(AsyncWebServerRequest* request) : _request(request) {}

            /**
             * Uses RAII to send the request response, being either the created JSON response or a 204,
             * if not manually send previously
             */
            ~Request();

            /**
             * Gives access to the wrapped request pointer to allow usual request handling
             * @return The wrapped AsyncWebServerRequest pointer
             */
            auto* operator->() const { return _request; }

            /**
             * Creates a new plain text response to be sent by the handler
             * @return A reference to the content of the plain text response
             */
            String& text();

            /**
             * Creates a new JSON response to be sent by the handler
             * @return The underlying JSON variant of the JSON response
             */
            JsonVariant& jr();

            /**
             * Creates a new JSON array response to be sent by the handler
             * @return The underlying JSON array of the JSON response
             */
            JsonArray jrArr();
        };

        /**
         * Helper class wrapping a request with a JSON payload
         */
        class JRequest : public Request
        {
        public:
            JsonVariantConst payload;
            explicit JRequest(AsyncWebServerRequest* request, JsonVariant& json) : Request(request), payload(json) {}
        };


        // delete copy construction
        Endpoint(Endpoint&) = delete;

        // allow move construction
        Endpoint(Endpoint&&) = default;
    };


    using Request = Endpoint::Request;
    using JRequest = Endpoint::JRequest;
}


#include "endpoint.ipp"


#endif //ENDPOINT_HPP
