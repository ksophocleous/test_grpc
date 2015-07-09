#include <grpc/grpc.h>
#include <grpc++/channel_arguments.h>
#include <grpc++/channel_interface.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/credentials.h>
#include <grpc++/status.h>

#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <grpc++/server_credentials.h>
#include <grpc++/status.h>

#include <iostream>
#include <sstream>

#pragma warning(push)
#pragma warning(disable:4244)
#include "test.grpc.pb.h"
#pragma warning(pop)

using grpc::ChannelArguments;
using grpc::ChannelInterface;
using grpc::ClientContext;
using grpc::Status;
using grpc::StatusCode;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;

using alfa::TestService;
using alfa::LoginInfo;
using alfa::PingInfo;
using alfa::User;

// ================================================================

class TestServiceImpl final : public TestService::Service {
	Status Login(ServerContext* ctx, const LoginInfo* loginInfo, User* output) override {
		if (loginInfo->username().compare("ksophocleous") == 0)
		{
			output->set_id("111111222222");
			output->set_name("kostas");
		}
		else
		{
			output->set_id("12");
			output->set_name("kostas");
		}
		return Status::OK;
	}

	Status PingMe(ServerContext* context, const User* request, PingInfo* response) override {
		if (request->id().compare("12983479238947"))
		{
			response->set_time(1000);
		}
		else
			response->set_time(0);

		return Status(StatusCode::PERMISSION_DENIED, "whaaat");
	}
};

void server() {
	std::string server_address("0.0.0.0:50051");
	TestServiceImpl service;

	ServerBuilder builder;
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	builder.RegisterService(&service);
	std::unique_ptr<Server> server(builder.BuildAndStart());
	std::cout << "Server listening on " << server_address << std::endl;
	server->Wait();
}

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


void client(TestServiceClient* clientObj, std::string username)
{
	try {
		int i = 0;
		const int max = 100000;
		while (i++ < max) {
			auto id = clientObj->Login(username, "secret");
			//std::cout << "login success... got id " << id << std::endl;
		}
	}
	catch (std::exception& e)
	{
		std::cout << "EXCEPTION: " << e.what() << std::endl;
	}

	std::cout << "done exiting" << std::endl;
}

#include <thread>

int main(int argc, char* argv[]) {
	if (argc > 1 && std::string(argv[1]).compare("--server") == 0)
	{
		std::cout << "Starting server" << std::endl << std::flush;
		server();
		//std::thread serverThread(server);
		//serverThread.join();
	}
	else
	{
		//std::thread clientThread(client);
		//clientThread.join();
		TestServiceClient clientObj(grpc::CreateChannel("localhost:50051", grpc::InsecureCredentials(), ChannelArguments()));
		std::thread clientThread1(client, &clientObj, "ksophocleous");
		std::thread clientThread2(client, &clientObj, "hi there");
		std::thread clientThread3(client, &clientObj, "hi thereaaa");
		std::thread clientThread4(client, &clientObj, "hi thereaaa");
		std::thread clientThread5(client, &clientObj, "hi thereaaa");
		std::thread clientThread6(client, &clientObj, "hi thereaaa");

		clientThread1.join();
		clientThread2.join();
		clientThread3.join();
		clientThread4.join();
		clientThread5.join();
		clientThread6.join();
	}
}
