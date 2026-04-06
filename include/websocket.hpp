#pragma once

#include <string>
#include <memory>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include "engine.hpp"
#include <fstream>

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
using tcp = asio::ip::tcp;

class BinanceStream : public std::enable_shared_from_this<BinanceStream> {
private:

    tcp::resolver resolver;
    websocket::stream<beast::ssl_stream<tcp::socket>> ws;
    beast::flat_buffer buffer;
  
    //reference to the math engine
    LOBEngine& math_engine; 

    std::string host = "stream.binance.com";
    std::string port = "9443";
    std::string target = "/ws/btcusdt@depth5@100ms";

    std::ofstream csv_file; 

public:
    //Constructor
    BinanceStream(asio::io_context& ioc, asio::ssl::context& ctx, LOBEngine& engine);

    //Kicks off the asynchronous connection pipeline
    void run();

private:
    //Asynchronous pipeline
    void on_resolve(beast::error_code ec, tcp::resolver::results_type results);
    void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep);
    void on_ssl_handshake(beast::error_code ec);
    void on_ws_handshake(beast::error_code ec);
    void read_loop();
    void on_read(beast::error_code ec, std::size_t bytes_transferred);
};
