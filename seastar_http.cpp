#include "http/httpd.hh"
#include "http/handlers.hh"
#include "http/function_handlers.hh"
#include "http/file_handler.hh"
#include "http/api_docs.hh"

namespace bpo = boost::program_options;

using namespace seastar;
using namespace httpd;

class handl : public httpd::handler_base {
public:
    virtual future<std::unique_ptr<reply> > handle(const sstring& path,
            std::unique_ptr<request> req, std::unique_ptr<reply> rep) {
        rep->_content = 
        R"(hell0
hell1
hell2
hell3
hell4
hell5
hell6
hell7
hell8
hell9
)";

        rep->add_header("Content-Type", "text/html");
        rep->add_header("Connection", "keep-alive");
        return make_ready_future<std::unique_ptr<reply>>(std::move(rep));
    }
};

void set_routes(routes& r) {
    r.add(operation_type::GET, url("/"), new handl());
    r.add(operation_type::GET, url("/file").remainder("path"),
            new directory_handler("/"));
}

int main(int ac, char** av) {
    app_template app;
    app.add_options()("port", bpo::value<uint16_t>()->default_value(8080),
            "HTTP Server port");
    return app.run_deprecated(ac, av, [&] {
        auto&& config = app.configuration();
        uint16_t port = config["port"].as<uint16_t>();
        auto server = new http_server_control();
        server->start().then([server] {
            return server->set_routes(set_routes);
        }).then([server, port] {
            return server->listen(port);
        }).then([server, port] {
            std::cout << "Thread count: " << seastar::smp::count << std::endl;
            std::cout << "Seastar HTTP server listening on port " << port << " ...\n";
            engine().at_exit([server] {
                return server->stop();
            });
        });

    });
}
