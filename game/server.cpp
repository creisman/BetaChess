#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <evhttp.h>
#include <iostream>
#include <memory>
#include <string>

#include "board.h"
#include "flags.h"
#include "search.h"

using namespace std;
using namespace board;
using namespace search;

Search searchT(true /* useTimeControl */);

string suggest(long wTime, long bTime) {
  searchT.updateTime(wTime, bTime);

  FindMoveStats stats = {0, 0};
  scored_move_t suggest = searchT.findMove(
      FLAGS_server_min_ply,
      FLAGS_server_min_nodes,
      &stats);

  double score = get<0>(suggest);
  move_t move = get<1>(suggest);
  string coords = Board::coordinateNotation(move);
  string alg = searchT.getRoot().algebraicNotation_slow(move);

  cout << "Got suggested Move: " << alg << " (raw: " << coords << ")"
       << " score: " << score / 100.00
       << " (searched " << stats.plyR << " plyR and " << stats.nodes << " nodes)" << endl;
  return coords;
}

string update(string move) {
  // TODO fix this (not sure who does it.)
  //if (!move.empty() && move.back() == '+' || move.back() == '#') {
  //  move.pop_back();
  //}

  bool foundMove = searchT.makeAlgebraicMove(move);
  if (!foundMove) {
    cout << "Didn't find move (" << (move.size() + 1) <<  "): \"" << move << "\"" << endl;
    for (Board c : searchT.getRoot().getLegalChildren()) {
      string moveToGetC = searchT.getRoot().algebraicNotation_slow(c.getLastMove());
      cout << "\twasn't \"" << moveToGetC << "\"" << endl;
    }
    assert(false);
  }

  // Check if game is over
  board_s result = searchT.getRoot().getGameResult_slow();
  if (result != Board::RESULT_IN_PROGRESS) {
    cout << "Server updating, game result: " << (int) result << endl;
  }

  string reply = foundMove ? "found" : "not found";
  return reply + " " + move;
}


string null2Empty(const char * cStr) {
  return cStr ? cStr : "";
}


long clockStrToMillis(string display) {
  // HH:MM:SS.MM
  float time = 0;
  while (!display.empty()) {
    size_t sz = 0;
    float part = stof(display.substr(sz), &sz);
    bool isLast = sz == display.size();

    // Skip all consumed characters + delimitor (if not at end)
    display = display.substr(sz + (isLast ? 0 : 1));

    time = (60 * time) + part;
  }
  return time * 1000L;
}


void moveHandler(evhttp_request * req, void *args) {
  auto uri = evhttp_request_get_uri(req);
  auto uriParsed = evhttp_request_get_evhttp_uri(req);

  auto *inBuffer = evhttp_request_get_input_buffer(req);
  size_t inSize = evbuffer_get_length(inBuffer);
  size_t readSize = min(inSize, (size_t) 1000);
  //char data[readSize+1];
  //memset(data, '\0', readSize+1);
  //cout << "\tdata size: " << inSize << endl;
  //evbuffer_copyout(inBuffer, data, readSize);

  struct evkeyvalq uriParams;
  evhttp_parse_query_str(evhttp_uri_get_query(uriParsed), &uriParams);

  string status = null2Empty( evhttp_find_header(&uriParams, "status") );
  string move   = null2Empty( evhttp_find_header(&uriParams, "move") );
  string wClock = null2Empty( evhttp_find_header(&uriParams, "white-clock") );
  string bClock = null2Empty( evhttp_find_header(&uriParams, "black-clock") );

  // TODO a replay function to test again would be nice.

  long wTime = clockStrToMillis(wClock);
  long bTime = clockStrToMillis(bClock);

  cout << "request: \""  << uri    << "\"\t"
       << "( status: \"" << status << "\" ) "
       << "( move: \""   << move   << "\" )"
       << "( time: \""   << wTime  << "\", \"" << bTime << "\" )" << endl;

  string reply;
  if (status == "start-game") {
    cout << "Reloaded board" << endl;;
    //searchT = Search(true /* useTimeControl */);
    reply = "ack on start-game";
  } else if (status == "suggest") {
    reply = suggest(wTime, bTime);
  } else if (!move.empty()) {
    reply = update(move);
  } else {
    reply = "Don't know what you want?";
  }

  //cout << "return: \"" << reply << "\"" << endl << endl;

  auto *outBuffer = evhttp_request_get_output_buffer(req);
  evbuffer_add_printf(outBuffer, "%s", reply.c_str());
  evhttp_add_header(req->output_headers, "Content-Type", "application/json");

  evhttp_send_reply(req, HTTP_OK, "", outBuffer);
}


void tryCatchSaveHandler(evhttp_request * req, void *args) {
  try {
    moveHandler(req, args);

  } catch (...) {
    // Ask search to save current state for potentially debugging.
    cout << endl << endl;
    cout << "ERROR from:" << endl;
    cout << "\t" << evhttp_request_get_uri(req) << endl;

    searchT.save();
  }
}


int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  //assert( IS_ANTICHESS ); // We don't really support playing other gametypes yet.

  if (!event_init()) {
    cerr << "Failed to init libevent." << endl;
    return -1;
  }

  string addr = "0.0.0.0";
  uint16_t port = 5094;

  unique_ptr<evhttp, decltype(&evhttp_free)> server(evhttp_start(addr.c_str(), port), &evhttp_free);
  if (!server) {
    cerr << "Failed to init http server." << endl;
    return -1;
  }

  cout << "Launching Server"
       << "\t(Search with depth = "
       << FLAGS_server_min_ply << ", "
       << FLAGS_server_min_nodes << ")"
       << endl << endl;

  evhttp_set_gencb(server.get(), tryCatchSaveHandler, nullptr);
  if (event_dispatch() == -1) {
    cout << "Failed in message loop" << endl;
    return -1;
  }

  return 0;
}
