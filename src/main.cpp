#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "websocket.hpp"
#include "engine.hpp"

namespace asio = boost::asio;

int main(int argc, char* argv[]) {
    std::cout << "[SYSTEM] Initializing Binance LOB Capturing...\n";
    std::cout << "[SYSTEM] Target Pair: BTC/USDT\n";

    try {
        asio::io_context ioc;
        asio::ssl::context ctx{asio::ssl::context::tlsv12_client};
        ctx.set_default_verify_paths();

        LOBEngine math_engine;
        std::make_shared<BinanceStream>(ioc, ctx, math_engine)->run();

        ioc.run();

    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Exception caught in main: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
