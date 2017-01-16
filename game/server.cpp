#include <cmath>
#include <cstdint>
#include <evhttp.h>
#include <iostream>
#include <cstring>
#include <memory>

#include "board.h"

using namespace std;
using namespace board;

// We only can process one game at a time (currently).
Board b(true /* init */);

// Read-Evaluate-Play loop.
string repLoop(int ply, string fen, string move) {
  return "TODO response to " + move;
  //if (!fen.empty()) {
  //  b = Board(fen);
  //  cout << "Loaded from fen: \"" << fen << "\"" << endl;
  //}
/*
  //b.printBoard();
  bool weAreWhite;
  cout << "[W]hite or [B]lack?" << endl;
  {
    string test;
    cin >> test;
    weAreWhite = (test == "w" || test == "W" ||
                  test == "White" || test == "white");
  }

  cout << "We are playing white: " << weAreWhite << endl;

  // white start with c4.
  if (weAreWhite) {
    b.makeMove(1, 2, 3, 2);
    b.printBoard();
  }

  for (int halfCount = weAreWhite ? 1 : 0; halfCount < 50; halfCount++) {
    move_t m;

    // Make the move.
    if (weAreWhite == (halfCount % 2 == 0)) {
      auto scoredMove = b.findMove(ply);
      m = scoredMove.second;
      cout << "iter: " << halfCount << "\tScore: " << scoredMove.first << endl;
    } else {
      // read move from input.
      string oldS, newS;
      while (true) {
        cout << "What move did they play?" << endl;
        cin >> oldS;
        cin >> newS;

        cout << "confirm [" << oldS << "] to [" << newS << "]" <<endl;
        string confirm;
        cin >> confirm; 
        if (confirm == "y" || confirm == "t") {
          break;
        }
      }
      m = make_tuple(oldS[1] - '1', oldS[0] - 'a',
                     newS[1] - '1', newS[0] - 'a',
                     1  moving , 0  removing );
    }

    cout << Board::moveNotation(m) <<
         "\t("   << (int)get<0>(m) << ", " << (int)get<1>(m) <<
         " to " << (int)get<2>(m) << ", " << (int)get<3>(m) <<
         "\tpiece: " <<(int) get<4>(m) << "\ttakes: " << (int)get<5>(m) << endl;

    // signals win.
    if (get<4>(m) == 0) {
      break;
    }


    b.makeMove(get<0>(m), get<1>(m), get<2>(m), get<3>(m));
    b.printBoard();
  }
*/
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

  string fenHeader  = null2Empty( evhttp_find_header(&uriParams, "fen") );
  string moveHeader = null2Empty( evhttp_find_header(&uriParams, "move") );

  cout << "\tparsed args:" << endl
       << "\t\tfen: \"" << fenHeader << "\"" << endl
       << "\t\tmove: \"" << moveHeader << "\"" << endl << endl;


  string moveReply;
  if (!fenHeader.empty()) {
    moveReply = repLoop(6, fenHeader, "");
  } else if (!moveHeader.empty()) {
    // assume we are playing the same game as before.
    moveReply = repLoop(6, "", moveHeader);
  } else {
    moveReply = "No fen or move param in args.";
  }


  cout << "return: \"" << moveReply << "\"" << endl << endl << endl;

  auto *outBuffer = evhttp_request_get_output_buffer(req);
  evbuffer_add_printf(outBuffer, moveReply.c_str());
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
