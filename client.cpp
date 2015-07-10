#include <grpc/grpc.h>
#include <grpc++/channel_arguments.h>
#include <grpc++/channel_interface.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/credentials.h>
#include <grpc++/status.h>

#include <iostream>
#include <sstream>

#include "grpc_inc.h"

using grpc::ChannelArguments;
using grpc::ChannelInterface;
using grpc::ClientContext;
using grpc::Status;
using grpc::StatusCode;

using alfa::TestService;
using alfa::LoginInfo;
using alfa::PingInfo;
using alfa::User;

// ================================================================

class TestServiceClient {
public:
	TestServiceClient(std::shared_ptr<ChannelInterface> channel)
		: _stub(TestService::NewStub(channel)) {}

	std::string Login(std::string username, std::string password) {
		LoginInfo login;
		login.set_username(username);
		login.set_password(password);
		User reply;
		ClientContext ctx;

		Status status = _stub->Login(&ctx, login, &reply);
		if (status.ok()) {
			return reply.id();
		}
		else
		{
			std::ostringstream ss;
			ss << "Login call failed. Desc = '" << status.error_message() << "' code = " << status.error_code();
			throw std::runtime_error(ss.str());
		}
	}

private:
	std::unique_ptr<TestService::Stub> _stub;
};

const uint32_t REQUESTS_PER_THREAD = 1000;
const uint32_t NUM_THREADS = 100;

void client(TestServiceClient* clientObj, std::string username)
{
	int i = 0;
	const int max = REQUESTS_PER_THREAD;
	int req = 0;
	while (i++ < max)
	{
		try
		{
			auto id = clientObj->Login(username, "secret");
			req++;
		}
		catch (std::exception& e)
		{
			std::cout << "EXCEPTION: " << e.what() << std::endl;
		}
		//std::cout << "login got id " << id << std::endl;
	}
	//std::cout << "done " << req << " requests" << std::endl;
}

#include <thread>
#include <chrono>
#include <fstream>

std::string loadFile(const std::string& filename)
{
	std::ifstream file(filename);
	if (file.is_open() == false)
		throw std::runtime_error(std::string("file not found: ") + filename);

	std::ostringstream ss;
	std::string line;
	while (std::getline(file, line))
		ss << line << std::endl;

	if (file.bad())
		throw std::runtime_error(std::string("something went wrong while reading file: ") + filename);

	return std::move(ss.str());
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cerr << "server:port required" << std::endl;
		return -1;
	}

	grpc::SslCredentialsOptions ssl_opts;
	ssl_opts.pem_root_certs = loadFile("final.cert.pem");
	std::shared_ptr<grpc::Credentials> creds = grpc::SslCredentials(ssl_opts);

	auto conn = grpc::CreateChannel(argv[1], creds, ChannelArguments());
	{
		TestServiceClient clientObj(conn);

		std::vector<std::unique_ptr<std::thread>> _threads;
		for (auto i = 0; i < NUM_THREADS; i++)
			_threads.emplace_back(std::unique_ptr<std::thread>(new std::thread(client, &clientObj, "some")));

		while (_threads.size() > 0)
		{
			auto thread = std::move(_threads.back());
			_threads.pop_back();
			thread->join();
		}

		std::cout << "Finished " << (REQUESTS_PER_THREAD * NUM_THREADS) << " requests" << std::endl;
	}

	//std::this_thread::sleep_for(std::chrono::seconds(10));
	conn = nullptr;

	std::cout << "Disconnected" << std::endl;

	exit(0);
}
