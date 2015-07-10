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

#include "grpc_inc.h"

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

void server()
{
	std::string server_address("0.0.0.0:50051");
	TestServiceImpl service;

	grpc::SslServerCredentialsOptions sslopt;
	grpc::SslServerCredentialsOptions::PemKeyCertPair pair;

	sslopt.pem_root_certs = loadFile("ca.cert.pem");
	pair.private_key = loadFile("server.key.pem");
	pair.cert_chain = loadFile("server.cert.pem");
	sslopt.pem_key_cert_pairs.emplace_back(std::move(pair));

	ServerBuilder builder;
	builder.AddListeningPort(server_address, grpc::SslServerCredentials(sslopt));
	builder.RegisterService(&service);
	std::unique_ptr<Server> server(builder.BuildAndStart());
	std::cout << "Server listening on " << server_address << std::endl;
	server->Wait();
}

#include <thread>

int main(int argc, char* argv[])
{
	std::cout << "Starting server" << std::endl << std::flush;
	try
	{
		server();
	}
	catch (std::exception& e)
	{
		std::cerr << "ERROR: " << e.what() << std::endl;
	}
}
