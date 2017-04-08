#include <cmath>
#include <cassert>
#include <cstdint>
#include <evhttp.h>
#include <iostream>
#include <cstring>
#include <string>
#include <memory>

#include "board.h"
#include "book.h"
#include "flags.h"
#include "search.h"

using namespace std;
using namespace board;
using namespace book;
using namespace search;

Search searchT;

// Read-Evaluate-Play loop.
string repLoop() {
  // TODO move book and this to search.
  //cout << endl << "looking for suggestion (" << moves.size() << ") moves in" << endl;
  //cout << "fen: " << boardT.generateFen_slow() << endl;
  //boardT.printBoard();

  FindMoveStats stats = {0, 0};
  // Number of nodes that can be evaled quickly.
  scored_move_t suggest = searchT.findMove(
      FLAGS_server_min_ply,
      FLAGS_server_min_nodes,
      &stats);

  double score = get<0>(suggest);
  move_t move = get<1>(suggest);
  string coords = Board::coordinateNotation(move);
  string alg = searchT.getRoot().algebraicNotation_slow(move);

  cout << "Got suggested Move: " << alg << " (raw: " << coords << ")"
       << " score: " << Search::scoreString(score)
       << " (searched " << stats.plyR << " plyR and " << stats.nodes << " nodes)" << endl;
  return coords;
}

string update(string move, string wTime, string bTime) {
  // TODO fix this (not sure who does it.)
  //if (!move.empty() && move.back() == '+' || move.back() == '#') {
  //  move.pop_back();
  //}

  bool foundMove = searchT.makeAlgebraicMove(move);
  searchT.updateTime(wTime, bTime);
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


void genericHandler(evhttp_request * req, void *args) {
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

  string startHeader   = null2Empty( evhttp_find_header(&uriParams, "start") );
  string moveHeader    = null2Empty( evhttp_find_header(&uriParams, "move") );
  string wTimeHeader    = null2Empty( evhttp_find_header(&uriParams, "wTime") );
  string bTimeHeader  = null2Empty( evhttp_find_header(&uriParams, "bTime") );

  cout << "request: \"" << uri << "\"\t"
       << "( start: \"" << startHeader << "\" ) "
       << "( move: \"" << moveHeader << "\" )" << endl;

  string reply;
  if (!startHeader.empty()) {
    searchT = Search();

    cout << "Reloaded board" << endl;;
    reply = "ack on start-game";
  } else if (moveHeader == "suggest") {
    reply = repLoop();
  } else if (!moveHeader.empty()) {
    reply = update(moveHeader, wTimeHeader, bTimeHeader);
  } else {
    reply = "Don't know what you want?";
  }

  //cout << "return: \"" << reply << "\"" << endl << endl;

  auto *outBuffer = evhttp_request_get_output_buffer(req);
  evbuffer_add_printf(outBuffer, "%s", reply.c_str());
  evhttp_add_header(req->output_headers, "Content-Type", "application/json");

  evhttp_send_reply(req, HTTP_OK, "", outBuffer);
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

  evhttp_set_gencb(server.get(), genericHandler, nullptr);
  if (event_dispatch() == -1) {
    cout << "Failed in message loop" << endl;
    return -1;
  }

  // Happens on server shutdown.
  return 0;
}
