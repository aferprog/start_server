#include <uwebsockets/App.h>
#include <string>
#include <vector>
#include <algorithm>
#include <thread>
#include <iostream>

using namespace std;

struct UserConnection {
	unsigned long user_id;
	string user_name;
};

int main() {
	atomic_ulong next_id = 10;
	vector<thread*> threads(thread::hardware_concurrency());
	std::transform(threads.begin(), threads.end(), threads.begin(), [&next_id](auto *thr) {
		return new thread([&next_id]() {
			uWS::App().ws<UserConnection>("/", {
				.open = [&next_id](uWS::WebSocket<false, true, UserConnection>* ws) {
					// on connection
					UserConnection* data = ws->getUserData();
					data->user_id = next_id++;
					if (ws->subscribe("broadcast"))
						puts("subscibed on broadcast");

					cout << "New client's id is " << data->user_id << endl;
				},
				.message = [](uWS::WebSocket<false, true, UserConnection>* ws, string_view message, uWS::OpCode opCode) {
					// on getting message
					UserConnection* data = ws->getUserData();
					cout << "Input message " << message << " from id " << data->user_id << endl;
					if (message.starts_with("set_name ")) {
						auto name = message.substr(9);
						data->user_name = string(name);
						ws->publish("broadcast", to_string(data->user_id) + " set its name " + data->user_name);
						cout << "notised that " << to_string(data->user_id) + " set its name " + data->user_name << endl;
					}
				}
			}).listen(9999, 
				[](auto *token) {
					if (token)
						puts("server start listening on 9999");
					else {
						puts("!ERROR. NO TOKEN");
						exit(1);
					}
				}).run();
		});
	});

	for_each(begin(threads), end(threads), [](thread* thr) {
		thr->join();
	});
}
