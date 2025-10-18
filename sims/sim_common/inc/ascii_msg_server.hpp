/* Copyright (C) 2015 - 2016 National Aeronautics and Space Administration. All Foreign Rights are Reserved to the U.S. Government.

   This software is provided "as is" without any warranty of any, kind either express, implied, or statutory, including, but not
   limited to, any warranty that the software will conform to, specifications any implied warranties of merchantability, fitness
   for a particular purpose, and freedom from infringement, and any warranty that the documentation will conform to the program, or
   any warranty that the software will be error free.

   In no event shall NASA be liable for any damages, including, but not limited to direct, indirect, special or consequential damages,
   arising out of, resulting from, or in any way connected with the software or its documentation.  Whether or not based upon warranty,
   contract, tort or otherwise, and whether or not loss was sustained from, or arose out of the results of, or use of, the software,
   documentation or services provided hereunder

   ITC Team
   NASA IV&V
   ivv-itc@lists.nasa.gov
*/

#ifndef NOS3_ASCIIMSGSERVER_HPP
#define NOS3_ASCIIMSGSERVER_HPP

#include <queue>
#include <unistd.h>

//#include <boost/shared_ptr.hpp>

namespace Nos3
{
    /** \brief Class for receiving ASCII messages on a TCP/IP connection.
     *
     *  \details Messages boundaries are determined based on the carriage return
     *  character.  Clients call listen_for_data and evaluate the return value
     *  to determine whether valid messages were received by the server.  Messages
     *  can be retrieved by calling get_next_message function.  This function
     *  can be called as many times as needed to drain the receive message
     *  queue.
     */
    class AsciiMsgServer
    {
    public:
        /// @name Constructors / destructors
        //@{
        AsciiMsgServer(const uint16_t port);
        ~AsciiMsgServer();

        /** \brief Initialize the server and start listening for connections
         *
         *  \returns true if the server initializes succesfully, otherwise false.
         */

        bool init();

        /** \brief Listen for messages from client connections
         *
         *  \details Uses a select call to block indefinately until one of the
         *  falling occurs.
         *    1) Receipt of a new client connection
         *    2) Receipt of data from an existing client connection
         *    3) A signal handler interrupts the select call
         *
         *  \returns true when the server has read one or more new messages from
         *  a client connection.
         */
        bool listen_for_data();

        /** \brief Get the next message received by the server
         *
         *  \returns true if the msg parameter was populated otherwise false.
         *  A value of false indicates the server's receive message queue is 
         *  empty.
         */
        bool get_next_message(std::string &msg)
        {
            bool queue_has_msg = ! _rcv_msg_queue.empty();

            if (queue_has_msg)
            {
                msg = _rcv_msg_queue.front();
                _rcv_msg_queue.pop();
            }

            return queue_has_msg;
        }

    private:

        // Helper struct to handle client connection
        struct ClientConnection
        {
            int fd;
            char rcv_buff[256];
            char *buff_tail;
        };

        // Private helper methods
        bool open_socket();
        bool wait_for_data (fd_set &read_fds);
        void read_socket_data(ClientConnection &client_conn);
        void parse_message(ClientConnection &client_conn);
        void reset_buffer(ClientConnection &client_conn);

        // Private data

        // Connection data
        uint16_t _port;

        // File descriptor for the connection accept socket
        int _socket_fd;

        // A queue used to store data received via client connections
        std::queue<std::string> _rcv_msg_queue;

        // Helps track multiple client connection
        static const int MAX_CLIENT_CONNECTIONS = 10;
        ClientConnection _client_conn[MAX_CLIENT_CONNECTIONS];
    };
}

#endif
