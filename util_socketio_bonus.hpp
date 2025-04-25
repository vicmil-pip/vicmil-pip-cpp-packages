#pragma once

#include "util_socketio.hpp"

namespace vicmil
{
    class SocketIOLargePayload
    /**
     * Protocoll for sending and recieving large amount of data over a socketio connection,
     *  large files for example
     *
     * 1. The cliend sends a start_download request, with a filename
     * 2. The server responds by sending file_chunk in chunks, one message at a time
     * 3. The server sends a download_complete message, with the filename
     */
    {
    public:
        class _CompleteHandler : public vicmil::SocketIOClient::OnDataRecieved
        {
        public:
            std::map<std::string, bool> download_completed = {};
            void on_data(vicmil::SocketIOClient::Data data) override
            {
                Print("DownloadCompleteHandler data recieved");
                std::string filename = data.read_str("filename");
                download_completed[filename] = true;
            }
        };

        class _RecievedDataHandler : public vicmil::SocketIOClient::OnDataRecieved
        {
        public:
            std::map<std::string, std::map<int, std::vector<unsigned char>>> data_recieved;
            void on_data(vicmil::SocketIOClient::Data data) override
            {
                // Print("RecievedDataHandler data recieved");
                std::string filename = data.read_str("filename");
                int index = data.read_int("index");

                if (data_recieved.count(filename) == 0)
                {
                    data_recieved[filename] = std::map<int, std::vector<unsigned char>>();
                }
                data_recieved[filename][index] = data.read_bytes("chunk");
                // Print(data_recieved[filename].size());
            }
        };
        _CompleteHandler complete_handler;
        _RecievedDataHandler recieved_data_handler;
        vicmil::SocketIOClient *socket_client = nullptr;
        bool handlers_added_to_socket = false;

        SocketIOLargePayload() {}
        SocketIOLargePayload(vicmil::SocketIOClient *socket_client_) : socket_client(socket_client_) {}

        bool download_complete(std::string filename)
        {
            return complete_handler.download_completed.count(filename);
        }
        std::vector<unsigned char> get_file_data(std::string filename)
        {
            if (!download_complete(filename))
            {
                return {};
            }

            std::vector<unsigned char> returned_data = {};

            // Pussle together all of the indecies into one final file
            std::map<int, std::vector<unsigned char>> &file_data = recieved_data_handler.data_recieved[filename];
            for (int i = 0; i < file_data.size(); i++)
            {
                if (file_data.count(i) == 0)
                {
                    Print("file data missing");
                    return {};
                }
                returned_data.insert(returned_data.end(), file_data[i].begin(), file_data[i].end());
            }

            return returned_data;
        }

        void _add_handlers()
        {
            if (!handlers_added_to_socket)
            {
                socket_client->add_OnDataRecieved("download_complete", &complete_handler);
                socket_client->add_OnDataRecieved("file_chunk", &recieved_data_handler);
            }
        }

        void request_file(std::string file_name)
        {
            if (!socket_client)
            {
                Print("Cannot request file without socket client!");
                return;
            }
            if (!socket_client->successfull_connection())
            {
                Print("Cannot request file without socket being successfully connected!");
                return;
            }
            _add_handlers(); // Ensure handlers for recieving data are loaded
            vicmil::SocketIOClient::Data request;
            request.write_str("filename", file_name);
            socket_client->emit_data("start_download", request);
        }
    };
}