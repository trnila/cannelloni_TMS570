#include <arpa/inet.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <map>
#include <utility>
#include <memory>
#include <thread>
#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-common/error.h>
#include <avahi-common/malloc.h>
#include <avahi-common/simple-watch.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sstream>

class Endpoint {
 public:
  virtual void read(std::vector<struct can_frame> &frames) = 0;
  virtual void write(std::vector<struct can_frame> &frames) = 0;

  int get_fd() const {
    return fd;
  }

 protected:
  int fd;
};

class CANEndpoint : public Endpoint {
 public:
  CANEndpoint(const char *if_name) {
    fd = socket(PF_CAN, SOCK_RAW | SOCK_NONBLOCK, CAN_RAW);
    if (fd == -1) {
      perror("socket");
      exit(1);
    }

    struct ifreq ifr;
    strcpy(ifr.ifr_name, if_name);
    if (ioctl(fd, SIOCGIFINDEX, &ifr) < 0) {
      perror("ioctl");
      exit(1);
    }

    struct sockaddr_can addr;
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
      perror("bind");
      exit(1);
    }
  }

  void read(std::vector<struct can_frame> &frames) override {
    struct can_frame frame;
    size_t n = ::read(fd, &frame, sizeof(frame));
    if (n != sizeof(frame)) {
      fprintf(stderr, "read failed %d != %d\n", n, sizeof(frame));
      exit(1);
    }

    frames.emplace_back(frame);
  }

  void write(std::vector<struct can_frame> &frames) override {
    for (const struct can_frame &frame : frames) {
      size_t n = ::write(fd, &frame, sizeof(frame));
      if (n != sizeof(frame)) {
        fprintf(stderr, "write failed %d != %d\n", n, sizeof(frame));
        exit(1);
      }
    }
  }
};

class UDPEndpoint : public Endpoint {
 public:
  UDPEndpoint(const char *addr, uint16_t port) {
    char service[16];
    snprintf(service, sizeof(service), "%d", port);
    struct addrinfo hints = {};
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_DGRAM;
    struct addrinfo *res = nullptr;
    if (getaddrinfo(addr, service, &hints, &res) != 0) {
      perror("getaddrinfo");
      exit(1);
    }

    fd = socket(AF_INET6, SOCK_DGRAM | SOCK_NONBLOCK, 0);
    if (fd < 0) {
      perror("socket creation failed");
      exit(1);
    }

    int enabled = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled)) != 0) {
      perror("setsockopt(SOL_SOCKET, SO_REUSEADDR)");
      exit(1);
    }

    struct sockaddr_in6 server_addr;
    memcpy(&server_addr, res->ai_addr, res->ai_addrlen);
    server_addr.sin6_addr.s6_addr16[0] = htons(0xff02);
    if (bind(fd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
      perror("bind failed");
      exit(1);
    }

    struct ipv6_mreq mreq = {};
    mreq.ipv6mr_multiaddr = server_addr.sin6_addr;
    mreq.ipv6mr_interface = server_addr.sin6_scope_id;
    if (setsockopt(fd, IPPROTO_IPV6, IPV6_JOIN_GROUP, &mreq, sizeof(mreq)) != 0) {
      perror("setsockopt(PPROTO_IPV6, IPV6_JOIN_GROUP)");
      exit(1);
    }

    if (sizeof(dst) != res->ai_addrlen) {
      fprintf(stderr, "Wrong address size %ld != %ld\n", sizeof(dst.sin6_addr), res->ai_addrlen);
      exit(1);
    }
    memcpy(&dst, res->ai_addr, res->ai_addrlen);

    freeaddrinfo(res);
  }

  void read(std::vector<struct can_frame> &frames) override {
    uint8_t buffer[1024];
    ssize_t n = ::read(fd, buffer, sizeof(buffer));
    if (n <= 0) {
      fprintf(stderr, "read failed %d\n", n);
      exit(1);
    }

    uint16_t count = (buffer[3] << 8) | buffer[4];
    size_t pos = 5;
    while (pos < n) {
      uint32_t id = (buffer[pos] << 24) | (buffer[pos + 1] << 16) |
                    (buffer[pos + 2] << 8) | buffer[pos + 3];
      uint8_t len = buffer[pos + 4];

      struct can_frame frame;
      frame.can_id = id;
      frame.can_dlc = len;
      pos += 5;
      for (int i = 0; i < len; i++) {
        frame.data[i] = buffer[pos++];
      }

      frames.emplace_back(frame);
      count--;
    }

    if (count != 0) {
      fprintf(stderr, "frames %d missing\n", count);
    }
  }

  void write(std::vector<struct can_frame> &frames) override {
    uint8_t tx[128];
    size_t pos = 0;
    tx[pos++] = 2;  // version
    tx[pos++] = 0;  // data
    tx[pos++] = 0;  // seq
    tx[pos++] = 0;
    tx[pos++] = 1;

    for (const can_frame &frame : frames) {
      uint32_t canid = htonl(frame.can_id);
      memcpy(&tx[pos], &canid, 4);
      pos += 4;

      tx[pos++] = frame.can_dlc;
      memcpy(&tx[pos], frame.data, frame.can_dlc);
      pos += frame.can_dlc;
    }

    ssize_t n = sendto(fd, tx, pos, 0, (struct sockaddr *)&dst, sizeof(dst));
    if (n != pos) {
      perror("UDP sendto failed");
      exit(1);
    }
  }

 private:
  struct sockaddr_in6 dst;
};

struct Bridge {
  Bridge() = default;
  Bridge(const std::shared_ptr<Endpoint> &rx, const std::shared_ptr<Endpoint> &tx) : rx(rx), tx(tx) {}

  std::shared_ptr<Endpoint> rx;
  std::shared_ptr<Endpoint> tx;
};

class Runner {
 public:
  Runner() {
    epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd < 0) {
      perror("epoll_create1");
      exit(1);
    }
  }

  void add(const char *canif_name, const char *addr, uint16_t port) {
    printf("Bridging %s <-> %s:%d\n", canif_name, addr, port);
    auto can = std::make_shared<CANEndpoint>(canif_name);
    auto udp = std::make_shared<UDPEndpoint>(addr, port);

    fds[can->get_fd()] = Bridge(can, udp);
    fds[udp->get_fd()] = Bridge(udp, can);

    add_epoll(can->get_fd());
    add_epoll(udp->get_fd());
  }

  void run() {
    std::vector<struct can_frame> frames;
    const size_t max_events = 16;
    for (;;) {
      struct epoll_event evts[max_events];
      int nfds = epoll_wait(epoll_fd, evts, max_events, -1);
      if (nfds == -1) {
        perror("epoll_wait");
        exit(1);
      }

      for (size_t i = 0; i < nfds; i++) {
        auto &tunnel = fds[evts[i].data.fd];

        frames.clear();
        tunnel.rx->read(frames);
        tunnel.tx->write(frames);
      }
    }
  }

 private:
  int epoll_fd;
  std::map<int, Bridge> fds;

  void add_epoll(int fd) {
    struct epoll_event evt = {0};
    evt.events = EPOLLIN;
    evt.data.fd = fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &evt) == -1) {
      perror("epoll_ctl");
      exit(1);
    }
  }
};

class Discovery {
 public:
  Discovery(Runner &runner) : runner(runner) {
    AvahiSimplePoll *simple_poll = avahi_simple_poll_new();
    if (!simple_poll) {
      fprintf(stderr, "Failed to create simple poll object.\n");
      exit(1);
    }

    int error = 0;
    client = avahi_client_new(avahi_simple_poll_get(simple_poll), (AvahiClientFlags)0, NULL, NULL, &error);
    if (!client) {
      fprintf(stderr, "Failed to create client: %s\n", avahi_strerror(error));
      exit(1);
    }

    AvahiServiceBrowser *sb = avahi_service_browser_new(client, AVAHI_IF_UNSPEC, AVAHI_PROTO_INET6, "_cannelloni._udp", NULL, (AvahiLookupFlags)0, browse_callback, this);
    if (!sb) {
      fprintf(stderr, "Failed to create service browser: %s\n", avahi_strerror(avahi_client_errno(client)));
      exit(1);
    }

    avahi_thread = std::thread([simple_poll] { avahi_simple_poll_loop(simple_poll); });
  }

  static void resolve_callback(
      AvahiServiceResolver *r,
      AVAHI_GCC_UNUSED AvahiIfIndex interface,
      AVAHI_GCC_UNUSED AvahiProtocol protocol,
      AvahiResolverEvent event,
      const char *name,
      const char *type,
      const char *domain,
      const char *host_name,
      const AvahiAddress *address,
      uint16_t port,
      AvahiStringList *txt,
      AvahiLookupResultFlags flags,
      void *userdata) {
    auto *discovery = static_cast<Discovery *>(userdata);

    switch (event) {
      case AVAHI_RESOLVER_FAILURE:
        fprintf(stderr, "Failed to resolve service '%s' of type '%s' in domain '%s': %s\n", name, type, domain, avahi_strerror(avahi_client_errno(avahi_service_resolver_get_client(r))));
        break;

      case AVAHI_RESOLVER_FOUND: {
        char address_str[AVAHI_ADDRESS_STR_MAX];
        avahi_address_snprint(address_str, sizeof(address_str), address);
        char ifname[IF_NAMESIZE];
        if (!if_indextoname(interface, ifname)) {
          perror("if_indextoname");
          exit(1);
        }

        std::stringstream ss;
        ss << address_str << '%' << ifname;

        discovery->runner.add(name, ss.str().c_str(), port);
      }
    }

    avahi_service_resolver_free(r);
  }

  static void browse_callback(
      AvahiServiceBrowser *b,
      AvahiIfIndex interface,
      AvahiProtocol protocol,
      AvahiBrowserEvent event,
      const char *name,
      const char *type,
      const char *domain,
      AvahiLookupResultFlags flags,
      void *userdata) {
    switch (event) {
      case AVAHI_BROWSER_FAILURE:
        fprintf(stderr, "Browser failure: %s\n", avahi_strerror(avahi_client_errno(avahi_service_browser_get_client(b))));
        break;

      case AVAHI_BROWSER_NEW:
        Discovery *discovery = static_cast<Discovery *>(userdata);
        if (!(avahi_service_resolver_new(discovery->client, interface, protocol, name, type, domain, AVAHI_PROTO_UNSPEC, (AvahiLookupFlags)0, resolve_callback, discovery))) {
          fprintf(stderr, "Failed to resolve service '%s': %s\n", name, avahi_strerror(avahi_client_errno(discovery->client)));
        }
        break;
    }
  }

 private:
  AvahiClient *client;
  std::thread avahi_thread;
  Runner &runner;
};

int main(int argc, char **argv) {
  Runner runner;
  Discovery discovery(runner);

  for (int i = 1; i < argc; i++) {
    char *pos1 = strchr(argv[i], ':');
    char *pos2 = strrchr(argv[i], ':');
    if (!pos1 || !pos2 || pos1 == pos2) {
      fprintf(stderr, "Invalid bridge: '%s'\n", argv[i]);
      exit(1);
    }
    *pos1 = '\0';
    *pos2 = '\0';
    runner.add(argv[i], pos1 + 1, atoi(pos2 + 1));
  }

  runner.run();
}
