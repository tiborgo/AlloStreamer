#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include "AlloShared/Stats.hpp"
#include <boost/bind.hpp>
#include <boost/algorithm/string/join.hpp>

static Stats stats;

class receiver
{
public:
	receiver(boost::asio::io_service& io_service,
		const boost::asio::ip::address& listen_address,
		const boost::asio::ip::address& multicast_address,
		short multicast_port,
		int face)
		: socket_(io_service), face(face)
	{
		// Create the socket so that multiple may be bound to the same address.
		boost::asio::ip::udp::endpoint listen_endpoint(
			listen_address, multicast_port);
		socket_.open(listen_endpoint.protocol());
		socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true));
		socket_.bind(listen_endpoint);

		// Join the multicast group.
		socket_.set_option(
			boost::asio::ip::multicast::join_group(multicast_address));

		socket_.async_receive_from(
			boost::asio::buffer(data_, max_length), sender_endpoint_,
			boost::bind(&receiver::handle_receive_from, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	}

	void handle_receive_from(const boost::system::error_code& error,
		size_t bytes_recvd)
	{
		if (!error)
		{
			stats.addedNALU(face, bytes_recvd, face);

			socket_.async_receive_from(
				boost::asio::buffer(data_, max_length), sender_endpoint_,
				boost::bind(&receiver::handle_receive_from, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
		}
	}

private:
	boost::asio::ip::udp::socket socket_;
	boost::asio::ip::udp::endpoint sender_endpoint_;
	enum { max_length = 65536 };
	char data_[max_length];
	int face;
};

int main(int argc, char* argv[])
{
	try
	{
		if (argc < 4)
		{
			std::cerr << "Usage: receiver <listen_address> <multicast_address> <port>+\n";
			std::cerr << "  For IPv4, try:\n";
			std::cerr << "    receiver 0.0.0.0 239.255.0.1 30001\n";
			std::cerr << "  For IPv6, try:\n";
			std::cerr << "    receiver 0::0 ff31::8000:1234 30001\n";
			return 1;
		}

		std::vector<short> ports;
		for (int i = 3; i < argc; i++)
		{
			ports.push_back(atoi(argv[i]));
		}

		boost::asio::ip::address listen_address    = boost::asio::ip::address::from_string(argv[1]);
		boost::asio::ip::address multicast_address = boost::asio::ip::address::from_string(argv[2]);

		std::stringstream ss;
		std::copy(ports.begin(), ports.end(), std::ostream_iterator<short>(ss, ", "));

		std::cout << "Monitoring packets from " << multicast_address.to_string() <<
			" with port(s) " << ss.str().substr(0, ss.str().length() - 2) <<
			" on interface address " << listen_address.to_string() << std::endl;

		std::vector<receiver*> receivers;
		std::vector<boost::asio::io_service*> io_services;

		for (int i = 0; i < ports.size(); i++)
		{
			io_services.push_back(new boost::asio::io_service());
			receivers.push_back(new receiver(*io_services[i],
				                         boost::asio::ip::address::from_string(argv[1]),
				                         boost::asio::ip::address::from_string(argv[2]),
				                         ports[i],
				                         i));
		}

		stats.autoSummary(boost::chrono::seconds(10));

		std::vector<boost::thread> io_threads;

		for (boost::asio::io_service* io_service : io_services)
		{
			io_threads.push_back(boost::thread(boost::bind(&boost::asio::io_service::run, io_service)));
		}

		for (boost::thread& io_thread : io_threads)
		{
			io_thread.join();
		}
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}