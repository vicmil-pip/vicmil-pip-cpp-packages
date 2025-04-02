#pragma once
/*
This file contains some utilities for handling socketio
The idea is to be cross platform, and also to support the web
*/

#include "util_js.hpp"

#ifndef __EMSCRIPTEN__
#include "socket.io-client/src/sio_client.h"
#else
#include <emscripten/emscripten.h>
#include <emscripten/bind.h>
#endif

namespace vicmil
{

#ifndef __EMSCRIPTEN__
    class SocketIOClient
    {
    private:
        sio::client client;

    public:
        bool _successfull_connection = false;
        bool _failed_connection = false;
        class Data
        {
        public:
            sio::message::ptr _data;
            Data()
            {
                _data = sio::object_message::create();
            }
            std::string read_str(std::string key)
            {
                std::string str = _data->get_map()[key]->get_string();
                return str;
            }
            void write_str(std::string key, std::string str)
            {
                _data->get_map()[key] = sio::string_message::create(str);
            }
            std::vector<unsigned char> read_bytes(std::string key)
            {
                std::string base64_bytes = _data->get_map()[key]->get_string();
                std::vector<unsigned char> raw_bytes_vec = base64_decode(base64_bytes);
                return raw_bytes_vec;
            }
            void write_bytes(std::string key, std::vector<unsigned char> bytes_data)
            {
                std::string base64_data = to_base64(bytes_data);
                _data->get_map()[key] = sio::string_message::create(base64_data);
            }
            void write_array(std::string key, std::vector<int> array_vals)
            {
                sio::message::ptr xy_array = sio::array_message::create();
                for (int i = 0; i < array_vals.size(); i++)
                {
                    xy_array->get_vector().push_back(sio::int_message::create(array_vals[i]));
                }
                _data->get_map()[key] = xy_array;
            }
            int size_in_bytes() const
            {
                size_t total_size = 0;

                auto &message_map = _data->get_map();
                for (const auto &pair : message_map)
                {
                    // Add the size of the key (assuming key is a string)
                    total_size += pair.first.size();

                    // Add the size of the value (if it's a string)
                    if (pair.second->get_flag() == pair.second->flag_string)
                    {
                        total_size += pair.second->get_string().size();
                    }
                    // Handle other types (e.g., number, object) if necessary
                }
                return total_size;
            }
        };

        // Interface class for handling when data is recieved
        class OnDataRecieved
        {
        public:
            virtual void on_data(Data) = 0;
        };

        bool successfull_connection()
        {
            return _successfull_connection;
        }
        bool failed_connection()
        {
            return _failed_connection;
        }

        SocketIOClient() {}
        // When recieving an event with the event name, invoke the function
        void add_OnDataRecieved(std::string event_name, OnDataRecieved *on_data_recieved)
        {
            auto lambda = [this, on_data_recieved](sio::event &event) { // Capture args by value
                SocketIOClient::Data data;
                data._data = event.get_message();
                on_data_recieved->on_data(data);
            };
            client.socket()->on(event_name, lambda);
        }
        void connect(const std::string &uri)
        {
            client.connect(uri);
            auto lambda = [this]() { // Capture args by value
                this->_successfull_connection = true;
            };
            client.set_open_listener(lambda);
        }
        void close()
        {
            client.sync_close();
            client.clear_con_listeners();
        }
        // Send more complicated data in a map: TODO
        void emit_data(const std::string &event_name, const SocketIOClient::Data &data)
        {
            // Note! There is a hard limit of sending 1_000_000 bytes, split it up if you need to send more
            Print("data size: " << std::to_string(data.size_in_bytes()));
            client.socket()->emit(event_name, data._data);
        }
    };

#else

    class SocketIOClient
    {
    private:
        bool connection_attempt = false;
        std::string socketID = "";

    public:
        bool _successfull_connection = false;
        bool _failed_connection = false;
        class Data : public JSData
        {
        };

        // Interface class for handling when data is recieved
        class OnDataRecieved
        {
        public:
            virtual void on_data(Data) = 0;
        };

        class _OnDataRecieved : public JsFunc
        {
        public:
            OnDataRecieved *to_invoke; // Invoke class whenever data is recived
            void on_data(emscripten::val raw_data)
            {
                Data data;
                data._payload = raw_data;
                to_invoke->on_data(data);
            }
        };
        class _OnConnection : public JsFunc
        {
        public:
            SocketIOClient *socket_io;
            void on_data(emscripten::val)
            {
                socket_io->_successfull_connection = true;
                Print("_OnConnection");

                // Setup all the events for handling incomming data
                // TODO
            }
        };
        bool successfull_connection()
        {
            return _successfull_connection;
        }
        bool failed_connection()
        {
            return _failed_connection;
        }
        _OnConnection _on_connection = _OnConnection();
        std::map<std::string, _OnDataRecieved> _js_on_data = {};
        void add_OnDataRecieved(std::string event_name, OnDataRecieved *on_data_recieved)
        {
            if (!connection_attempt)
            {
                ThrowError("add_OnDataRecieved invoked before connection attempt");
            }
            _js_on_data[event_name] = _OnDataRecieved();
            _js_on_data[event_name].to_invoke = on_data_recieved;
            vicmil::JsFuncManager::add_js_func(&_js_on_data[event_name]); // Expose class instance to javascript

            // OnConnection invoked when a connection has been made
            EM_ASM({
            // Listen for messages from the server
            window.socket.on(UTF8ToString($0), function(data) {
                console.log('Received data from socket');
                Module.JsFuncManager(data, UTF8ToString($1));
            }); }, event_name.c_str(), _js_on_data[event_name].key.c_str());
        }
        void emit_data(std::string event_name, Data data)
        {
            if (!_successfull_connection)
            {
                ThrowError("send_data invoked without successfull socket connection");
            }
            emscripten::val socket = emscripten::val::global("socket"); // Get the JavaScript 'socket' object
            socket.call<void>("emit", emscripten::val(event_name.c_str()), data._payload);
        }
        void connect(const std::string &uri)
        {
            if (connection_attempt)
            {
                ThrowError("socket connection_attempt has already been made!");
            }
            _on_connection.socket_io = this;
            vicmil::JsFuncManager::add_js_func(&_on_connection); // Expose class instance to javascript
            EM_ASM({
            // Load socket.io if not already loaded
            if (typeof io === 'undefined') {
                var script = document.createElement('script');
                script.src = 'https://cdn.socket.io/4.5.4/socket.io.min.js';
                script.onload = function() {
                    console.log("Socket.io loaded.");
                    startSocket();
                };
                document.head.appendChild(script);
            } else {
                startSocket();
            }

            function startSocket() {
                // Connect to the Socket.io server
                window.socket = io(UTF8ToString($0)); // Change to your server

                window.socket.on('connect', function() {
                    console.log('Connected to server');
                    var data = {"connection": "success"};
                    Module.JsFuncManager(data, UTF8ToString($1));
                });
            } }, uri.c_str(), _on_connection.key.c_str());
            connection_attempt = true; // Connection attempt made
        }
        void close()
        { // Close the connection to the server
          // TODO
        }
    };

#endif

}
