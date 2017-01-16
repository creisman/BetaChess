#include <cmath>
#include <cstdint>
#include <evhttp.h>
#include <iostream>
#include <cstring>
#include <string>
#include <memory>

#include "board.h"

using namespace std;
using namespace board;

// We only can process one game at a time (currently).
Board b(true /* init */);
int halfMoveNum = 0;
vector<move_t> moves = 0;

// Read-Evaluate-Play loop.
string repLoop(int ply) {

  // Some Book stuff here.
  if (halfMoveNum == 0) {
    return "g2 - g3";
  }

  cout << "looking for suggestion on halfMoveNum: " << halfMoveNum << endl;

  scored_move_t suggest = b.findMove(ply);
  double score = get<0>(suggest);
  move_t move = get<1>(suggest);

  string coords = b.coordinateNotation(move);
  
  string alg = b.algebraicNotation(move); 

  cout << "Got suggested Move: " << alg << " (raw: " << coords << ") score: " << score << endl;
  return coords;
}

string update(string move) {
  // TODO fix this (not sure who does it.)
  if (!move.empty() && move.back() == '+' || move.back() == '#') {
    move.pop_back();
  }

  bool foundMove = false;
  for (Board c : b.getLegalChildren()) {
    string moveToGetC = b.algebraicNotation(c.getLastMove());
    if (moveToGetC == move) {
      cout << "found move(" << move << ")!" << endl;
      foundMove = true;
      b = c;
      halfMoveNum += 1;
      moves.clear();
      break;
    }
  }

  if (!foundMove) {
    cout << "Didn't find move: " << move << endl;
    for (Board c : b.getLegalChildren()) {
      string moveToGetC = b.algebraicNotation(c.getLastMove());
      cout << "\twasn't " << moveToGetC << endl;
    }
  }

  b.printBoard();
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
  size_t readSize = min(inSize, (unsigned int) 1000);
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
    b = Board(true /* init */);
    halfMoveNum = 0;
    cout << "Reloaded board" << endl;;
    reply = "ack on start-game";
  } else if (moveHeader == "suggest") {
    reply = repLoop(4);
  } else if (!moveHeader.empty()) {
    // assume we are playing the same game as before.
    reply = update(moveHeader);
  } else {
    reply = "Don't know what you want?";
  }

  cout << "return: \"" << reply << "\"" << endl << endl << endl;

  auto *outBuffer = evhttp_request_get_output_buffer(req);
  evbuffer_add_printf(outBuffer, reply.c_str());
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
  evhttp_set_gencb(server.get(), genericHandler, nullptr);
  if (event_dispatch() == -1) {
    cout << "Failed in message loop" << endl;
    return -1;
  }

  // Happens on server shutdown.
  return 0;
}
