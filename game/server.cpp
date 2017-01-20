#include <cmath>
#include <cstdint>
#include <evhttp.h>
#include <iostream>
#include <cstring>
#include <string>
#include <memory>

#include "board.h"
#include "book.h"

using namespace std;
using namespace board;
using namespace book;

Book bookT;
Board boardT(true /* init */);
vector<move_t> moves;
vector<string> moveNames;

// Read-Evaluate-Play loop.
string repLoop(int ply) {
  cout << "looking for suggestion (" << moves.size() << ") moves in" << endl;

  // Some Book stuff here.
  scored_move_t suggest;

  move_t *bookMove = bookT.multiArmBandit(moves);
  bool foundBookResponse = false;
  if (bookMove != nullptr) {
    cout << "Got move from book" << endl;
    suggest = make_pair(NAN, *bookMove);

    // Verify it's a real move
    for (Board c : boardT.getLegalChildren()) {
      if (*bookMove == c.getLastMove()) {
        foundBookResponse = true;
        break;
      }
    }
    if (!foundBookResponse) {
      cout << "Got bad suggestion from book after: " << endl;
      for (string moveName : moveNames) {
        cout << "\t" << moveName << endl;
      }
    }
  }

  if (!foundBookResponse) {
    suggest = boardT.findMove(ply);
  }

  double score = get<0>(suggest);
  move_t move = get<1>(suggest);
  string coords = boardT.coordinateNotation(move);
  string alg = boardT.algebraicNotation(move);

  cout << "Got suggested Move: " << alg << " (raw: " << coords << ") score: " << score << endl;
  return coords;
}

string update(string move) {
  // TODO fix this (not sure who does it.)
  if (!move.empty() && move.back() == '+' || move.back() == '#') {
    move.pop_back();
  }

  bool foundMove = false;
  for (Board c : boardT.getLegalChildren()) {
    string moveToGetC = boardT.algebraicNotation(c.getLastMove());
    if (moveToGetC == move) {
      cout << "found move(" << move << ")!" << endl;
      foundMove = true;
      boardT = c;
      moves.push_back(c.getLastMove());
      moveNames.push_back(moveToGetC);
      break;
    }
  }

  if (!foundMove) {
    cout << "Didn't find move: " << move << endl;
    for (Board c : boardT.getLegalChildren()) {
      string moveToGetC = boardT.algebraicNotation(c.getLastMove());
      cout << "\twasn't " << moveToGetC << endl;
    }
  }

  boardT.printBoard();
  string reply = foundMove ? "found" : "not found";
  return reply + " " + move;
}


string null2Empty(const char * cStr) {
  return cStr ? cStr : "";
}


void genericHandler(evhttp_request * req, void *args) {
  auto uri = evhttp_request_get_uri(req);
  auto uriParsed = evhttp_request_get_evhttp_uri(req);
  cout << "request: \"" << uri << "\"" << endl;

  auto *inBuffer = evhttp_request_get_input_buffer(req);
  size_t inSize = evbuffer_get_length(inBuffer);
  size_t readSize = min(inSize, (size_t) 1000);
  char data[readSize+1];
  memset(data, '\0', readSize+1);
  cout << "\tdata size: " << inSize << endl;

  evbuffer_copyout(inBuffer, data, readSize);

  struct evkeyvalq uriParams;
  evhttp_parse_query_str(evhttp_uri_get_query(uriParsed), &uriParams);

  string startHeader = null2Empty( evhttp_find_header(&uriParams, "start") );
  string moveHeader  = null2Empty( evhttp_find_header(&uriParams, "move") );

  cout << "\tparsed args:" << endl
       << "\t\tstart: \"" << startHeader << "\"" << endl
       << "\t\tmove: \"" << moveHeader << "\"" << endl << endl;


  string reply;
  if (!startHeader.empty()) {
    boardT = Board(true /* init */);
    moves.clear();
    moveNames.clear();

    cout << "Reloaded board" << endl;;
    reply = "ack on start-game";
  } else if (moveHeader == "suggest") {
    reply = repLoop(4);
  } else if (!moveHeader.empty()) {
    reply = update(moveHeader);
  } else {
    reply = "Don't know what you want?";
  }

  cout << "return: \"" << reply << "\"" << endl << endl << endl;

  auto *outBuffer = evhttp_request_get_output_buffer(req);
  evbuffer_add_printf(outBuffer, "%s", reply.c_str());
  evhttp_add_header(req->output_headers, "Content-Type", "application/json");

  evhttp_send_reply(req, HTTP_OK, "", outBuffer);
}


int main() {
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

  cout << "Launching Server" << endl << endl;
  bookT.load();

  evhttp_set_gencb(server.get(), genericHandler, nullptr);
  if (event_dispatch() == -1) {
    cout << "Failed in message loop" << endl;
    return -1;
  }

  // Happens on server shutdown.
  return 0;
}
