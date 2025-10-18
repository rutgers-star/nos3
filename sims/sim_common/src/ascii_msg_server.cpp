/* Copyright (C) 2015 - 2017 National Aeronautics and Space Administration. All Foreign Rights are Reserved to the U.S. Government.

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

#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <netdb.h>

#include <ItcLogger/Logger.hpp>

#include <ascii_msg_server.hpp>

namespace Nos3
{
    extern ItcLogger::Logger *sim_logger;

    /*************************************************************************
     * Constructors / Destructors
     *************************************************************************/

    AsciiMsgServer::AsciiMsgServer(const uint16_t port)
        : _port(port),
          _socket_fd(-1)
    {
        for (int i=0 ; i<MAX_CLIENT_CONNECTIONS ; ++i)
        {
            _client_conn[i].fd = -1;
            reset_buffer(_client_conn[i]);
        }
    }

    AsciiMsgServer::~AsciiMsgServer(void)
    {
        sim_logger->info("Ascii Msg Server: closing");

        // Stop incoming connections
        close(_socket_fd);

        // Close all active client connections
        for (int i=0 ; i<MAX_CLIENT_CONNECTIONS ; ++i)
        {
            if (_client_conn[i].fd > 0)
                close(_client_conn[i].fd);
        }
    }

    /*************************************************************************
     * Public methods
     *************************************************************************/

     bool AsciiMsgServer::init()
     {
         bool ok_to_continue = true;

         /**
           * Create the client connection listener socket
         **/
         _socket_fd = socket(AF_INET, SOCK_STREAM, 0);

         if (_socket_fd == -1)
         {
             sim_logger->error("Ascii Msg Server socket creation failed:  %s", strerror(errno));
             ok_to_continue = false;
         }
         else
         {
             sim_logger->debug("ASCII Msg Server socket successfully created");
         }

         /**
           * Bind the listener socket to the specified address
         **/
         if (ok_to_continue)
         {
             struct sockaddr_in server;
             bzero(&server, sizeof(server));

             server.sin_family = AF_INET;
             server.sin_addr.s_addr = htonl(INADDR_ANY);
             server.sin_port = htons(_port);

             int bind_result = bind(_socket_fd, (sockaddr*)&server, sizeof(server));

             if (bind_result != 0)
             {
                 sim_logger->error("socket bind failed: %s", strerror(errno));
                 ok_to_continue = false;
             }
             else if (ok_to_continue)
             {
                  sim_logger->debug("Socket successfully bound");
             }
         }

         /**
           * Start listening for incoming connection requests
         **/
         if (ok_to_continue)
         {
             int listen_result = listen(_socket_fd, 5);

             if (listen_result != 0)
             {
                 sim_logger->error("listen failed:  %s", strerror(errno));
                 ok_to_continue = false;
             }
             else if (ok_to_continue)
             {
                 sim_logger->info("ASCII Msg Server listening on port %d", _port);
             }
         }

         return ok_to_continue;
     }

     bool AsciiMsgServer::listen_for_data()
     {
         int prev_msg_queue_size = _rcv_msg_queue.size();

         fd_set read_fds;

         /**
           * Wait for any data to become available on both the main socket and all
           * active client connections.
         **/
         bool select_success = wait_for_data(read_fds);

         /**
           * Service the main socket to handle any new client connection requests
         **/
         bool connection_request = FD_ISSET(_socket_fd, &read_fds);

         if (select_success && connection_request)
         {
             sockaddr_in client;
             socklen_t client_addr_len = sizeof(client);

             int client_fd = accept(_socket_fd, (sockaddr*)&client, &client_addr_len);

             // Ignore EAGAIN.  Since we're using select, we only get EAGAIN when
             // the application is shutting down.
             if (client_fd < 0 && errno != EAGAIN)
             {
                 sim_logger->error("Ascii Msg Server socket accept failed: %s: %d", strerror(errno), errno);
             }
             else if (client_fd > 0)
             {
                 bool client_accepted = false;

                 for (uint32_t i=0; i<MAX_CLIENT_CONNECTIONS ; ++i)
                 {
                     if (_client_conn[i].fd == -1)
                     {
                         _client_conn[i].fd = client_fd;
                         reset_buffer(_client_conn[i]);

                         client_accepted = true;

                         sim_logger->debug("Added new client connection fd=%d", client_fd);

                         break;
                     }
                 }

                 if (! client_accepted)
                 {
                     sim_logger->error("Max number of client connections exceeded (%d).  "
                        "Rejecting connection request.", MAX_CLIENT_CONNECTIONS);
                 }
             }
         }

         /**
           * Service any data available on client connection sockets
         **/
         if (select_success)
         {
             for (uint32_t i=0; i<MAX_CLIENT_CONNECTIONS ; ++i)
             {
                 // Only want active connections
                 if (_client_conn[i].fd < 0)
                    continue;

                 bool data_available = FD_ISSET(_client_conn[i].fd, &read_fds);

                 if (data_available)
                 {
                     read_socket_data(_client_conn[i]);
                 }
             }
         }

         /**
           * If the message queue size changed since we read it at the begning
           * of the function, then a new message was succesfully received on
           * one of the client connections.
         **/
         return (prev_msg_queue_size != (int) _rcv_msg_queue.size());
     }

    /*************************************************************************
    * Private helper methods
    *************************************************************************/

    bool AsciiMsgServer::wait_for_data (fd_set &read_fds)
    {
        /**
          * Initialize the structs needed for select.  select also requires the
          * max file descriptor.
          *
          * We add the fild descriptor for the connection socket and client
          * sockets.
        **/
        int max_fd = _socket_fd;

        FD_ZERO(&read_fds);
        FD_SET(_socket_fd, &read_fds);

        for (uint32_t i = 0 ; i < MAX_CLIENT_CONNECTIONS ; ++i)
        {
            int fd = _client_conn[i].fd;

            // Only process active connections
            if (fd < 0)
                continue;

            FD_SET(fd, &read_fds);

            if(fd > max_fd)
                max_fd = fd;
        }

        /**
          * Wait for read data available on connection request socket and client
          * sockets.  No timeout needed since a signal will interrupt this call.
        **/
        int result = select(max_fd+1, &read_fds, NULL, NULL, NULL);

        if (result < 0)
        {
            // Log error if select returned error but not from a signal.
            if (errno != EINTR)
            {
                sim_logger->error("Ascii Msg Server 'select' error: %d/%d %s\n",
                    result, errno, strerror(errno));
            }
            else // we're shutting down if we get EINTR
            {
                sim_logger->info("Ascii Msg Server: 'select' EINTR, prepare for shutdown");
            }
        }

        // specify if select was succesful so we can evaluate the fd_set
        return result > -1;
     }

     void AsciiMsgServer::read_socket_data(ClientConnection &client_conn)
     {
        int num_bytes = 0;

        /**
          * Calculate number of bytes avaialable in the client connection receive
          * buffer.  If the buffer is empty then the tail is pointing to the first
          * element and all bytes are avaialable
        **/
        int buffer_size = client_conn.rcv_buff + sizeof(client_conn.rcv_buff) - client_conn.buff_tail;

        sim_logger->debug("Ascii Msg Server: read_socket_data: client=%d status=%d", client_conn.fd, buffer_size);

        if (buffer_size > 0)
        {
            num_bytes = read(client_conn.fd, client_conn.buff_tail, buffer_size);

            if (num_bytes > 0) // Got data, try to parse messages
            {
                client_conn.buff_tail += num_bytes;
                parse_message(client_conn);
            }
            else if (num_bytes < 0) // Error occured, reset the receive buffer
            {
                sim_logger->error("Ascii Msg Server: socket 'read' error:  %s\n", strerror(errno));

                // Reset buffer pointers
                reset_buffer(client_conn);
            }
            else // Client disconnect
            {
                sim_logger->debug("Ascii Msg Server: Client disconnect for fd=%d", client_conn.fd);

                close(client_conn.fd);
                client_conn.fd = -1;
            }
        }
        else
        {
            sim_logger->error("Ascii Msg Server: receive buffer full for client fd %d.  Dumping buffer...", client_conn.fd);

            reset_buffer(client_conn);
        }
     }

     void AsciiMsgServer::parse_message(ClientConnection &client_conn)
     {
         char* curr_char = client_conn.rcv_buff;

         // Keeps track of the character that starts the next incomplete message
         // in the receive buffer.  When a message is found, this pointer is updated
         // to one character past new line character of the found message.
         char *next_msg_start = client_conn.rcv_buff;

         /**
           * Search for the new line character.  This is our message delimiter.
           * If found, we copy the buffer from the last msg -> tail to a string
           * and add it to the mssage queue.
           *
           * The last_msg pointer is then moved to 1 character past the newline
           * character in the array.  This allows us to keep searching for more
           * messsages in case multiple messages were returned in a single read call.
         **/
         while (curr_char < client_conn.buff_tail)
         {
             if (*curr_char == '\n')
             {
                 // Don't add 1 here because we don't want to include the new line
                 // character as part of the message.
                 int message_size = curr_char - next_msg_start;

                 if (message_size > 0)
                 {
                     std::string msg(next_msg_start, message_size);

                     _rcv_msg_queue.push(msg);

                     sim_logger->debug("New msg %d/%ld", message_size, msg.size());
                 }

                 // New line character is the end of this message.  So next
                 // character is the start of the next message.
                 next_msg_start = curr_char + 1;
             }

             curr_char++;
         }

         sim_logger->debug("Ascii Msg Server: ---Buffer status: base=%p next_msg=%p tail=%p",
            client_conn.rcv_buff, next_msg_start, client_conn.buff_tail);

         /**
           * If there are any remaining bytes in the buffer, then we didn't find
           * a new line character for them.  Move the reamining bytes to the begining
           * of the buffer and set the tail to one past the last remaining byte.
           * This will allow subsequent reads to append their data to the remaining
           * bytes.
         **/
         int remaining_bytes = 0;

         if (next_msg_start != client_conn.buff_tail)
         {
             remaining_bytes = client_conn.buff_tail - next_msg_start;

             sim_logger->debug("Ascii Msg Server: %d bytes remaining in buffer", remaining_bytes);

             memmove(client_conn.rcv_buff, next_msg_start, remaining_bytes);

             client_conn.buff_tail = client_conn.rcv_buff + remaining_bytes;
        }

        // Update our tail - will go back to beginning of buffer if remaining bytes is 0
        client_conn.buff_tail = client_conn.rcv_buff + remaining_bytes;

        sim_logger->debug("Ascii Msg Server: ++++Buffer update: base=%p tail=%p",
           client_conn.rcv_buff, client_conn.buff_tail);
     }

    void AsciiMsgServer::reset_buffer(ClientConnection &client_conn)
    {
        client_conn.buff_tail = client_conn.rcv_buff;

        memset(client_conn.rcv_buff, '\0', sizeof(client_conn.rcv_buff));
    }
}
