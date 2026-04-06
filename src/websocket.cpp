#include "websocket.hpp"
#include <iostream>
#include <rapidjson/document.h>
#include <chrono>

BinanceStream::BinanceStream(asio::io_context& ioc, asio::ssl::context& ctx, LOBEngine& engine)
    : resolver(ioc), ws(ioc, ctx), math_engine(engine) {
    
    csv_file.open("data/lob_dataset.csv", std::ios::app);
    if (csv_file.is_open()) {
        csv_file << "timestamp,mid_price,micro_price,depth_micro_price,ofi\n";
    }
}

void BinanceStream::run() {
    std::cout << "[NETWORK] Resolving Binance IP address for " << host << "\n";
    
    resolver.async_resolve(host, port,
        beast::bind_front_handler(&BinanceStream::on_resolve, shared_from_this()));
}

void BinanceStream::on_resolve(beast::error_code ec, tcp::resolver::results_type results) {
    if (ec) { std::cerr << "[ERROR] Resolve failed: " << ec.message() << "\n"; return; }
    
    std::cout << "[NETWORK] IP resolved, Connecting TCP...\n";
    
    boost::asio::async_connect(
        beast::get_lowest_layer(ws), 
        results,
        beast::bind_front_handler(&BinanceStream::on_connect, shared_from_this())
    );
}

void BinanceStream::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep) {
    if (ec) { std::cerr << "[ERROR] Connect failed: " << ec.message() << "\n"; return; }

    std::cout << "[NETWORK] TCP Connected, Performing SSL Handshake...\n";

    if (!SSL_set_tlsext_host_name(ws.next_layer().native_handle(), host.c_str())) {
        std::cerr << "[ERROR] SNI Setup failed.\n";
        return;
    }

    ws.next_layer().async_handshake(asio::ssl::stream_base::client,
        beast::bind_front_handler(&BinanceStream::on_ssl_handshake, shared_from_this()));
}

void BinanceStream::on_ssl_handshake(beast::error_code ec) {
    if (ec) { std::cerr << "[ERROR] SSL Handshake failed: " << ec.message() << "\n"; return; }

    std::cout << "[NETWORK] SSL Secured, Upgrading to WebSocket...\n";

    ws.async_handshake(host, target,
        beast::bind_front_handler(&BinanceStream::on_ws_handshake, shared_from_this()));
}

void BinanceStream::on_ws_handshake(beast::error_code ec) {
    if (ec) { std::cerr << "[ERROR] WebSocket Handshake failed: " << ec.message() << "\n"; return; }

    std::cout << "[NETWORK] Connected to Binance Stream: " << target << "\n";

    //infinite read loop
    read_loop();
}

void BinanceStream::read_loop() {
    //Clear memory buffer before reading new data
    buffer.consume(buffer.size());

    ws.async_read(buffer,
        beast::bind_front_handler(&BinanceStream::on_read, shared_from_this()));
}

void BinanceStream::on_read(beast::error_code ec, std::size_t bytes_transferred) {
    if (ec) { std::cerr << "[ERROR] Read failed: " << ec.message() << "\n"; return; }

    std::string raw_json = beast::buffers_to_string(buffer.data());

    rapidjson::Document doc;
    doc.Parse(raw_json.c_str());

    if (!doc.HasParseError() && doc.HasMember("bids") && doc.HasMember("asks")) {
        OrderBookSnapshot snapshot;
        auto& bids = doc["bids"];
        auto& asks = doc["asks"];

        //Loop through all 5 levels
        int levels = std::min(bids.Size(), asks.Size());
        for (int i = 0; i < levels; ++i) {
            snapshot.bids.push_back({std::stod(bids[i][0].GetString()), std::stod(bids[i][1].GetString())});
            snapshot.asks.push_back({std::stod(asks[i][0].GetString()), std::stod(asks[i][1].GetString())});
        }

        if (!snapshot.bids.empty() && !snapshot.asks.empty()) {
            double mid_price = (snapshot.bids[0].price + snapshot.asks[0].price) / 2.0;
            double micro_price = math_engine.calculateMicroPrice(snapshot);
            double depth_micro = math_engine.calculateDepthMicroPrice(snapshot, 0.5); //Alpha = 0.5
            double ofi = math_engine.calculateOFI(snapshot);

            //Get local timestamp in milliseconds
            auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count();

            std::cout << "[TICK] Depth-Price: " << depth_micro << " | OFI: " << ofi << "\n";

            if (csv_file.is_open()) {
                csv_file << now << "," << mid_price << "," << micro_price << "," 
                         << depth_micro << "," << ofi << "\n";
            }
        }
    }

    read_loop();
}
