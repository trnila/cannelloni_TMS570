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
    fd = socket(AF_INET6, SOCK_DGRAM | SOCK_NONBLOCK, 0);
    if (fd < 0) {
      perror("socket creation failed");
      exit(1);
    }

    char *iface = "eth";
    if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, iface, strlen(iface) + 1) < 0) {
      perror("setsockopt(SO_BINDTODEVICE) failed");
      exit(1);
    }

    struct sockaddr_in6 server_addr = {};
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_addr = in6addr_any;
    server_addr.sin6_port = htons(port);
    if (bind(fd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
      perror("bind failed");
      exit(1);
    }

    dst.sin6_family = AF_INET6;
    dst.sin6_port = htons(port);
    inet_pton(AF_INET6, addr, &dst.sin6_addr);
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

int main(int argc, char **argv) {
  Runner runner;

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
  runner.add("can-0-0", "fe80::1222:3344:5500", 20000);
  runner.run();
}
