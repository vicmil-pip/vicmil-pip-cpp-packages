#include "util_socketio.hpp"

class CompleteHandler : public vicmil::SocketIOClient::OnDataRecieved
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

class RecievedDataHandler : public vicmil::SocketIOClient::OnDataRecieved
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

class DownloadManager
{
public:
    CompleteHandler complete_handler;
    RecievedDataHandler recieved_data_handler;
    vicmil::SocketIOClient *socket_client = nullptr;

    /*DownloadManager() {}
    DownloadManager(vicmil::SocketIOClient *socket_client_) : socket_client(socket_client_)
    {
        recieved_data_handler = RecievedDataHandler();
        complete_handler = CompleteHandler();
        // socket_client->add_OnDataRecieved("file_chunk", &recieved_data_handler);
        socket_client->add_OnDataRecieved("download_complete", &complete_handler);
    }*/

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

    void add_handlers()
    {
        socket_client->add_OnDataRecieved("download_complete", &complete_handler);
        socket_client->add_OnDataRecieved("file_chunk", &recieved_data_handler);
    }

    void request_file(std::string file_name)
    {
        if (!socket_client)
        {
            Print("Cannot request file without socket client!");
            return;
        }
        vicmil::SocketIOClient::Data request;
        request.write_str("filename", file_name);
        socket_client->emit_data("start_download", request);
    }
};

vicmil::SocketIOClient socket_client = vicmil::SocketIOClient();
DownloadManager download_manager;

bool payload_sent = false;
std::vector<unsigned char> recieved_data = {};

void update()
{
    // Send the file request after connection is successful
    if ((!payload_sent) && socket_client.successfull_connection())
    {
        Print("Send payload");
        vicmil::sleep_s(0.5);
        payload_sent = true;
        download_manager.socket_client = &socket_client;
        download_manager.add_handlers();
        download_manager.request_file("bigfile.txt");
    }
    Print("Connection " << socket_client.successfull_connection());
    Print("Download_complete: " << download_manager.download_complete("bigfile.txt"));
    if (download_manager.download_complete("bigfile.txt") && recieved_data.size() == 0)
    {
        recieved_data = download_manager.get_file_data("bigfile.txt");
        Print("recieved data size: " << recieved_data.size());
    }
    vicmil::sleep_s(1.0);
}

void init()
{
    Print("Make connection");
    socket_client.connect("127.0.0.1", 5050); // Or your server
    Print("exit init");
}

int main()
{
    Print("Main");
    vicmil::set_app_init(init);
    vicmil::set_app_update(update);
    vicmil::app_start();
    return 0;
}