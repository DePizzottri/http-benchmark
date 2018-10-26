#define _CRT_SECURE_NO_WARNINGS

#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>

#include <string>
#include <iostream>
#include <thread>
#include <vector>

#define MAXEVENTS 11000

int consume(int fd, char* begin, char* end) {
    thread_local std::string buf = 
R"(HTTP/1.1 200 OK
Server: Seastar httpd
Date: 18 Oct 2018 12:00:10 GMT
Content-Length: 60

hell0
hell1
hell2
hell3
hell4
hell5
hell6
hell7
hell8
hell9
)";

    auto sended = write(fd, buf.c_str(), buf.size());
    
    if(sended != buf.length())
    {
        perror("Error write");
        close(fd);
    }

    return 0;
}


static int
make_socket_non_blocking (int sfd)
{
  int flags, s;

  flags = fcntl (sfd, F_GETFL, 0);
  if (flags == -1)
    {
      perror ("fcntl");
      return -1;
    }

  flags |= O_NONBLOCK;
  s = fcntl (sfd, F_SETFL, flags);
  if (s == -1)
    {
      perror ("fcntl");
      return -1;
    }

  return 0;
}

static int
create_and_bind (char *port)
{
  addrinfo hints;
  addrinfo *result, *rp;
  int s, sfd;

  memset (&hints, 0, sizeof (struct addrinfo));
  hints.ai_family = AF_UNSPEC;     /* Return IPv4 and IPv6 choices */
  hints.ai_socktype = SOCK_STREAM; /* We want a TCP socket */
  hints.ai_flags = AI_PASSIVE;     /* All interfaces */

  s = getaddrinfo (NULL, port, &hints, &result);
  if (s != 0)
    {
      fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (s));
      return -1;
    }

  for (rp = result; rp != NULL; rp = rp->ai_next)
    {
      sfd = socket (rp->ai_family, rp->ai_socktype, rp->ai_protocol);
      if (sfd == -1)
        continue;

      s = bind (sfd, rp->ai_addr, rp->ai_addrlen);
      if (s == 0)
        {
          /* We managed to bind successfully! */
          break;
        }

      close (sfd);
    }

  if (rp == NULL)
    {
      fprintf (stderr, "Could not bind\n");
      return -1;
    }

  freeaddrinfo (result);

  return sfd;
}

#include <thread>
#include <vector>
#include <cstring>

void read_loop(int efd, int sfd, epoll_event* events) {
      int n, i;

      n = epoll_wait (efd, events, MAXEVENTS/4, 0);
      for (i = 0; i < n; i++)
	{
	  if ((events[i].events & EPOLLERR) ||
              (events[i].events & EPOLLHUP) ||
              (!(events[i].events & EPOLLIN)))
	    {
              /* An error has occured on this fd, or the socket is not
                 ready for reading (why were we notified then?) */
	      //fprintf (stderr, "epoll error\n");
	      close (events[i].data.fd);
	      continue;
	    }

	  else if (sfd == events[i].data.fd)
	    {
              /* We have a notification on the listening socket, which
                 means one or more incoming connections. */
              while (1)
                {
                  struct sockaddr in_addr;
                  socklen_t in_len;
                  int infd;
                  char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

                  in_len = sizeof in_addr;
                  infd = accept (sfd, &in_addr, &in_len);
                  if (infd == -1)
                    {
                      if ((errno == EAGAIN) ||
                          (errno == EWOULDBLOCK))
                        {
                          /* We have processed all incoming
                             connections. */
                          break;
                        }
                      else
                        {
                          //perror ("accept");
                          break;
                        }
                    }

                  /* Make the incoming socket non-blocking and add it to the
                     list of fds to monitor. */
                  auto s = make_socket_non_blocking (infd);
                  if (s == -1)
                    abort ();
                  epoll_event event;
                  event.data.fd = infd;
                  event.events = EPOLLIN | EPOLLET;
                  s = epoll_ctl (efd, EPOLL_CTL_ADD, infd, &event);
                  if (s == -1)
                    {
                      //perror ("epoll_ctl");
                      abort ();
                    }

                }
              continue;
            }
          else
            {
              /* We have data on the fd waiting to be read. Read and
                 display it. We must read whatever data is available
                 completely, as we are running in edge-triggered mode
                 and won't get a notification again for the same
                 data. */
              //int done = 0;

              //while (1)
              //  {

                  //gqueue()->enqueue([fd = events[i].data.fd] {
                  auto fd = events[i].data.fd;
                  ssize_t count;
                  thread_local char buf[2048];

                  count = read (fd, buf, sizeof buf);
                  if (count == -1)
                    {
                      /* If errno == EAGAIN, that means we have read all
                         data. So go back to the main loop. */
                      if (errno != EAGAIN)
                        {
                          //perror ("read");
                          //done = 1;                                                
                        }
                      continue;
                    }
                  else if (count == 0)
                    {
                      /* End of file. The remote has closed the
                         connection. */
                      //printf("remote host closed connection on descriptor %d\n", fd);
                      close(fd);
                      continue;
                    
                    }

                  /* Write the buffer to standard output */
                   consume(fd, buf, buf + count);
              //});
            }
        }
}

int
main (int argc, char *argv[])
{
  std::cout << "epoll+nomadb oct 18 (parallel epoll, affinity, full unsafe, epoll 0)" << std::endl;

  int sfd, s;
  int efd;
  epoll_event event;

  sfd = create_and_bind ((char*)"8080");
  if (sfd == -1)
    abort ();

  s = make_socket_non_blocking (sfd);
  if (s == -1)
    abort ();

  s = listen (sfd, SOMAXCONN);
  if (s == -1)
  {
    perror ("listen");
    abort ();
  }

  efd = epoll_create1 (0);
  if (efd == -1)
  {
    perror ("epoll_create");
    abort ();
  }

  event.data.fd = sfd;
  event.events = EPOLLIN | EPOLLET;
  s = epoll_ctl (efd, EPOLL_CTL_ADD, sfd, &event);
  if (s == -1)
  {
    perror ("epoll_ctl");
    abort ();
  }

  using namespace std;

  vector<thread> threads;

  const auto thread_num = 2;

  for(int i = 0; i<thread_num; ++i) {
      threads.push_back(thread{[&] {
    //   cpu_set_t cpuset;
    //   pthread_t thread;

    //   thread = pthread_self();

    //   CPU_ZERO(&cpuset);
    //   CPU_SET(i, &cpuset);
    //   int s1;
    //   s1 = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
      auto events = (epoll_event*) calloc (MAXEVENTS/4, sizeof (epoll_event));

      while(1) {
        read_loop(efd, sfd, events);
      }
    }});
  }

  for(auto& th: threads) {
    th.join();
  }  

  close (sfd);

  return EXIT_SUCCESS;
}
