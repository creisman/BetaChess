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

using namespace std;
using namespace board;
using namespace book;

Book bookT;
Board boardT;
vector<string> moves;

// Read-Evaluate-Play loop.
string repLoop() {
  cout << endl << "looking for suggestion (" << moves.size() << ") moves in" << endl;
  boardT.printBoard();

  // Some Book stuff here.
  scored_move_t suggest;

  string bookMove = bookT.multiArmBandit(boardT);
  bool foundBookResponse = false;
  if (bookMove != "") {
    cout << "Got move(" << bookMove << ") from book" << endl;

    // Verify it's a real move
    for (Board c : boardT.getLegalChildren()) {
      if (bookMove == boardT.algebraicNotation_slow(c.getLastMove())) {
        suggest = make_pair(NAN, c.getLastMove());
        foundBookResponse = true;
        break;
      }
    }
    if (!foundBookResponse) {
      cout << "Got bad suggestion from book after: " << endl;
      for (string moveName : moves) {
        cout << "\t" << moveName << endl;
      }
    }
  }

  FindMoveStats stats;
  if (!foundBookResponse) {
    // Number of nodes that can be evaled quickly.
    suggest = boardT.findMove(
        FLAGS_server_min_depth,
        FLAGS_server_min_nodes,
        &stats);
  }

  double score = get<0>(suggest);
  move_t move = get<1>(suggest);
  string coords = boardT.coordinateNotation(move);
  string alg = boardT.algebraicNotation_slow(move);
  int nodesS = foundBookResponse ? 0 : stats.nodes;
  int depthS = foundBookResponse ? 0 : stats.plyR;

  cout << "Got suggested Move: " << alg << " (raw: " << coords << ")"
       << " score: " << score / 100.00
       << " (searched " << nodesS << " nodes in " << depthS << " ply)" << endl;
  return coords;
}

string update(string move) {
  // TODO fix this (not sure who does it.)
  //if (!move.empty() && move.back() == '+' || move.back() == '#') {
  //  move.pop_back();
  //}

  bool foundMove = boardT.makeAlgebraicMove_slow(move);
  if (!foundMove) {
    cout << "Didn't find move (" << (move.size() + 1) <<  "): \"" << move << "\"" << endl;
    for (Board c : boardT.getLegalChildren()) {
      string moveToGetC = boardT.algebraicNotation_slow(c.getLastMove());
      cout << "\twasn't \"" << moveToGetC << "\"" << endl;
    }
    assert(false);
  }
  moves.push_back(move);

  // Check if game is over
  board_s result = boardT.getGameResult_slow();
  if (result != Board::RESULT_IN_PROGRESS) {
    cout << "Server updating, game result: " << (int) result << endl;

    // Update some of the book (let Book figure out how much).
    bookT.updateResult(moves, result /* for white */);
    bookT.write();
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

  string startHeader = null2Empty( evhttp_find_header(&uriParams, "start") );
  string moveHeader  = null2Empty( evhttp_find_header(&uriParams, "move") );
  cout << "request: \"" << uri << "\"\t"
       << "( start: \"" << startHeader << "\" ) "
       << "( move: \"" << moveHeader << "\" )" << endl;

  string reply;
  if (!startHeader.empty()) {
    boardT = Board();
    moves.clear();

    cout << "Reloaded board" << endl;;
    reply = "ack on start-game";
  } else if (moveHeader == "suggest") {
    reply = repLoop();
  } else if (!moveHeader.empty()) {
    reply = update(moveHeader);
  } else {
    reply = "Don't know what you want?";
  }

  cout << "return: \"" << reply << "\"" << endl << endl;

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
         << FLAGS_server_min_depth << ", "
         << FLAGS_server_min_nodes << ")"
       << endl << endl;
  bookT.load();

  evhttp_set_gencb(server.get(), genericHandler, nullptr);
  if (event_dispatch() == -1) {
    cout << "Failed in message loop" << endl;
    return -1;
  }

  // Happens on server shutdown.
  return 0;
}
