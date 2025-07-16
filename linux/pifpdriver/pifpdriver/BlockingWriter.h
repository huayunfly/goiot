#pragma once
#include <boost/asio/serial_port.hpp>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>

namespace goiot
{
    class BlockingWriter
    {
    public:
        // Constructs a blocking writer, pass in an open serial_port and
        // a timeout in milliseconds.
        BlockingWriter(std::shared_ptr<boost::asio::serial_port> port, boost::asio::io_context& io, std::size_t timeout) :
            _port(port), _io(io), _timer(io), _timeout(timeout),
            _write_error(boost::system::errc::make_error_code(boost::system::errc::success)), _bytes_transferred(0)
        {
        }
        BlockingWriter() = delete;
        BlockingWriter(const BlockingWriter&) = delete;
        BlockingWriter& operator=(const BlockingWriter&) = delete;

        // Read until the end_of_cmd, return false if time out.
        bool Write(const std::string& req)
        {
            _io.restart();
            // Async write
            boost::asio::async_write(*_port, boost::asio::buffer(req),
                boost::bind(&BlockingWriter::OnWriteCompleted, this, 
                    boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)
            );

            _timer.expires_from_now(boost::posix_time::milliseconds(_timeout));
            _timer.async_wait(boost::bind(&BlockingWriter::OnTimeout, this, boost::asio::placeholders::error));
            // The run() function blocks() until all works have finished, and no more handlers to be dispatched.
            _io.run();
            return !_write_error;
        }

    private:
        // Called when an async read completes or has been cancelled
        void OnWriteCompleted(const boost::system::error_code& error, std::size_t bytes_transferred)
        {
            _write_error = error;
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
        boost::system::error_code _write_error;
        std::size_t _bytes_transferred;
    };
}
