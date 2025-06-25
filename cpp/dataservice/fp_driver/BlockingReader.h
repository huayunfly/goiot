#pragma once
#include <boost/asio/serial_port.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

namespace goiot
{
    class BlockingReader
    {
    public:
        // Constructs a blocking reader, pass in an open serial_port and
        // a timeout in milliseconds.
        BlockingReader(std::shared_ptr<boost::asio::serial_port> port, boost::asio::io_context& io, std::size_t timeout) :
            _port(port), _io(io), _timer(io), _timeout(timeout), 
            _read_error(boost::system::errc::make_error_code(boost::system::errc::success)), _bytes_transferred(0)
        {
        }
        BlockingReader() = delete;
        BlockingReader(const BlockingReader&) = delete;
        BlockingReader& operator=(const BlockingReader&) = delete;

        // Read until the end_of_cmd, return false if time out.
        bool ReadUntil(const std::string& end_of_cmd, std::string& reply)
        {
            _io.reset();
            // Async read
            boost::system::error_code ec;
            boost::asio::streambuf input_buffer;
            boost::asio::async_read_until(*_port, input_buffer, end_of_cmd,
                std::bind(&BlockingReader::OnReadCompleted, this, std::placeholders::_1, std::placeholders::_2));

            _timer.expires_from_now(boost::posix_time::milliseconds(_timeout));
            _timer.async_wait(boost::bind(&BlockingReader::OnTimeout, this, boost::asio::placeholders::error));
            // The run() function blocks() until all works have finished, and no more handlers to be dispatched.
            _io.run();
            if (!_read_error)
            {
                std::string str((std::istreambuf_iterator<char>(&input_buffer)), std::istreambuf_iterator<char>());
                input_buffer.consume(_bytes_transferred);
                reply = str;
            }
            return !_read_error;
        }

    private:
        // Called when an async read completes or has been cancelled
        void OnReadCompleted(const boost::system::error_code& error, std::size_t bytes_transferred)
        {
            _read_error = error;
            // Read has finished, so cancel the timer.
            _timer.cancel();
            _bytes_transferred = bytes_transferred;
        }

        // Called when the timer's deadline expires.
        void OnTimeout(const boost::system::error_code& error) {
            // Was the timeout was cancelled?
            if (error)
            {
                return;
            }
            _port->cancel();
        }

    private:
        std::shared_ptr<boost::asio::serial_port> _port;
        boost::asio::io_context& _io;
        boost::asio::deadline_timer _timer;
        std::size_t _timeout;
        boost::system::error_code _read_error;
        std::size_t _bytes_transferred;
    };
}