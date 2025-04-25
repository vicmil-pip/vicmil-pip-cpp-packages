#include "util_socketio_bonus.hpp"

vicmil::SocketIOClient socket_client = vicmil::SocketIOClient();
vicmil::SocketIOLargePayload download_manager;

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
    socket_client.connect("127.0.0.1", 5050);
    download_manager = vicmil::SocketIOLargePayload(&socket_client);
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