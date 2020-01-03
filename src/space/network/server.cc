#include <cstdio>
#include <cstring>

#include "platform/platform.cc"

static ThreadInfo thread;

struct ServerParam {
  const char* ip;
  const char* port;
};
static ServerParam thread_param;

struct PlayerState {
  Udp4 peer;
  uint64_t num_players;
  uint64_t game_id;
  uint64_t last_active;
};
static PlayerState zero_player;

static bool running = true;
#define MAX_BUFFER (4 * 1024)
const uint64_t greeting_size = 6;
const uint64_t greeting_packet = greeting_size + sizeof(uint64_t);
const char greeting[greeting_size] = {"space"};
#define MAX_PLAYER 2
PlayerState player[MAX_PLAYER];
uint64_t next_game_id = 1;
bool game_ready;
Clock_t server_clock;
#define TIMEOUT_USEC (2 * 1000 * 1000)

int
GetPlayerIndexFromPeer(Udp4* peer)
{
  for (int i = 0; i < MAX_PLAYER; ++i) {
    if (memcmp(peer, &player[i].peer, sizeof(Udp4)) == 0) return i;
  }

  return -1;
}

int
GetNextPlayerIndex()
{
  for (int i = 0; i < MAX_PLAYER; ++i)
  {
    if (memcmp(&zero_player, &player[i], sizeof(PlayerState)) == 0)
      return i;
  }

  return -1;
}

void
drop_inactive_players(uint64_t rt_usec)
{
  for (int i = 0; i < MAX_PLAYER; ++i) {
    if (memcmp(&zero_player, &player[i], sizeof(PlayerState)) == 0) continue;
    if (rt_usec - player[i].last_active > TIMEOUT_USEC) {
      player[i] = PlayerState{};
      printf("dropped player %d\n", i);
    }
  }
}

void*
server_main(ThreadInfo* t)
{
  ServerParam* thread_param = (ServerParam*)t->arg;

  uint8_t buffer[MAX_BUFFER];
  if (!udp::Init()) {
    puts("server: fail init");
    return 0;
  }

  Udp4 location;
  if (!udp::GetAddr4(thread_param->ip, thread_param->port, &location)) {
    puts("server: fail GetAddr4");
    puts(thread_param->ip);
    puts(thread_param->port);
    return 0;
  }

  printf("Server binding %s:%s\n", thread_param->ip, thread_param->port);
  if (!udp::Bind(location)) {
    puts("server: fail Bind");
    return 0;
  }

  uint64_t realtime_usec = 0;
  uint64_t time_step = 1000;
  platform::clock_init(time_step, &server_clock);
  while (running) {
    uint16_t received_bytes;
    Udp4 peer;

    uint64_t sleep_usec;
    if (platform::elapse_usec(&server_clock, &sleep_usec)) {
      realtime_usec += time_step;
    }
    if (!udp::ReceiveAny(location, MAX_BUFFER, buffer, &received_bytes,
                         &peer)) {
      if (udp_errno) running = false;
      if (udp_errno) printf("udp_errno %d\n", udp_errno);
      drop_inactive_players(realtime_usec);
      platform::sleep_usec(sleep_usec);
      continue;
    }

    int pidx = GetPlayerIndexFromPeer(&peer);

    // Handshake packet
    if (received_bytes >= greeting_packet &&
        strncmp(greeting, (char*)buffer, greeting_size) == 0) {
      // No room for clients on this server
      int player_index = GetNextPlayerIndex();
      if (player_index == -1) continue;
      // Duplicate handshake packet, idx already assigned
      if (pidx != -1) continue;

      uint64_t* header = (uint64_t*)(buffer + greeting_size);
      uint64_t num_players = *header;
      ++header;
      printf("Accepted %d\n", player_index);
      player[player_index].peer = peer;
      player[player_index].num_players = num_players;
      player[player_index].game_id = 0;
      player[player_index].last_active = realtime_usec;

      int ready_players = 0;
      for (int i = 0; i < MAX_PLAYER; ++i) {
        if (player[i].game_id) continue;
        if (player[i].num_players != num_players) continue;
        ++ready_players;
      }

      if (ready_players >= num_players) {
        uint64_t* header = (uint64_t*)(buffer);
        memcpy(buffer, greeting, greeting_size);

        uint64_t player_id = 0;
        for (int i = 0; i < MAX_PLAYER; ++i) {
          if (player[i].game_id) continue;
          if (player[i].num_players != num_players) continue;

          printf("greet player index %d\n", i);
          header = (uint64_t*)(buffer + greeting_size);
          *header = player_id;
          ++header;
          *header = num_players;
          ++header;
          *header = next_game_id;
          ++header;
          if (!udp::SendTo(location, player[i].peer, buffer,
                           greeting_size + 3 * sizeof(uint64_t)))
            puts("greet failed");
          player[i].game_id = next_game_id;
          ++player_id;
        }
        ++next_game_id;
      }
    }

    // Filter Identified clients
    if (pidx == -1) continue;

    // Mark player connection active
    player[pidx].last_active = realtime_usec;

    // Filter for game-ready clients
    if (!player[pidx].game_id) continue;

    // Echo bytes to game participants
    uint64_t game_id = player[pidx].game_id;
#if 0
    printf("socket %d echo %d bytes to %lu game_id\n", location.socket, received_bytes, game_id);
#endif
    for (int i = 0; i < MAX_PLAYER; ++i) {
      if (player[i].game_id != game_id) continue;

      if (!udp::SendTo(location, player[i].peer, buffer, received_bytes)) {
        puts("server send failed");
        break;
      }
    }
  }

  return 0;
}

bool
CreateNetworkServer(const char* ip, const char* port)
{
  if (thread.id) return false;

  thread.arg = &thread_param;
  thread_param.ip = ip;
  thread_param.port = port;
  platform::thread_create(&thread, server_main);

  return true;
}

uint64_t
WaitForNetworkServer()
{
  if (!thread.id) return 0;

  platform::thread_join(&thread);
  return thread.return_value;
}

