#include <algorithm>
#include <chrono>
#include <cassert>
#include <cstdio>
#include <iostream>
#include <iterator>
#include <set>
#include <vector>

#include "board.h"
#include "flags.h"

using namespace std;
using namespace board;

// The general idea is that these test tricky problems.

// Gloabl State variables
int testPlySize = 0;
int testNodeSize = 0;

int success = 0;
int missed = 0;

int countPly = 0;
long countNodes = 0;


const int K_NODES = 1000;
const map<string, int> predefinedPlySizes = {
  {"",        4}, // default is small.
  {"instant", 3},
  {"small",   4},
  {"medium",  4},
  {"large",   5},
};

const map<string, int> predefinedNodeSizes = {
  {"",        100  * K_NODES}, // default is small.
  {"instant", 10   * K_NODES},
  {"small",   100  * K_NODES},
  {"medium",  200  * K_NODES},
  {"large",  1000  * K_NODES},
};

vector<string> getBestMoves(Board &b, string epd);
bool verifyIsInLegal(Board &b, vector<string> moves);

void printStats(string setName) {
  int total = success + missed;
  cout << success << " out of (" << total << ") for set \"" << setName << "\"" << endl
       << "\t\tplys searched: " << countPly << endl
       << "\t\tnodes counted: " << countNodes << endl;
}


bool eval(string epd) {
  // Extended Position Description.

  // Step 1: Parse position
  // Step 2: Parse best moves.
  // Step 2a: check if we consider this a legal move.
  // Step 3: Test with depth / count / time.

  // TODO add avoid moves (am).

  // Step 1.
  // The epd's parsed here general start with "<FEN> bm Ke3; id set.012;".
  string fen = epd.substr(0, epd.find(" bm"));
  // They also tend to be missing move counts.
  fen += " 0 0";
  Board b(fen);

  // Step 2.
  vector<string> bestMoves = getBestMoves(b, epd);

  // Step 3.
  FindMoveStats stats;
  move_t move = get<1>(b.findMove(testPlySize, testNodeSize, &stats));
  string moveName = b.algebraicNotation_slow(move);

  countPly   += stats.plyR;
  countNodes += stats.nodes;

  bool found = find(bestMoves.begin(), bestMoves.end(), moveName) != bestMoves.end();
  if (found) {
    success += 1;
  } else {
    missed += 1;
  }

  if (FLAGS_verbosity >= 2) {
    cout << epd << endl;
    if (found) {
      cout << "Found: " << moveName << endl;
    } else {
      cout << "Missed (played " << moveName << " instead of " << bestMoves[0] << ")" << endl;
    }
  }

  bool infrequent = (success + missed) % 25 == 0;
  if (FLAGS_verbosity + infrequent >= 2) {
    cout << "\t(" << success << "/" << (success + missed) << ")" << endl;
  }
}

void evalWinAtChess0(void) {
  eval("2rr3k/pp3pp1/1nnqbN1p/3pN3/2pP4/2P3Q1/PPB4P/R4RK1 w - - bm Qg6; id \"WAC.001\";");
  eval("8/7p/5k2/5p2/p1p2P2/Pr1pPK2/1P1R3P/8 b - - bm Rxb2; id \"WAC.002\";");
  eval("5rk1/1ppb3p/p1pb4/6q1/3P1p1r/2P1R2P/PP1BQ1P1/5RKN w - - bm Rg3; id \"WAC.003\";");
  eval("r1bq2rk/pp3pbp/2p1p1pQ/7P/3P4/2PB1N2/PP3PPR/2KR4 w - - bm Qxh7+; id \"WAC.004\";");
  eval("5k2/6pp/p1qN4/1p1p4/3P4/2PKP2Q/PP3r2/3R4 b - - bm Qc4+; id \"WAC.005\";");
  eval("7k/p7/1R5K/6r1/6p1/6P1/8/8 w - - bm Rb7; id \"WAC.006\";");
  eval("rnbqkb1r/pppp1ppp/8/4P3/6n1/7P/PPPNPPP1/R1BQKBNR b KQkq - bm Ne3; id \"WAC.007\";");
  eval("r4q1k/p2bR1rp/2p2Q1N/5p2/5p2/2P5/PP3PPP/R5K1 w - - bm Rf7; id \"WAC.008\";");
  eval("3q1rk1/p4pp1/2pb3p/3p4/6Pr/1PNQ4/P1PB1PP1/4RRK1 b - - bm Bh2+; id \"WAC.009\";");
  eval("2br2k1/2q3rn/p2NppQ1/2p1P3/Pp5R/4P3/1P3PPP/3R2K1 w - - bm Rxh7; id \"WAC.010\";");
  eval("r1b1kb1r/3q1ppp/pBp1pn2/8/Np3P2/5B2/PPP3PP/R2Q1RK1 w kq - bm Bxc6; id \"WAC.011\";");
  eval("4k1r1/2p3r1/1pR1p3/3pP2p/3P2qP/P4N2/1PQ4P/5R1K b - - bm Qxf3+; id \"WAC.012\";");
  eval("5rk1/pp4p1/2n1p2p/2Npq3/2p5/6P1/P3P1BP/R4Q1K w - - bm Qxf8+; id \"WAC.013\";");
  eval("r2rb1k1/pp1q1p1p/2n1p1p1/2bp4/5P2/PP1BPR1Q/1BPN2PP/R5K1 w - - bm Qxh7+; id \"WAC.014\";");
  eval("1R6/1brk2p1/4p2p/p1P1Pp2/P7/6P1/1P4P1/2R3K1 w - - bm Rxb7; id \"WAC.015\";");
  eval("r4rk1/ppp2ppp/2n5/2bqp3/8/P2PB3/1PP1NPPP/R2Q1RK1 w - - bm Nc3; id \"WAC.016\";");
  eval("1k5r/pppbn1pp/4q1r1/1P3p2/2NPp3/1QP5/P4PPP/R1B1R1K1 w - - bm Ne5; id \"WAC.017\";");
  eval("R7/P4k2/8/8/8/8/r7/6K1 w - - bm Rh8; id \"WAC.018\";");
  eval("r1b2rk1/ppbn1ppp/4p3/1QP4q/3P4/N4N2/5PPP/R1B2RK1 w - - bm c6; id \"WAC.019\";");
  eval("r2qkb1r/1ppb1ppp/p7/4p3/P1Q1P3/2P5/5PPP/R1B2KNR b kq - bm Bb5; id \"WAC.020\";");
  eval("5rk1/1b3p1p/pp3p2/3n1N2/1P6/P1qB1PP1/3Q3P/4R1K1 w - - bm Qh6; id \"WAC.021\";");
  eval("r1bqk2r/ppp1nppp/4p3/n5N1/2BPp3/P1P5/2P2PPP/R1BQK2R w KQkq - bm Ba2 Nxf7; id \"WAC.022\";");
  eval("r3nrk1/2p2p1p/p1p1b1p1/2NpPq2/3R4/P1N1Q3/1PP2PPP/4R1K1 w - - bm g4; id \"WAC.023\";");
  eval("6k1/1b1nqpbp/pp4p1/5P2/1PN5/4Q3/P5PP/1B2B1K1 b - - bm Bd4; id \"WAC.024\";");
  eval("3R1rk1/8/5Qpp/2p5/2P1p1q1/P3P3/1P2PK2/8 b - - bm Qh4+; id \"WAC.025\";");
  eval("3r2k1/1p1b1pp1/pq5p/8/3NR3/2PQ3P/PP3PP1/6K1 b - - bm Bf5; id \"WAC.026\";");
  eval("7k/pp4np/2p3p1/3pN1q1/3P4/Q7/1r3rPP/2R2RK1 w - - bm Qf8+; id \"WAC.027\";");
  eval("1r1r2k1/4pp1p/2p1b1p1/p3R3/RqBP4/4P3/1PQ2PPP/6K1 b - - bm Qe1+; id \"WAC.028\";");
  eval("r2q2k1/pp1rbppp/4pn2/2P5/1P3B2/6P1/P3QPBP/1R3RK1 w - - bm c6; id \"WAC.029\";");
  eval("1r3r2/4q1kp/b1pp2p1/5p2/pPn1N3/6P1/P3PPBP/2QRR1K1 w - - bm Nxd6; id \"WAC.030\";");
  eval("rb3qk1/pQ3ppp/4p3/3P4/8/1P3N2/1P3PPP/3R2K1 w - - bm Qxa8 d6 dxe6 g3; id \"WAC.031\";");
  eval("6k1/p4p1p/1p3np1/2q5/4p3/4P1N1/PP3PPP/3Q2K1 w - - bm Qd8+; id \"WAC.032\";");
  eval("8/p1q2pkp/2Pr2p1/8/P3Q3/6P1/5P1P/2R3K1 w - - bm Qe5+ Qf4; id \"WAC.033\";");
  eval("7k/1b1r2p1/p6p/1p2qN2/3bP3/3Q4/P5PP/1B1R3K b - - bm Bg1; id \"WAC.034\";");
  eval("r3r2k/2R3pp/pp1q1p2/8/3P3R/7P/PP3PP1/3Q2K1 w - - bm Rxh7+; id \"WAC.035\";");
  eval("3r4/2p1rk2/1pQq1pp1/7p/1P1P4/P4P2/6PP/R1R3K1 b - - bm Re1+; id \"WAC.036\";");
  eval("2r5/2rk2pp/1pn1pb2/pN1p4/P2P4/1N2B3/nPR1KPPP/3R4 b - - bm Nxd4+; id \"WAC.037\";");
  eval("4k3/p4prp/1p6/2b5/8/2Q3P1/P2R1PKP/4q3 w - - bm Qd3 Rd8+; id \"WAC.038\";");
  eval("r1br2k1/pp2bppp/2nppn2/8/2P1PB2/2N2P2/PqN1B1PP/R2Q1R1K w - - bm Na4; id \"WAC.039\";");
  eval("3r1r1k/1p4pp/p4p2/8/1PQR4/6Pq/P3PP2/2R3K1 b - - bm Rc8; id \"WAC.040\";");
  eval("1k6/5RP1/1P6/1K6/6r1/8/8/8 w - - bm Ka5 Kc5 b7; id \"WAC.041\";");
  eval("r1b1r1k1/pp1n1pbp/1qp3p1/3p4/1B1P4/Q3PN2/PP2BPPP/R4RK1 w - - bm Ba5; id \"WAC.042\";");
  eval("r2q3k/p2P3p/1p3p2/3QP1r1/8/B7/P5PP/2R3K1 w - - bm Be7 Qxa8; id \"WAC.043\";");
  eval("3rb1k1/pq3pbp/4n1p1/3p4/2N5/2P2QB1/PP3PPP/1B1R2K1 b - - bm dxc4; id \"WAC.044\";");
  eval("7k/2p1b1pp/8/1p2P3/1P3r2/2P3Q1/1P5P/R4qBK b - - bm Qxa1; id \"WAC.045\";");
  eval("r1bqr1k1/pp1nb1p1/4p2p/3p1p2/3P4/P1N1PNP1/1PQ2PP1/3RKB1R w K - bm Nb5; id \"WAC.046\";");
  eval("r1b2rk1/pp2bppp/2n1pn2/q5B1/2BP4/2N2N2/PP2QPPP/2R2RK1 b - - bm Nxd4; id \"WAC.047\";");
  eval("1rbq1rk1/p1p1bppp/2p2n2/8/Q1BP4/2N5/PP3PPP/R1B2RK1 b - - bm Rb4; id \"WAC.048\";");
  eval("2b3k1/4rrpp/p2p4/2pP2RQ/1pP1Pp1N/1P3P1P/1q6/6RK w - - bm Qxh7+; id \"WAC.049\";");
  eval("k4r2/1R4pb/1pQp1n1p/3P4/5p1P/3P2P1/r1q1R2K/8 w - - bm Rxb6+; id \"WAC.050\";");
}

void evalWinAtChess50(void) {
  eval("r1bq1r2/pp4k1/4p2p/3pPp1Q/3N1R1P/2PB4/6P1/6K1 w - - bm Rg4+; id \"WAC.051\";");
  eval("r1k5/1p3q2/1Qpb4/3N1p2/5Pp1/3P2Pp/PPPK3P/4R3 w - - bm Re7 c4; id \"WAC.052\";");
  eval("6k1/6p1/p7/3Pn3/5p2/4rBqP/P4RP1/5QK1 b - - bm Re1; id \"WAC.053\";");
  eval("r3kr2/1pp4p/1p1p4/7q/4P1n1/2PP2Q1/PP4P1/R1BB2K1 b q - bm Qh1+; id \"WAC.054\";");
  eval("r3r1k1/pp1q1pp1/4b1p1/3p2B1/3Q1R2/8/PPP3PP/4R1K1 w - - bm Qxg7+; id \"WAC.055\";");
  eval("r1bqk2r/pppp1ppp/5n2/2b1n3/4P3/1BP3Q1/PP3PPP/RNB1K1NR b KQkq - bm Bxf2+; id \"WAC.056\";");
  eval("r3q1kr/ppp5/3p2pQ/8/3PP1b1/5R2/PPP3P1/5RK1 w - - bm Rf8+; id \"WAC.057\";");
  eval("8/8/2R5/1p2qp1k/1P2r3/2PQ2P1/5K2/8 w - - bm Qd1+; id \"WAC.058\";");
  eval("r1b2rk1/2p1qnbp/p1pp2p1/5p2/2PQP3/1PN2N1P/PB3PP1/3R1RK1 w - - bm Nd5; id \"WAC.059\";");
  eval("rn1qr1k1/1p2np2/2p3p1/8/1pPb4/7Q/PB1P1PP1/2KR1B1R w - - bm Qh8+; id \"WAC.060\";");
  eval("3qrbk1/ppp1r2n/3pP2p/3P4/2P4P/1P3Q2/PB6/R4R1K w - - bm Qf7+; id \"WAC.061\";");
  eval("6r1/3Pn1qk/p1p1P1rp/2Q2p2/2P5/1P4P1/P3R2P/5RK1 b - - bm Rxg3+; id \"WAC.062\";");
  eval("r1brnbk1/ppq2pp1/4p2p/4N3/3P4/P1PB1Q2/3B1PPP/R3R1K1 w - - bm Nxf7; id \"WAC.063\";");
  eval("8/6pp/3q1p2/3n1k2/1P6/3NQ2P/5PP1/6K1 w - - bm g4+; id \"WAC.064\";");
  eval("1r1r1qk1/p2n1p1p/bp1Pn1pQ/2pNp3/2P2P1N/1P5B/P6P/3R1RK1 w - - bm Ne7+; id \"WAC.065\";");
  eval("1k1r2r1/ppq5/1bp4p/3pQ3/8/2P2N2/PP4P1/R4R1K b - - bm Qxe5; id \"WAC.066\";");
  eval("3r2k1/p2q4/1p4p1/3rRp1p/5P1P/6PK/P3R3/3Q4 w - - bm Rxd5; id \"WAC.067\";");
  eval("6k1/5ppp/1q6/2b5/8/2R1pPP1/1P2Q2P/7K w - - bm Qxe3; id \"WAC.068\";");
  eval("2k5/pppr4/4R3/4Q3/2pp2q1/8/PPP2PPP/6K1 w - - bm f3 h3; id \"WAC.069\";");
  eval("2kr3r/pppq1ppp/3p1n2/bQ2p3/1n1PP3/1PN1BN1P/1PP2PP1/2KR3R b - - bm Na2+; id \"WAC.070\";");
  eval("2kr3r/pp1q1ppp/5n2/1Nb5/2Pp1B2/7Q/P4PPP/1R3RK1 w - - bm Nxa7+; id \"WAC.071\";");
  eval("r3r1k1/pp1n1ppp/2p5/4Pb2/2B2P2/B1P5/P5PP/R2R2K1 w - - bm e6; id \"WAC.072\";");
  eval("r1q3rk/1ppbb1p1/4Np1p/p3pP2/P3P3/2N4R/1PP1Q1PP/3R2K1 w - - bm Qd2; id \"WAC.073\";");
  eval("5r1k/pp4pp/2p5/2b1P3/4Pq2/1PB1p3/P3Q1PP/3N2K1 b - - bm Qf1+; id \"WAC.074\";");
  eval("r3r1k1/pppq1ppp/8/8/1Q4n1/7P/PPP2PP1/RNB1R1K1 b - - bm Qd6; id \"WAC.075\";");
  eval("r1b1qrk1/2p2ppp/pb1pnn2/1p2pNB1/3PP3/1BP5/PP2QPPP/RN1R2K1 w - - bm Bxf6; id \"WAC.076\";");
  eval("3r2k1/ppp2ppp/6q1/b4n2/3nQB2/2p5/P4PPP/RN3RK1 b - - bm Ng3; id \"WAC.077\";");
  eval("r2q3r/ppp2k2/4nbp1/5Q1p/2P1NB2/8/PP3P1P/3RR1K1 w - - bm Ng5+; id \"WAC.078\";");
  eval("r3k2r/pbp2pp1/3b1n2/1p6/3P3p/1B2N1Pq/PP1PQP1P/R1B2RK1 b kq - bm Qxh2+; id \"WAC.079\";");
  eval("r4rk1/p1B1bpp1/1p2pn1p/8/2PP4/3B1P2/qP2QP1P/3R1RK1 w - - bm Ra1; id \"WAC.080\";");
  eval("r4rk1/1bR1bppp/4pn2/1p2N3/1P6/P3P3/4BPPP/3R2K1 b - - bm Bd6; id \"WAC.081\";");
  eval("3rr1k1/pp3pp1/4b3/8/2P1B2R/6QP/P3q1P1/5R1K w - - bm Bh7+; id \"WAC.082\";");
  eval("3rr1k1/ppqbRppp/2p5/8/3Q1n2/2P3N1/PPB2PPP/3R2K1 w - - bm Qxd7; id \"WAC.083\";");
  eval("r2q1r1k/2p1b1pp/p1n5/1p1Q1bN1/4n3/1BP1B3/PP3PPP/R4RK1 w - - bm Qg8+; id \"WAC.084\";");
  eval("kr2R3/p4r2/2pq4/2N2p1p/3P2p1/Q5P1/5P1P/5BK1 w - - bm Na6; id \"WAC.085\";");
  eval("8/p7/1ppk1n2/5ppp/P1PP4/2P1K1P1/5N1P/8 b - - bm Ng4+; id \"WAC.086\";");
  eval("8/p3k1p1/4r3/2ppNpp1/PP1P4/2P3KP/5P2/8 b - - bm Rxe5; id \"WAC.087\";");
  eval("r6k/p1Q4p/2p1b1rq/4p3/B3P3/4P3/PPP3P1/4RRK1 b - - bm Rxg2+; id \"WAC.088\";");
  eval("1r3b1k/p4rpp/4pp2/3q4/2ppbPPQ/6RK/PP5P/2B1NR2 b - - bm g5; id \"WAC.089\";");
  eval("3qrrk1/1pp2pp1/1p2bn1p/5N2/2P5/P1P3B1/1P4PP/2Q1RRK1 w - - bm Nxg7; id \"WAC.090\";");
  eval("2qr2k1/4b1p1/2p2p1p/1pP1p3/p2nP3/PbQNB1PP/1P3PK1/4RB2 b - - bm Be6; id \"WAC.091\";");
  eval("r4rk1/1p2ppbp/p2pbnp1/q7/3BPPP1/2N2B2/PPP4P/R2Q1RK1 b - - bm Bxg4; id \"WAC.092\";");
  eval("r1b1k1nr/pp3pQp/4pq2/3pn3/8/P1P5/2P2PPP/R1B1KBNR w KQkq - bm Bh6; id \"WAC.093\";");
  eval("8/k7/p7/3Qp2P/n1P5/3KP3/1q6/8 b - - bm e4+; id \"WAC.094\";");
  eval("2r5/1r6/4pNpk/3pP1qp/8/2P1QP2/5PK1/R7 w - - bm Ng4+; id \"WAC.095\";");
  eval("r1b4k/ppp2Bb1/6Pp/3pP3/1qnP1p1Q/8/PPP3P1/1K1R3R w - - bm Qd8+ b3; id \"WAC.096\";");
  eval("6k1/5p2/p5np/4B3/3P4/1PP1q3/P3r1QP/6RK w - - bm Qa8+; id \"WAC.097\";");
  eval("1r3rk1/5pb1/p2p2p1/Q1n1q2p/1NP1P3/3p1P1B/PP1R3P/1K2R3 b - - bm Nxe4; id \"WAC.098\";");
  eval("r1bq1r1k/1pp1Np1p/p2p2pQ/4R3/n7/8/PPPP1PPP/R1B3K1 w - - bm Rh5; id \"WAC.099\";");
  eval("8/k1b5/P4p2/1Pp2p1p/K1P2P1P/8/3B4/8 w - - bm Be3 b6+; id \"WAC.100\";");
}


void evalWinAtChess100(void) {
  eval("5rk1/p5pp/8/8/2Pbp3/1P4P1/7P/4RN1K b - - bm Bc3; id \"WAC.101\";");
  eval("2Q2n2/2R4p/1p1qpp1k/8/3P3P/3B2P1/5PK1/r7 w - - bm Qxf8+; id \"WAC.102\";");
  eval("6k1/2pb1r1p/3p1PpQ/p1nPp3/1q2P3/2N2P2/PrB5/2K3RR w - - bm Qxg6+; id \"WAC.103\";");
  eval("b4r1k/pq2rp2/1p1bpn1p/3PN2n/2P2P2/P2B3K/1B2Q2N/3R2R1 w - - bm Qxh5; id \"WAC.104\";");
  eval("r2r2k1/pb3ppp/1p1bp3/7q/3n2nP/PP1B2P1/1B1N1P2/RQ2NRK1 b - - bm Bxg3 Qxh4; id \"WAC.105\";");
  eval("4rrk1/pppb4/7p/3P2pq/3Qn3/P5P1/1PP4P/R3RNNK b - - bm Nf2+; id \"WAC.106\";");
  eval("5n2/pRrk2p1/P4p1p/4p3/3N4/5P2/6PP/6K1 w - - bm Nb5; id \"WAC.107\";");
  eval("r5k1/1q4pp/2p5/p1Q5/2P5/5R2/4RKPP/r7 w - - bm Qe5; id \"WAC.108\";");
  eval("rn2k1nr/pbp2ppp/3q4/1p2N3/2p5/QP6/PB1PPPPP/R3KB1R b KQkq - bm c3; id \"WAC.109\";");
  eval("2kr4/bp3p2/p2p2b1/P7/2q5/1N4B1/1PPQ2P1/2KR4 b - - bm Be3; id \"WAC.110\";");
  eval("6k1/p5p1/5p2/2P2Q2/3pN2p/3PbK1P/7P/6q1 b - - bm Qf1+; id \"WAC.111\";");
  eval("r4kr1/ppp5/4bq1b/7B/2PR1Q1p/2N3P1/PP3P1P/2K1R3 w - - bm Rxe6; id \"WAC.112\";");
  eval("rnbqkb1r/1p3ppp/5N2/1p2p1B1/2P5/8/PP2PPPP/R2QKB1R b KQkq - bm Qxf6; id \"WAC.113\";");
  eval("r1b1rnk1/1p4pp/p1p2p2/3pN2n/3P1PPq/2NBPR1P/PPQ5/2R3K1 w - - bm Bxh7+; id \"WAC.114\";");
  eval("4N2k/5rpp/1Q6/p3q3/8/P5P1/1P3P1P/5K2 w - - bm Nd6; id \"WAC.115\";");
  eval("r2r2k1/2p2ppp/p7/1p2P1n1/P6q/5P2/1PB1QP1P/R5RK b - - bm Rd2; id \"WAC.116\";");
  eval("3r1rk1/q4ppp/p1Rnp3/8/1p6/1N3P2/PP3QPP/3R2K1 b - - bm Ne4; id \"WAC.117\";");
  eval("r5k1/pb2rpp1/1p6/2p4q/5R2/2PB2Q1/P1P3PP/5R1K w - - bm Rh4; id \"WAC.118\";");
  eval("r2qr1k1/p1p2ppp/2p5/2b5/4nPQ1/3B4/PPP3PP/R1B2R1K b - - bm Qxd3; id \"WAC.119\";");
  eval("r4rk1/1bn2qnp/3p1B1Q/p2P1pP1/1pp5/5N1P/PPB2P2/2KR3R w - - bm Rhg1 g6; id \"WAC.120\";");
  eval("6k1/5p1p/2bP2pb/4p3/2P5/1p1pNPPP/1P1Q1BK1/1q6 b - - bm Bxf3+; id \"WAC.121\";");
  eval("1k6/ppp4p/1n2pq2/1N2Rb2/2P2Q2/8/P4KPP/3r1B2 b - - bm Rxf1+; id \"WAC.122\";");
  eval("6k1/1b2rp2/1p4p1/3P4/PQ4P1/2N2q2/5P2/3R2K1 b - - bm Bxd5 Rc7 Re6; id \"WAC.123\";");
  eval("6k1/3r4/2R5/P5P1/1P4p1/8/4rB2/6K1 b - - bm g3; id \"WAC.124\";");
  eval("r1bqr1k1/pp3ppp/1bp5/3n4/3B4/2N2P1P/PPP1B1P1/R2Q1RK1 b - - bm Bxd4+; id \"WAC.125\";");
  eval("r5r1/pQ5p/1qp2R2/2k1p3/4P3/2PP4/P1P3PP/6K1 w - - bm Rxc6+; id \"WAC.126\";");
  eval("2k4r/1pr1n3/p1p1q2p/5pp1/3P1P2/P1P1P3/1R2Q1PP/1RB3K1 w - - bm Rxb7; id \"WAC.127\";");
  eval("6rk/1pp2Qrp/3p1B2/1pb1p2R/3n1q2/3P4/PPP3PP/R6K w - - bm Qg6; id \"WAC.128\";");
  eval("3r1r1k/1b2b1p1/1p5p/2p1Pp2/q1B2P2/4P2P/1BR1Q2K/6R1 b - - bm Bf3; id \"WAC.129\";");
  eval("6k1/1pp3q1/5r2/1PPp4/3P1pP1/3Qn2P/3B4/4R1K1 b - - bm Qh6 Qh8; id \"WAC.130\";");
  eval("2rq1bk1/p4p1p/1p4p1/3b4/3B1Q2/8/P4PpP/3RR1K1 w - - bm Re8; id \"WAC.131\";");
  eval("4r1k1/5bpp/2p5/3pr3/8/1B3pPq/PPR2P2/2R2QK1 b - - bm Re1; id \"WAC.132\";");
  eval("r1b1k2r/1pp1q2p/p1n3p1/3QPp2/8/1BP3B1/P5PP/3R1RK1 w kq - bm Bh4; id \"WAC.133\";");
  eval("3r2k1/p6p/2Q3p1/4q3/2P1p3/P3Pb2/1P3P1P/2K2BR1 b - - bm Rd1+; id \"WAC.134\";");
  eval("3r1r1k/N2qn1pp/1p2np2/2p5/2Q1P2N/3P4/PP4PP/3R1RK1 b - - bm Nd4; id \"WAC.135\";");
  eval("6kr/1q2r1p1/1p2N1Q1/5p2/1P1p4/6R1/7P/2R3K1 w - - bm Rc8+; id \"WAC.136\";");
  eval("3b1rk1/1bq3pp/5pn1/1p2rN2/2p1p3/2P1B2Q/1PB2PPP/R2R2K1 w - - bm Rd7; id \"WAC.137\";");
  eval("r1bq3r/ppppR1p1/5n1k/3P4/6pP/3Q4/PP1N1PP1/5K1R w - - bm h5; id \"WAC.138\";");
  eval("rnb3kr/ppp2ppp/1b6/3q4/3pN3/Q4N2/PPP2KPP/R1B1R3 w - - bm Nf6+; id \"WAC.139\";");
  eval("r2b1rk1/pq4p1/4ppQP/3pB1p1/3P4/2R5/PP3PP1/5RK1 w - - bm Bc7 Rc7; id \"WAC.140\";");
  eval("4r1k1/p1qr1p2/2pb1Bp1/1p5p/3P1n1R/1B3P2/PP3PK1/2Q4R w - - bm Qxf4; id \"WAC.141\";");
  eval("r2q3n/ppp2pk1/3p4/5Pr1/2NP1Qp1/2P2pP1/PP3K2/4R2R w - - bm Re8 f6+; id \"WAC.142\";");
  eval("5b2/pp2r1pk/2pp1pRp/4rP1N/2P1P3/1P4QP/P3q1P1/5R1K w - - bm Rxh6+; id \"WAC.143\";");
  eval("r2q1rk1/pp3ppp/2p2b2/8/B2pPPb1/7P/PPP1N1P1/R2Q1RK1 b - - bm d3; id \"WAC.144\";");
  eval("r1bq4/1p4kp/3p1n2/p4pB1/2pQ4/8/1P4PP/4RRK1 w - - bm Re8; id \"WAC.145\";");
  eval("8/8/2Kp4/3P1B2/2P2k2/5p2/8/8 w - - bm Bc8 Bd3 Bh3; id \"WAC.146\";");
  eval("r2r2k1/ppqbppbp/2n2np1/2pp4/6P1/1P1PPNNP/PBP2PB1/R2QK2R b KQ - bm Nxg4; id \"WAC.147\";");
  eval("2r1k3/6pr/p1nBP3/1p3p1p/2q5/2P5/P1R4P/K2Q2R1 w - - bm Rxg7; id \"WAC.148\";");
  eval("6k1/6p1/2p4p/4Pp2/4b1qP/2Br4/1P2RQPK/8 b - - bm Bxg2; id \"WAC.149\";");
  eval("r3r1k1/5p2/pQ1b2pB/1p6/4p3/6P1/Pq2BP1P/2R3K1 b - - bm Ba3 Be5 Bf8 e3; c0 \"All win but e3 is best.\"; id \"WAC.150\";");
  eval("8/3b2kp/4p1p1/pr1n4/N1N4P/1P4P1/1K3P2/3R4 w - - bm Nc3; id \"WAC.151\";");
  eval("1br2rk1/1pqb1ppp/p3pn2/8/1P6/P1N1PN1P/1B3PP1/1QRR2K1 w - - bm Ne4; id \"WAC.152\";");
  eval("2r3k1/q4ppp/p3p3/pnNp4/2rP4/2P2P2/4R1PP/2R1Q1K1 b - - bm Nxd4; id \"WAC.153\";");
  eval("r1b2rk1/2p2ppp/p7/1p6/3P3q/1BP3bP/PP3QP1/RNB1R1K1 w - - bm Qxf7+; id \"WAC.154\";");
  eval("5bk1/1rQ4p/5pp1/2pP4/3n1PP1/7P/1q3BB1/4R1K1 w - - bm d6; id \"WAC.155\";");
  eval("r1b1qN1k/1pp3p1/p2p3n/4p1B1/8/1BP4Q/PP3KPP/8 w - - bm Qxh6+; id \"WAC.156\";");
  eval("5rk1/p4ppp/2p1b3/3Nq3/4P1n1/1p1B2QP/1PPr2P1/1K2R2R w - - bm Ne7+; id \"WAC.157\";");
  eval("5rk1/n1p1R1bp/p2p4/1qpP1QB1/7P/2P3P1/PP3P2/6K1 w - - bm Rxg7+; id \"WAC.158\";");
  eval("r1b2r2/5P1p/ppn3pk/2p1p1Nq/1bP1PQ2/3P4/PB4BP/1R3RK1 w - - bm Ne6+; id \"WAC.159\";");
  eval("qn1kr2r/1pRbb3/pP5p/P2pP1pP/3N1pQ1/3B4/3B1PP1/R5K1 w - - bm Qxd7+; id \"WAC.160\";");
  eval("3r3k/3r1P1p/pp1Nn3/2pp4/7Q/6R1/Pq4PP/5RK1 w - - bm Qxd8+; id \"WAC.161\";");
  eval("r3kbnr/p4ppp/2p1p3/8/Q1B3b1/2N1B3/PP3PqP/R3K2R w KQkq - bm Bd5; id \"WAC.162\";");
  eval("5rk1/2p4p/2p4r/3P4/4p1b1/1Q2NqPp/PP3P1K/R4R2 b - - bm Qg2+; id \"WAC.163\";");
  eval("8/6pp/4p3/1p1n4/1NbkN1P1/P4P1P/1PR3K1/r7 w - - bm Rxc4+; id \"WAC.164\";");
  eval("1r5k/p1p3pp/8/8/4p3/P1P1R3/1P1Q1qr1/2KR4 w - - bm Re2; id \"WAC.165\";");
  eval("r3r1k1/5pp1/p1p4p/2Pp4/8/q1NQP1BP/5PP1/4K2R b K - bm d4; id \"WAC.166\";");
  eval("7Q/ppp2q2/3p2k1/P2Ppr1N/1PP5/7R/5rP1/6K1 b - - bm Rxg2+; id \"WAC.167\";");
  eval("r3k2r/pb1q1p2/8/2p1pP2/4p1p1/B1P1Q1P1/P1P3K1/R4R2 b kq - bm Qd2+; id \"WAC.168\";");
  eval("5rk1/1pp3bp/3p2p1/2PPp3/1P2P3/2Q1B3/4q1PP/R5K1 b - - bm Bh6; id \"WAC.169\";");
  eval("5r1k/6Rp/1p2p3/p2pBp2/1qnP4/4P3/Q4PPP/6K1 w - - bm Qxc4; id \"WAC.170\";");
  eval("2rq4/1b2b1kp/p3p1p1/1p1nNp2/7P/1B2B1Q1/PP3PP1/3R2K1 w - - bm Bh6+; id \"WAC.171\";");
  eval("5r1k/p5pp/8/1P1pq3/P1p2nR1/Q7/5BPP/6K1 b - - bm Qe1+; id \"WAC.172\";");
  eval("2r1b3/1pp1qrk1/p1n1P1p1/7R/2B1p3/4Q1P1/PP3PP1/3R2K1 w - - bm Qh6+; id \"WAC.173\";");
  eval("2r2rk1/6p1/p3pq1p/1p1b1p2/3P1n2/PP3N2/3N1PPP/1Q2RR1K b - - bm Nxg2; id \"WAC.174\";");
  eval("r5k1/pppb3p/2np1n2/8/3PqNpP/3Q2P1/PPP5/R4RK1 w - - bm Nh5; id \"WAC.175\";");
  eval("r1bq3r/ppp2pk1/3p1pp1/8/2BbPQ2/2NP2P1/PPP4P/R4R1K b - - bm Rxh2+; id \"WAC.176\";");
  eval("r1b3r1/4qk2/1nn1p1p1/3pPp1P/p4P2/1p3BQN/PKPBN3/3R3R b - - bm Qa3+; id \"WAC.177\";");
  eval("3r2k1/p1rn1p1p/1p2pp2/6q1/3PQNP1/5P2/P1P4R/R5K1 w - - bm Nxe6; id \"WAC.178\";");
  eval("r1b2r1k/pp4pp/3p4/3B4/8/1QN3Pn/PP3q1P/R3R2K b - - bm Qg1+; id \"WAC.179\";");
  eval("r1q2rk1/p3bppb/3p1n1p/2nPp3/1p2P1P1/6NP/PP2QPB1/R1BNK2R b KQ - bm Nxd5; id \"WAC.180\";");
  eval("r3k2r/2p2p2/p2p1n2/1p2p3/4P2p/1PPPPp1q/1P5P/R1N2QRK b kq - bm Ng4; id \"WAC.181\";");
  eval("r1b2rk1/ppqn1p1p/2n1p1p1/2b3N1/2N5/PP1BP3/1B3PPP/R2QK2R w KQ - bm Qh5; id \"WAC.182\";");
  eval("1r2k1r1/5p2/b3p3/1p2b1B1/3p3P/3B4/PP2KP2/2R3R1 w - - bm Bf6; id \"WAC.183\";");
  eval("4kn2/r4p1r/p3bQ2/q1nNP1Np/1p5P/8/PPP3P1/2KR3R w - - bm Qe7+; id \"WAC.184\";");
  eval("1r1rb1k1/2p3pp/p2q1p2/3PpP1Q/Pp1bP2N/1B5R/1P4PP/2B4K w - - bm Qxh7+; id \"WAC.185\";");
  eval("r5r1/p1q2p1k/1p1R2pB/3pP3/6bQ/2p5/P1P1NPPP/6K1 w - - bm Bf8+; id \"WAC.186\";");
  eval("6k1/5p2/p3p3/1p3qp1/2p1Qn2/2P1R3/PP1r1PPP/4R1K1 b - - bm Nh3+; id \"WAC.187\";");
  eval("3RNbk1/pp3p2/4rQpp/8/1qr5/7P/P4P2/3R2K1 w - - bm Qg7+; id \"WAC.188\";");
  eval("3r1k2/1ppPR1n1/p2p1rP1/3P3p/4Rp1N/5K2/P1P2P2/8 w - - bm Re8+; id \"WAC.189\";");
  eval("8/p2b2kp/1q1p2p1/1P1Pp3/4P3/3B2P1/P2Q3P/2Nn3K b - - bm Bh3; id \"WAC.190\";");
  eval("2r1Rn1k/1p1q2pp/p7/5p2/3P4/1B4P1/P1P1QP1P/6K1 w - - bm Qc4; id \"WAC.191\";");
  eval("r3k3/ppp2Npp/4Bn2/2b5/1n1pp3/N4P2/PPP3qP/R2QKR2 b Qq - bm Nd3+; id \"WAC.192\";");
  eval("5bk1/p4ppp/Qp6/4B3/1P6/Pq2P1P1/2rr1P1P/R4RK1 b - - bm Qxe3; id \"WAC.193\";");
  eval("5rk1/ppq2ppp/2p5/4bN2/4P3/6Q1/PPP2PPP/3R2K1 w - - bm Nh6+; id \"WAC.194\";");
  eval("3r1rk1/1p3p2/p3pnnp/2p3p1/2P2q2/1P5P/PB2QPPN/3RR1K1 w - - bm g3; id \"WAC.195\";");
  eval("rr4k1/p1pq2pp/Q1n1pn2/2bpp3/4P3/2PP1NN1/PP3PPP/R1B1K2R b KQ - bm Nb4; id \"WAC.196\";");
  eval("7k/1p4p1/7p/3P1n2/4Q3/2P2P2/PP3qRP/7K b - - bm Qf1+; id \"WAC.197\";");
  eval("2br2k1/ppp2p1p/4p1p1/4P2q/2P1Bn2/2Q5/PP3P1P/4R1RK b - - bm Rd3; id \"WAC.198\";");
  eval("r1br2k1/pp2nppp/2n5/1B1q4/Q7/4BN2/PP3PPP/2R2RK1 w - - bm Bxc6 Rcd1 Rfd1; id \"WAC.199\";");
  eval("2rqrn1k/pb4pp/1p2pp2/n2P4/2P3N1/P2B2Q1/1B3PPP/2R1R1K1 w - - bm Bxf6; id \"WAC.200\";");
}


void evalWinAtChess200(void) {
  eval("2b2r1k/4q2p/3p2pQ/2pBp3/8/6P1/1PP2P1P/R5K1 w - - bm Ra7; id \"WAC.201\";");
  eval("QR2rq1k/2p3p1/3p1pPp/8/4P3/8/P1r3PP/1R4K1 b - - bm Rxa2; id \"WAC.202\";");
  eval("r4rk1/5ppp/p3q1n1/2p2NQ1/4n3/P3P3/1B3PPP/1R3RK1 w - - bm Qh6; id \"WAC.203\";");
  eval("r1b1qrk1/1p3ppp/p1p5/3Nb3/5N2/P7/1P4PQ/K1R1R3 w - - bm Rxe5; id \"WAC.204\";");
  eval("r3rnk1/1pq2bb1/p4p2/3p1Pp1/3B2P1/1NP4R/P1PQB3/2K4R w - - bm Qxg5; id \"WAC.205\";");
  eval("1Qq5/2P1p1kp/3r1pp1/8/8/7P/p4PP1/2R3K1 b - - bm Rc6; id \"WAC.206\";");
  eval("r1bq2kr/p1pp1ppp/1pn1p3/4P3/2Pb2Q1/BR6/P4PPP/3K1BNR w - - bm Qxg7+; id \"WAC.207\";");
  eval("3r1bk1/ppq3pp/2p5/2P2Q1B/8/1P4P1/P6P/5RK1 w - - bm Bf7+; id \"WAC.208\";");
  eval("4kb1r/2q2p2/r2p4/pppBn1B1/P6P/6Q1/1PP5/2KRR3 w k - bm Rxe5+; id \"WAC.209\";");
  eval("3r1rk1/pp1q1ppp/3pn3/2pN4/5PP1/P5PQ/1PP1B3/1K1R4 w - - bm Rh1; id \"WAC.210\";");
  eval("r1bqrk2/pp1n1n1p/3p1p2/P1pP1P1Q/2PpP1NP/6R1/2PB4/4RBK1 w - - bm Qxf7+; id \"WAC.211\";");
  eval("rn1qr2Q/pbppk1p1/1p2pb2/4N3/3P4/2N5/PPP3PP/R4RK1 w - - bm Qxg7+; id \"WAC.212\";");
  eval("3r1r1k/1b4pp/ppn1p3/4Pp1R/Pn5P/3P4/4QP2/1qB1NKR1 w - - bm Rxh7+; id \"WAC.213\";");
  eval("r2r2k1/1p2qpp1/1np1p1p1/p3N3/2PPN3/bP5R/4QPPP/4R1K1 w - - bm Ng5; id \"WAC.214\";");
  eval("3r2k1/pb1q1pp1/1p2pb1p/8/3N4/P2QB3/1P3PPP/1Br1R1K1 w - - bm Qh7+; id \"WAC.215\";");
  eval("r2qr1k1/1b1nbppp/p3pn2/1p1pN3/3P1B2/2PB1N2/PP2QPPP/R4RK1 w - - bm Nxf7 a4; id \"WAC.216\";");
  eval("r3kb1r/1pp3p1/p3bp1p/5q2/3QN3/1P6/PBP3P1/3RR1K1 w kq - bm Qd7+; id \"WAC.217\";");
  eval("6k1/pp5p/2p3q1/6BP/2nPr1Q1/8/PP3R1K/8 w - - bm Bh6; id \"WAC.218\";");
  eval("7k/p4q1p/1pb5/2p5/4B2Q/2P1B3/P6P/7K b - - bm Qf1+; id \"WAC.219\";");
  eval("3rr1k1/ppp2ppp/8/5Q2/4n3/1B5R/PPP1qPP1/5RK1 b - - bm Qxf1+; id \"WAC.220\";");
  eval("r3k3/P5bp/2N1bp2/4p3/2p5/6NP/1PP2PP1/3R2K1 w q - bm Rd8+; id \"WAC.221\";");
  eval("2r1r2k/1q3ppp/p2Rp3/2p1P3/6QB/p3P3/bP3PPP/3R2K1 w - - bm Bf6; id \"WAC.222\";");
  eval("r1bqk2r/pp3ppp/5n2/8/1b1npB2/2N5/PP1Q2PP/1K2RBNR w kq - bm Nxe4; id \"WAC.223\";");
  eval("5rk1/p1q3pp/1p1r4/2p1pp1Q/1PPn1P2/3B3P/P2R2P1/3R2K1 b - - bm Rh6 e4; id \"WAC.224\";");
  eval("4R3/4q1kp/6p1/1Q3b2/1P1b1P2/6KP/8/8 b - - bm Qh4+; id \"WAC.225\";");
  eval("2b2rk1/p1p4p/2p1p1p1/br2N1Q1/1p2q3/8/PB3PPP/3R1RK1 w - - bm Nf7; id \"WAC.226\";");
  eval("2k1rb1r/ppp3pp/2np1q2/5b2/2B2P2/2P1BQ2/PP1N1P1P/2KR3R b - - bm d5; id \"WAC.227\";");
  eval("r4rk1/1bq1bp1p/4p1p1/p2p4/3BnP2/1N1B3R/PPP3PP/R2Q2K1 w - - bm Bxe4; id \"WAC.228\";");
  eval("8/8/8/1p5r/p1p1k1pN/P2pBpP1/1P1K1P2/8 b - - bm Rxh4 b4; id \"WAC.229\";");
  eval("2b5/1r6/2kBp1p1/p2pP1P1/2pP4/1pP3K1/1R3P2/8 b - - bm Rb4; id \"WAC.230\";");
  eval("r4rk1/1b1nqp1p/p5p1/1p2PQ2/2p5/5N2/PP3PPP/R1BR2K1 w - - bm Bg5; id \"WAC.231\";");
  eval("1R2rq1k/2p3p1/Q2p1pPp/8/4P3/8/P1r3PP/1R4K1 w - - bm Qb5 Rxe8; id \"WAC.232\";");
  eval("5rk1/p1p2r1p/2pp2p1/4p3/PPPnP3/3Pq1P1/1Q1R1R1P/4NK2 b - - bm Nb3; id \"WAC.233\";");
  eval("2kr1r2/p6p/5Pp1/2p5/1qp2Q1P/7R/PP6/1KR5 w - - bm Rb3; id \"WAC.234\";");
  eval("5r2/1p1RRrk1/4Qq1p/1PP3p1/8/4B3/1b3P1P/6K1 w - - bm Qe4 Qxf7+ Rxf7+; id \"WAC.235\";");
  eval("1R6/p5pk/4p2p/4P3/8/2r3qP/P3R1b1/4Q1K1 b - - bm Rc1; id \"WAC.236\";");
  eval("r5k1/pQp2qpp/8/4pbN1/3P4/6P1/PPr4P/1K1R3R b - - bm Rc1+; id \"WAC.237\";");
  eval("1k1r4/pp1r1pp1/4n1p1/2R5/2Pp1qP1/3P2QP/P4PB1/1R4K1 w - - bm Bxb7; id \"WAC.238\";");
  eval("8/6k1/5pp1/Q6p/5P2/6PK/P4q1P/8 b - - bm Qf1+; id \"WAC.239\";");
  eval("2b4k/p1b2p2/2p2q2/3p1PNp/3P2R1/3B4/P1Q2PKP/4r3 w - - bm Qxc6; id \"WAC.240\";");
  eval("2rq1rk1/pp3ppp/2n2b2/4NR2/3P4/PB5Q/1P4PP/3R2K1 w - - bm Qxh7+; id \"WAC.241\";");
  eval("r1b1r1k1/pp1nqp2/2p1p1pp/8/4N3/P1Q1P3/1P3PPP/1BRR2K1 w - - bm Rxd7; id \"WAC.242\";");
  eval("1r3r1k/3p4/1p1Nn1R1/4Pp1q/pP3P1p/P7/5Q1P/6RK w - - bm Qe2; id \"WAC.243\";");
  eval("r6r/pp3ppp/3k1b2/2pb4/B4Pq1/2P1Q3/P5PP/1RBR2K1 w - - bm Qxc5+; id \"WAC.244\";");
  eval("4rrn1/ppq3bk/3pPnpp/2p5/2PB4/2NQ1RPB/PP5P/5R1K w - - bm Qxg6+; id \"WAC.245\";");
  eval("6R1/4qp1p/ppr1n1pk/8/1P2P1QP/6N1/P4PP1/6K1 w - - bm Qh5+; id \"WAC.246\";");
  eval("2k1r3/1p2Bq2/p2Qp3/Pb1p1p1P/2pP1P2/2P5/2P2KP1/1R6 w - - bm Rxb5; id \"WAC.247\";");
  eval("5r1k/1p4pp/3q4/3Pp1R1/8/8/PP4PP/4Q1K1 b - - bm Qc5+; id \"WAC.248\";");
  eval("r4rk1/pbq2pp1/1ppbpn1p/8/2PP4/1P1Q1N2/PBB2PPP/R3R1K1 w - - bm c5 d5; id \"WAC.249\";");
  eval("1b5k/7P/p1p2np1/2P2p2/PP3P2/4RQ1R/q2r3P/6K1 w - - bm Re8+; id \"WAC.250\";");
  eval("k7/p4p2/P1q1b1p1/3p3p/3Q4/7P/5PP1/1R4K1 w - - bm Qe5 Qf4; id \"WAC.251\";");
  eval("1rb1r1k1/p1p2ppp/5n2/2pP4/5P2/2QB4/qNP3PP/2KRB2R b - - bm Bg4 Re2; c0 \"Bg4 wins, but Re2 is far better.\"; id \"WAC.252\";");
  eval("k5r1/p4b2/2P5/5p2/3P1P2/4QBrq/P5P1/4R1K1 w - - bm Qe8+; id \"WAC.253\";");
  eval("r6k/pp3p1p/2p1bp1q/b3p3/4Pnr1/2PP2NP/PP1Q1PPN/R2B2RK b - - bm Nxh3; id \"WAC.254\";");
  eval("3r3r/p4pk1/5Rp1/3q4/1p1P2RQ/5N2/P1P4P/2b4K w - - bm Rfxg6+; id \"WAC.255\";");
  eval("3r1rk1/1pb1qp1p/2p3p1/p7/P2Np2R/1P5P/1BP2PP1/3Q1BK1 w - - bm Nf5; id \"WAC.256\";");
  eval("4r1k1/pq3p1p/2p1r1p1/2Q1p3/3nN1P1/1P6/P1P2P1P/3RR1K1 w - - bm Rxd4; id \"WAC.257\";");
  eval("r3brkn/1p5p/2p2Ppq/2Pp3B/3Pp2Q/4P1R1/6PP/5R1K w - - bm Bxg6; id \"WAC.258\";");
  eval("r1bq1rk1/ppp2ppp/2np4/2bN1PN1/2B1P3/3p4/PPP2nPP/R1BQ1K1R w - - bm Qh5; id \"WAC.259\";");
  eval("2r2b1r/p1Nk2pp/3p1p2/N2Qn3/4P3/q6P/P4PP1/1R3K1R w - - bm Qe6+; id \"WAC.260\";");
  eval("r5k1/1bp3pp/p2p4/1p6/5p2/1PBP1nqP/1PP3Q1/R4R1K b - - bm Nd4; id \"WAC.261\";");
  eval("6k1/p1B1b2p/2b3r1/2p5/4p3/1PP1N1Pq/P2R1P2/3Q2K1 b - - bm Rh6; id \"WAC.262\";");
  eval("rnbqr2k/pppp1Qpp/8/b2NN3/2B1n3/8/PPPP1PPP/R1B1K2R w KQ - bm Qg8+; id \"WAC.263\";");
  eval("r2r2k1/1R2qp2/p5pp/2P5/b1PN1b2/P7/1Q3PPP/1B1R2K1 b - - bm Qe5 Rab8; id \"WAC.264\";");
  eval("2r1k2r/2pn1pp1/1p3n1p/p3PP2/4q2B/P1P5/2Q1N1PP/R4RK1 w k - bm exf6; id \"WAC.265\";");
  eval("r3q2r/2p1k1p1/p5p1/1p2Nb2/1P2nB2/P7/2PNQbPP/R2R3K b - - bm Rxh2+; id \"WAC.266\";");
  eval("2r1kb1r/pp3ppp/2n1b3/1q1N2B1/1P2Q3/8/P4PPP/3RK1NR w Kk - bm Nc7+; id \"WAC.267\";");
  eval("2r3kr/ppp2n1p/7B/5q1N/1bp5/2Pp4/PP2RPPP/R2Q2K1 w - - bm Re8+; id \"WAC.268\";");
  eval("2kr2nr/pp1n1ppp/2p1p3/q7/1b1P1B2/P1N2Q1P/1PP1BPP1/R3K2R w KQ - bm axb4; id \"WAC.269\";");
  eval("2r1r1k1/pp1q1ppp/3p1b2/3P4/3Q4/5N2/PP2RPPP/4R1K1 w - - bm Qg4; id \"WAC.270\";");
  eval("2kr4/ppp3Pp/4RP1B/2r5/5P2/1P6/P2p4/3K4 w - - bm Rd6; id \"WAC.271\";");
  eval("nrq4r/2k1p3/1p1pPnp1/pRpP1p2/P1P2P2/2P1BB2/1R2Q1P1/6K1 w - - bm Bxc5; id \"WAC.272\";");
  eval("2k4B/bpp1qp2/p1b5/7p/1PN1n1p1/2Pr4/P5PP/R3QR1K b - - bm Ng3+ g3; id \"WAC.273\";");
  eval("8/1p6/p5R1/k7/Prpp4/K7/1NP5/8 w - - bm Rb6 Rg5+; am Rd6; id \"WAC.274\";");
  eval("r1b2rk1/1p1n1ppp/p1p2q2/4p3/P1B1Pn2/1QN2N2/1P3PPP/3R1RK1 b - - bm Nc5 Nxg2 b5; id \"WAC.275\";");
  eval("r5k1/pp1RR1pp/1b6/6r1/2p5/B6P/P4qPK/3Q4 w - - bm Qd5+; id \"WAC.276\";");
  eval("1r4r1/p2kb2p/bq2p3/3p1p2/5P2/2BB3Q/PP4PP/3RKR2 b - - bm Rg3 Rxg2; id \"WAC.277\";");
  eval("r2qkb1r/pppb2pp/2np1n2/5pN1/2BQP3/2N5/PPP2PPP/R1B1K2R w KQkq - bm Bf7+; id \"WAC.278\";");
  eval("r7/4b3/2p1r1k1/1p1pPp1q/1P1P1P1p/PR2NRpP/2Q3K1/8 w - - bm Nxf5 Rc3; id \"WAC.279\";");
  eval("r1r2bk1/5p1p/pn4p1/N2b4/3Pp3/B3P3/2q1BPPP/RQ3RK1 b - - bm Bxa3; id \"WAC.280\";");
  eval("2R5/2R4p/5p1k/6n1/8/1P2QPPq/r7/6K1 w - - bm Rxh7+; id \"WAC.281\";");
  eval("6k1/2p3p1/1p1p1nN1/1B1P4/4PK2/8/2r3b1/7R w - - bm Rh8+; id \"WAC.282\";");
  eval("3q1rk1/4bp1p/1n2P2Q/3p1p2/6r1/Pp2R2N/1B4PP/7K w - - bm Ng5; id \"WAC.283\";");
  eval("3r3k/pp4pp/8/1P6/3N4/Pn2P1qb/1B1Q2B1/2R3K1 w - - bm Nf5; id \"WAC.284\";");
  eval("2rr3k/1b2bppP/p2p1n2/R7/3P4/1qB2P2/1P4Q1/1K5R w - - bm Qxg7+; id \"WAC.285\";");
  eval("3r1k2/1p6/p4P2/2pP2Qb/8/1P1KB3/P6r/8 b - - bm Rxd5+; id \"WAC.286\";");
  eval("rn3k1r/pp2bBpp/2p2n2/q5N1/3P4/1P6/P1P3PP/R1BQ1RK1 w - - bm Qg4 Qh5; id \"WAC.287\";");
  eval("r1b2rk1/p4ppp/1p1Qp3/4P2N/1P6/8/P3qPPP/3R1RK1 w - - bm Nf6+; id \"WAC.288\";");
  eval("2r3k1/5p1p/p3q1p1/2n3P1/1p1QP2P/1P4N1/PK6/2R5 b - - bm Qe5; id \"WAC.289\";");
  eval("2k2r2/2p5/1pq5/p1p1n3/P1P2n1B/1R4Pp/2QR4/6K1 b - - bm Ne2+; id \"WAC.290\";");
  eval("5r1k/3b2p1/p6p/1pRpR3/1P1P2q1/P4pP1/5QnP/1B4K1 w - - bm h3; id \"WAC.291\";");
  eval("4r3/1Q1qk2p/p4pp1/3Pb3/P7/6PP/5P2/4R1K1 w - - bm d6+; id \"WAC.292\";");
  eval("1nbq1r1k/3rbp1p/p1p1pp1Q/1p6/P1pPN3/5NP1/1P2PPBP/R4RK1 w - - bm Nfg5; id \"WAC.293\";");
  eval("3r3k/1r3p1p/p1pB1p2/8/p1qNP1Q1/P6P/1P4P1/3R3K w - - bm Bf8 Nf5 Qf4; id \"WAC.294\";");
  eval("4r3/p4r1p/R1p2pp1/1p1bk3/4pNPP/2P1K3/2P2P2/3R4 w - - bm Rxd5+; id \"WAC.295\";");
  eval("3r4/1p2k2p/p1b1p1p1/4Q1Pn/2B3KP/4pP2/PP2R1N1/6q1 b - - bm Rd4+ Rf8; id \"WAC.296\";");
  eval("3r1rk1/p3qp1p/2bb2p1/2p5/3P4/1P6/PBQN1PPP/2R2RK1 b - - bm Bxg2 Bxh2+; id \"WAC.297\";");
  eval("3Q4/p3b1k1/2p2rPp/2q5/4B3/P2P4/7P/6RK w - - bm Qh8+; id \"WAC.298\";");
  eval("1n2rr2/1pk3pp/pNn2p2/2N1p3/8/6P1/PP2PPKP/2RR4 w - - bm Nca4; id \"WAC.299\";");
  eval("b2b1r1k/3R1ppp/4qP2/4p1PQ/4P3/5B2/4N1K1/8 w - - bm g6; id \"WAC.300\";");
}


void evalQuietMoves(void) {
  eval("1qr3k1/p2nbppp/bp2p3/3p4/3P4/1P2PNP1/P2Q1PBP/1N2R1K1 b - - bm Qc7; id \"sbd.001\";");
  eval("1r2r1k1/3bnppp/p2q4/2RPp3/4P3/6P1/2Q1NPBP/2R3K1 w - - bm Rc7; id \"sbd.002\";");
  eval("2b1k2r/2p2ppp/1qp4n/7B/1p2P3/5Q2/PPPr2PP/R2N1R1K b k - bm O-O; id \"sbd.003\";");
  eval("2b5/1p4k1/p2R2P1/4Np2/1P3Pp1/1r6/5K2/8 w - - bm Rd8; id \"sbd.004\";");
  eval("2brr1k1/ppq2ppp/2pb1n2/8/3NP3/2P2P2/P1Q2BPP/1R1R1BK1 w - - bm g3; id \"sbd.005\";");
  eval("2kr2nr/1pp3pp/p1pb4/4p2b/4P1P1/5N1P/PPPN1P2/R1B1R1K1 b - - bm Bf7; id \"sbd.006\";");
  eval("2r1k2r/1p1qbppp/p3pn2/3pBb2/3P4/1QN1P3/PP2BPPP/2R2RK1 b k - bm O-O; id \"sbd.007\";");
  eval("2r1r1k1/pbpp1npp/1p1b3q/3P4/4RN1P/1P4P1/PB1Q1PB1/2R3K1 w - - bm Rce1; id \"sbd.008\";");
  eval("2r2k2/r4p2/2b1p1p1/1p1p2Pp/3R1P1P/P1P5/1PB5/2K1R3 w - - bm Kd2; id \"sbd.009\";");
  eval("2r3k1/5pp1/1p2p1np/p1q5/P1P4P/1P1Q1NP1/5PK1/R7 w - - bm Rd1; id \"sbd.010\";");
  eval("2r3qk/p5p1/1n3p1p/4PQ2/8/3B4/5P1P/3R2K1 w - - bm e6; id \"sbd.011\";");
  eval("3b4/3k1pp1/p1pP2q1/1p2B2p/1P2P1P1/P2Q3P/4K3/8 w - - bm Qf3; id \"sbd.012\";");
  eval("3n1r1k/2p1p1bp/Rn4p1/6N1/3P3P/2N1B3/2r2PP1/5RK1 w - - bm Na4 Nce4; id \"sbd.013\";");
  eval("3q1rk1/3rbppp/ppbppn2/1N6/2P1P3/BP6/P1B1QPPP/R3R1K1 w - - bm Nd4; id \"sbd.014\";");
  eval("3r1rk1/p1q4p/1pP1ppp1/2n1b1B1/2P5/6P1/P1Q2PBP/1R3RK1 w - - bm Bh6; id \"sbd.015\";");
  eval("3r2k1/2q2p1p/5bp1/p1pP4/PpQ5/1P3NP1/5PKP/3R4 b - - bm Qd6; id \"sbd.016\";");
  eval("3r2k1/p1q1npp1/3r1n1p/2p1p3/4P2B/P1P2Q1P/B4PP1/1R2R1K1 w - - bm Bc4; id \"sbd.017\";");
  eval("3r4/2k5/p3N3/4p3/1p1p4/4r3/PPP3P1/1K1R4 b - - bm Kd7; id \"sbd.018\";");
  eval("3r4/2R1np1p/1p1rpk2/p2b1p2/8/PP2P3/4BPPP/2R1NK2 w - - bm b4; id \"sbd.019\";");
  eval("3rk2r/1b2bppp/p1qppn2/1p6/4P3/PBN2PQ1/1PP3PP/R1B1R1K1 b k - bm O-O; id \"sbd.020\";");
  eval("3rk2r/1bq2pp1/2pb1n1p/p3pP2/P1B1P3/8/1P2QBPP/2RN1R1K b k - bm Be7 O-O; id \"sbd.021\";");
  eval("3rkb1r/pppb1pp1/4n2p/2p5/3NN3/1P5P/PBP2PP1/3RR1K1 w - - bm Nf5; id \"sbd.022\";");
  eval("3rr1k1/1pq2ppp/p1n5/3p4/6b1/2P2N2/PP1QBPPP/3R1RK1 w - - bm Rfe1; id \"sbd.023\";");
  eval("4r1k1/1q1n1ppp/3pp3/rp6/p2PP3/N5P1/PPQ2P1P/3RR1K1 w - - bm Rc1; id \"sbd.024\";");
  eval("4rb1k/1bqn1pp1/p3rn1p/1p2pN2/1PP1p1N1/P1P2Q1P/1BB2PP1/3RR1K1 w - - bm Qe2; id \"sbd.025\";");
  eval("4rr2/2p5/1p1p1kp1/p6p/P1P4P/6P1/1P3PK1/3R1R2 w - - bm Rfe1; id \"sbd.026\";");
  eval("5r2/pp1b1kpp/8/2pPp3/2P1p2P/4P1r1/PPRKB1P1/6R1 b - - bm Ke7; id \"sbd.027\";");
  eval("6k1/1R5p/r2p2p1/2pN2B1/2bb4/P7/1P1K2PP/8 w - - bm Nf6+; id \"sbd.028\";");
  eval("6k1/pp1q1pp1/2nBp1bp/P2pP3/3P4/8/1P2BPPP/2Q3K1 w - - bm Qc5; id \"sbd.029\";");
  eval("6k1/pp2rp1p/2p2bp1/1n1n4/1PN3P1/P2rP2P/R3NPK1/2B2R2 w - - bm Rd2; id \"sbd.030\";");
  eval("8/2p2kpp/p6r/4Pp2/1P2pPb1/2P3P1/P2B1K1P/4R3 w - - bm h4; id \"sbd.031\";");
  eval("Q5k1/5pp1/5n1p/2b2P2/8/5N1P/5PP1/2q1B1K1 b - - bm Kh7; id \"sbd.032\";");
  eval("r1b1k1nr/1p3ppp/p1np4/4p1q1/2P1P3/N1NB4/PP3PPP/2RQK2R w Kkq - bm O-O; id \"sbd.033\";");
  eval("r1b1k2r/p1pp1ppp/1np1q3/4P3/1bP5/1P6/PB1NQPPP/R3KB1R b KQkq - bm O-O; id \"sbd.034\";");
  eval("r1b1k2r/ppppqppp/8/2bP4/3p4/6P1/PPQPPPBP/R1B2RK1 b kq - bm O-O; id \"sbd.035\";");
  eval("r1b1k2r/ppq1bppp/2n5/2N1p3/8/2P1B1P1/P3PPBP/R2Q1RK1 b kq - bm O-O; id \"sbd.036\";");
  eval("r1b1kb1r/pp2qppp/2pp4/8/4nP2/2N2N2/PPPP2PP/R1BQK2R w KQkq - bm O-O; id \"sbd.037\";");
  eval("r1b1qrk1/pp4b1/2pRn1pp/5p2/2n2B2/2N2NPP/PPQ1PPB1/5RK1 w - - bm Rd3; id \"sbd.038\";");
  eval("r1b2rk1/1pqn1pp1/p2bpn1p/8/3P4/2NB1N2/PPQB1PPP/3R1RK1 w - - bm Rc1; id \"sbd.039\";");
  eval("r1b2rk1/2qnbp1p/p1npp1p1/1p4PQ/4PP2/1NNBB3/PPP4P/R4RK1 w - - bm Qh6; id \"sbd.040\";");
  eval("r1b2rk1/pp2ppbp/2n2np1/2q5/5B2/1BN1PN2/PP3PPP/2RQK2R w K - bm O-O; id \"sbd.041\";");
  eval("r1b2rk1/pp4pp/1q1Nppn1/2n4B/1P3P2/2B2RP1/P6P/R2Q3K b - - bm Na6; id \"sbd.042\";");
  eval("r1b2rk1/ppp1qppp/1b1n4/8/B2n4/3NN3/PPPP1PPP/R1BQK2R w KQ - bm O-O; id \"sbd.043\";");
  eval("r1b2rk1/ppq1bppp/2p1pn2/8/2NP4/2N1P3/PP2BPPP/2RQK2R w K - bm O-O; id \"sbd.044\";");
  eval("r1bq1rk1/1p1n1pp1/p4n1p/2bp4/8/2NBPN2/PPQB1PPP/R3K2R w KQ - bm O-O; id \"sbd.045\";");
  eval("r1bq1rk1/1p2ppbp/p2p1np1/6B1/2P1P3/2N5/PP1QBPPP/R3K2R w KQ - bm O-O; id \"sbd.046\";");
  eval("r1bq1rk1/1p3ppp/p1np4/3Np1b1/2B1P3/P7/1PP2PPP/RN1QK2R w KQ - bm O-O; id \"sbd.047\";");
  eval("r1bq1rk1/4bppp/ppnppn2/8/2P1P3/2N5/PPN1BPPP/R1BQK2R w KQ - bm O-O; id \"sbd.048\";");
  eval("r1bq1rk1/pp1n1pbp/2n1p1p1/2ppP3/8/2PP1NP1/PP1N1PBP/R1BQ1RK1 w - - bm d4; id \"sbd.049\";");
  eval("r1bq1rk1/pp1pppbp/2n2np1/8/4P3/1NN5/PPP1BPPP/R1BQK2R w KQ - bm O-O; id \"sbd.050\";");
  eval("r1bq1rk1/pp2ppbp/2n2np1/2p3B1/4P3/2P2N2/PP1NBPPP/R2QK2R w KQ - bm O-O; id \"sbd.051\";");
  eval("r1bq1rk1/pp2ppbp/2n3p1/2p5/2BPP3/2P1B3/P3NPPP/R2QK2R w KQ - bm O-O; id \"sbd.052\";");
  eval("r1bq1rk1/pp3ppp/2n1pn2/2p5/1bBP4/2N1PN2/PP3PPP/R1BQ1RK1 w - - bm a3; id \"sbd.053\";");
  eval("r1bq1rk1/pp3ppp/2n2n2/3p4/8/P1NB4/1PP2PPP/R1BQK2R w KQ - bm O-O; id \"sbd.054\";");
  eval("r1bq1rk1/ppp1npb1/3p2pp/3Pp2n/1PP1P3/2N5/P2NBPPP/R1BQR1K1 b - - bm Nf4; id \"sbd.055\";");
  eval("r1bq1rk1/ppp2ppp/2n1pn2/3p4/1bPP4/2NBPN2/PP3PPP/R1BQK2R w KQ - bm O-O; id \"sbd.056\";");
  eval("r1bq1rk1/pppp1pbp/2n2np1/4p3/2P5/P1N2NP1/1P1PPPBP/R1BQK2R w KQ - bm O-O; id \"sbd.057\";");
  eval("r1bqk2r/2ppbppp/p1n2n2/1p2p3/4P3/1B3N2/PPPPQPPP/RNB2RK1 b kq - bm O-O; id \"sbd.058\";");
  eval("r1bqk2r/5ppp/p1np4/1p1Np1b1/4P3/2P5/PPN2PPP/R2QKB1R b KQkq - bm O-O; id \"sbd.059\";");
  eval("r1bqk2r/bp3ppp/p1n1pn2/3p4/1PP5/P1N1PN2/1B3PPP/R2QKB1R b KQkq - bm O-O; id \"sbd.060\";");
  eval("r1bqk2r/p2pppbp/2p3pn/2p5/4P3/2P2N2/PP1P1PPP/RNBQR1K1 b kq - bm O-O; id \"sbd.061\";");
  eval("r1bqk2r/pp2bppp/2n1p3/1B1n4/3P4/2N2N2/PP3PPP/R1BQ1RK1 b kq - bm O-O; id \"sbd.062\";");
  eval("r1bqk2r/pp2bppp/2n1p3/3n4/3P4/2NB1N2/PP3PPP/R1BQ1RK1 b kq - bm O-O; id \"sbd.063\";");
  eval("r1bqk2r/pp2ppbp/2np1np1/2p5/4P3/1B1P1N1P/PPP2PP1/RNBQK2R w KQkq - bm O-O; id \"sbd.064\";");
  eval("r1bqk2r/ppn1bppp/2n5/2p1p3/8/2NP1NP1/PP1BPPBP/R2Q1RK1 b kq - bm O-O; id \"sbd.065\";");
  eval("r1bqk2r/ppp1bppp/2n5/3p4/3P4/2PB1N2/P1P2PPP/R1BQ1RK1 b kq - bm O-O; id \"sbd.066\";");
  eval("r1bqk2r/ppp2ppp/2nb4/3np3/8/PP2P3/1BQP1PPP/RN2KBNR b KQkq - bm O-O; id \"sbd.067\";");
  eval("r1bqk2r/ppp2ppp/3b4/4p3/8/1PPP1N2/2PB1PPP/R2Q1RK1 b kq - bm O-O; id \"sbd.068\";");
  eval("r1bqk2r/pppp1ppp/5n2/4p3/Bb2P3/5Q2/PPPPNPPP/R1B1K2R b KQkq - bm O-O; id \"sbd.069\";");
  eval("r1bqkb1r/pp3ppp/2n5/2pp4/3Pn3/2N2N2/PPP1BPPP/R1BQK2R w KQkq - bm O-O; id \"sbd.070\";");
  eval("r1bqkb1r/pp3ppp/2npp3/3nP3/2BP4/5N2/PP3PPP/RNBQK2R w KQkq - bm O-O; id \"sbd.071\";");
  eval("r1bqkbnr/3p1ppp/p1p1p3/8/4P3/3B4/PPP2PPP/RNBQK2R w KQkq - bm O-O; id \"sbd.072\";");
  eval("r1bqkbnr/ppp2ppp/2n5/8/2BpP3/5N2/PP3PPP/RNBQK2R w KQkq - bm O-O; id \"sbd.073\";");
  eval("r1bqrbk1/1pp3pp/2n2p2/p2np3/8/PP1PPN2/1BQNBPPP/R3K2R w KQ - bm O-O; id \"sbd.074\";");
  eval("r1br2k1/1p2qppp/pN2pn2/P7/2pn4/4N1P1/1P2PPBP/R3QRK1 b - - bm Rb8; id \"sbd.075\";");
  eval("r1q1k2r/1b1nbppp/pp1ppn2/8/2PQP3/1PN2NP1/PB3PBP/R2R2K1 b kq - bm O-O; id \"sbd.076\";");
  eval("r1q1k2r/pb1nbppp/1p2pn2/8/P1PNP3/2B3P1/2QN1PBP/R4RK1 b kq - bm O-O; id \"sbd.077\";");
  eval("r1r3k1/1bq2pbp/pp1pp1p1/2n5/P3PP2/R2B4/1PPBQ1PP/3N1R1K w - - bm Bc3; id \"sbd.078\";");
  eval("r1rn2k1/pp1qppbp/6p1/3pP3/3P4/1P3N1P/PB1Q1PP1/R3R1K1 w - - bm Rac1; id \"sbd.079\";");
  eval("r2q1rk1/1b1nbpp1/pp2pn1p/8/2BN3B/2N1P3/PP2QPPP/2R2RK1 w - - bm Rfd1; id \"sbd.080\";");
  eval("r2q1rk1/1b3ppp/4pn2/1pP5/1b6/2NBPN2/1PQ2PPP/R3K2R w KQ - bm O-O; id \"sbd.081\";");
  eval("r2q1rk1/pb1nppbp/6p1/1p6/3PP3/3QBN1P/P3BPP1/R3K2R w KQ - bm O-O; id \"sbd.082\";");
  eval("r2q1rk1/pb2bppp/npp1pn2/3pN3/2PP4/1PB3P1/P2NPPBP/R2Q1RK1 w - - bm e4; id \"sbd.083\";");
  eval("r2q1rk1/pppb1pbp/2np1np1/4p3/2P5/P1NPPNP1/1P3PBP/R1BQK2R w KQ - bm O-O; id \"sbd.084\";");
  eval("r2qk2r/1b1n1ppp/4pn2/p7/1pPP4/3BPN2/1B3PPP/R2QK2R w KQkq - bm O-O; id \"sbd.085\";");
  eval("r2qk2r/1b2bppp/p1n1pn2/1p6/1P6/P2BPN2/1B2QPPP/RN3RK1 b kq - bm O-O; id \"sbd.086\";");
  eval("r2qk2r/2p2ppp/p1n1b3/1pbpP3/4n3/1BP2N2/PP1N1PPP/R1BQ1RK1 b kq - bm O-O; id \"sbd.087\";");
  eval("r2qk2r/3n1ppp/p3p3/3nP3/3R4/5N2/1P1N1PPP/3QR1K1 b kq - bm O-O; id \"sbd.088\";");
  eval("r2qk2r/p1pn1ppp/b3pn2/3p4/Nb1P4/1P3NP1/P3PPBP/1RBQ1RK1 b kq - bm O-O Qe7; id \"sbd.089\";");
  eval("r2qk2r/ppp1bppp/2n2n2/8/2BP2b1/2N2N2/PP3PPP/R1BQR1K1 b kq - bm O-O; id \"sbd.090\";");
  eval("r2qkb1r/pb1n1p2/2p1p2p/4P1pn/PppP4/2N2NB1/1P2BPPP/R2Q1RK1 w kq - bm Ne4; id \"sbd.091\";");
  eval("r2qkb1r/pp2nppp/1np1p3/4Pb2/3P4/PB3N2/1P3PPP/RNBQ1RK1 b kq - bm Ned5; id \"sbd.092\";");
  eval("r2qkb1r/pp3ppp/2bppn2/8/2PQP3/2N2N2/PP3PPP/R1B1K2R w KQkq - bm O-O; id \"sbd.093\";");
  eval("r2qr1k1/p3bppp/1p2n3/3Q1N2/5P2/4B1P1/PP3R1P/R5K1 w - - bm Rd1; id \"sbd.094\";");
  eval("r2r2k1/p1pnqpp1/1p2p2p/3b4/3P4/3BPN2/PP3PPP/2RQR1K1 b - - bm c5; id \"sbd.095\";");
  eval("r2r2k1/pp1b1ppp/8/3p2P1/3N4/P3P3/1P3P1P/3RK2R b K - bm Rac8; id \"sbd.096\";");
  eval("r3k2r/1b1nb1p1/p1q1pn1p/1pp3N1/4PP2/2N5/PPB3PP/R1BQ1RK1 w kq - bm Nf3; id \"sbd.097\";");
  eval("r3k2r/1pqnnppp/p5b1/1PPp1p2/3P4/2N5/P2NB1PP/2RQ1RK1 b kq - bm O-O; id \"sbd.098\";");
  eval("r3k2r/p1q1nppp/1pn5/2P1p3/4P1Q1/P1P2P2/4N1PP/R1B2K1R b kq - bm O-O; id \"sbd.099\";");
  eval("r3k2r/pp2pp1p/6p1/2nP4/1R2PB2/4PK2/P5PP/5bNR w kq - bm Ne2; id \"sbd.100\";");
  eval("r3k2r/ppp1bppp/2n5/3n4/3PB3/8/PP3PPP/RNB1R1K1 b kq - bm O-O-O; id \"sbd.101\";");
  eval("r3kb1r/pp3ppp/4bn2/3p4/P7/4N1P1/1P2PPBP/R1B1K2R w KQkq - bm O-O; id \"sbd.102\";");
  eval("r3kbnr/1pp3pp/p1p2p2/8/3qP3/5Q1P/PP3PP1/RNB2RK1 w kq - bm Rd1; id \"sbd.103\";");
  eval("r3kr2/pppb1p2/2n3p1/3Bp2p/4P2N/2P5/PP3PPP/2KR3R b q - bm O-O-O; id \"sbd.104\";");
  eval("r3nrk1/pp2qpb1/3p1npp/2pPp3/2P1P2N/2N3Pb/PP1BBP1P/R2Q1RK1 w - - bm Re1; id \"sbd.105\";");
  eval("r3r1k1/1pqn1pbp/p2p2p1/2nP2B1/P1P1P3/2NB3P/5PP1/R2QR1K1 w - - bm Rc1; id \"sbd.106\";");
  eval("r3r1k1/pp1q1ppp/2p5/P2n1p2/1b1P4/1B2PP2/1PQ3PP/R1B2RK1 w - - bm e4; id \"sbd.107\";");
  eval("r3r1k1/pp3ppp/2ppqn2/5R2/2P5/2PQP1P1/P2P2BP/5RK1 w - - bm Qd4; id \"sbd.108\";");
  eval("r3rbk1/p2b1p2/5p1p/1q1p4/N7/6P1/PP1BPPBP/3Q1RK1 w - - bm Nc3; id \"sbd.109\";");
  eval("r4r1k/pp1bq1b1/n2p2p1/2pPp1Np/2P4P/P1N1BP2/1P1Q2P1/2KR3R w - - bm Ne6; id \"sbd.110\";");
  eval("r4rk1/1bqp1ppp/pp2pn2/4b3/P1P1P3/2N2BP1/1PQB1P1P/2R2RK1 w - - bm b3; id \"sbd.111\";");
  eval("r4rk1/1q2bppp/p1bppn2/8/3BPP2/3B2Q1/1PP1N1PP/4RR1K w - - bm e5; id \"sbd.112\";");
  eval("r4rk1/pp2qpp1/2pRb2p/4P3/2p5/2Q1PN2/PP3PPP/4K2R w K - bm O-O; id \"sbd.113\";");
  eval("r7/3rq1kp/2p1bpp1/p1Pnp3/2B4P/PP4P1/1B1RQP2/2R3K1 b - - bm Rad8; id \"sbd.114\";");
  eval("r7/pp1bpp2/1n1p2pk/1B3P2/4P1P1/2N5/PPP5/1K5R b - - bm Kg5; id \"sbd.115\";");
  eval("rn1q1rk1/p4pbp/bp1p1np1/2pP4/8/P1N2NP1/1PQ1PPBP/R1B1K2R w KQ - bm O-O; id \"sbd.116\";");
  eval("rn1q1rk1/pb3p2/1p5p/3n2P1/3p4/P4P2/1P1Q1BP1/R3KBNR b KQ - bm Re8+; id \"sbd.117\";");
  eval("rn1q1rk1/pp2bppp/1n2p1b1/8/2pPP3/1BN1BP2/PP2N1PP/R2Q1RK1 w - - bm Bc2; id \"sbd.118\";");
  eval("rn1q1rk1/pp3ppp/4bn2/2bp4/5B2/2NBP1N1/PP3PPP/R2QK2R w KQ - bm O-O; id \"sbd.119\";");
  eval("rn1qkbnr/pp1b1ppp/8/1Bpp4/3P4/8/PPPNQPPP/R1B1K1NR b KQkq - bm Qe7; id \"sbd.120\";");
  eval("rn1qr1k1/pb3p2/1p5p/3n2P1/3p4/P4P2/1P1QNBP1/R3KB1R b KQ - bm d3; id \"sbd.121\";");
  eval("rn2kb1r/pp2nppp/1q2p3/3pP3/3P4/5N2/PP2NPPP/R1BQK2R w KQkq - bm O-O; id \"sbd.122\";");
  eval("rn3rk1/1bqp1ppp/p3pn2/8/Nb1NP3/4B3/PP2BPPP/R2Q1RK1 w - - bm Rc1; id \"sbd.123\";");
  eval("rn3rk1/pbp1qppp/1p1ppn2/8/2PP4/P1Q2NP1/1P2PPBP/R1B1K2R w KQ - bm O-O; id \"sbd.124\";");
  eval("rnb1k2r/1pq2ppp/p2ppn2/2b5/3NPP2/2P2B2/PP4PP/RNBQ1R1K b kq - bm O-O; id \"sbd.125\";");
  eval("rnb2rk1/ppq1ppbp/6p1/2p5/3PP3/2P2N2/P3BPPP/1RBQK2R w K - bm O-O; id \"sbd.126\";");
  eval("rnbq1rk1/5ppp/p3pn2/1p6/2BP4/P1P2N2/5PPP/R1BQ1RK1 w - - bm Bd3; id \"sbd.127\";");
  eval("rnbq1rk1/pp2ppbp/2pp1np1/8/P2PP3/2N2N2/1PP1BPPP/R1BQK2R w KQ - bm O-O; id \"sbd.128\";");
  eval("rnbq1rk1/ppp1ppbp/6p1/8/8/2P2NP1/P2PPPBP/R1BQK2R w KQ - bm O-O; id \"sbd.129\";");
  eval("rnbqk1nr/pp3pbp/2ppp1p1/8/2BPP3/2N2Q2/PPP2PPP/R1B1K1NR w KQkq - bm Nge2; id \"sbd.130\";");
  eval("rnbqk2r/ppp2ppp/1b1p1n2/4p3/2B1P3/2PP1N2/PP1N1PPP/R1BQK2R b KQkq - bm O-O; id \"sbd.131\";");
  eval("rnbqk2r/pppp2pp/4pn2/5p2/1b1P4/2P2NP1/PP2PPBP/RNBQK2R b KQkq - bm Be7; id \"sbd.132\";");
  eval("rnbqr1k1/pp1p1ppp/5n2/3Pb3/1P6/P1N3P1/4NPBP/R1BQK2R w KQ - bm O-O; id \"sbd.133\";");
  eval("rnq1nrk1/pp3pbp/6p1/3p4/3P4/5N2/PP2BPPP/R1BQK2R w KQ - bm O-O; id \"sbd.134\";");

  printStats("silent but deadly");
}


void evalMain() {
  //eval("1k6/5RP1/1P6/1K6/6r1/8/8/8 w - - bm Ka5 Kc5 b7; id \"WAC.500\";");
  //eval("1k6/8/8/1K6/8/8/8/8 w - - bm Ka5 Kc5; id \"WAC.500\";");

  evalWinAtChess0();
  evalWinAtChess50();
  evalWinAtChess100();
  evalWinAtChess200();
  printStats("Win at Chess");

  // Saving for later after tactics have been refined a little.
  //evalQuietMoves();
}


int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  if (FLAGS_eval_test_custom_size > 0) {
    int flag = FLAGS_eval_test_custom_size;
    testPlySize = flag <= 10 ? flag : 2;
    testNodeSize = flag <= 10 ? 1000 : flag;
  } else {
    testPlySize = predefinedPlySizes.at(FLAGS_eval_test_size);
    testNodeSize = predefinedNodeSizes.at(FLAGS_eval_test_size);
  }
  cout << "\twith test size: " << testPlySize << ", " << testNodeSize << endl;;

  auto T0 = chrono::system_clock().now();

  evalMain();

  auto T1 = chrono::system_clock().now();
  chrono::duration<double> duration = T1 - T0;
  double duration_s = duration.count();

  printf("\t(%.2f seconds)\n", duration_s);
}


vector<string> getBestMoves(Board &b, string epd) {
  vector<string> moves;

  size_t bestIndex = epd.find("bm ");
  size_t bestIndexEnd = epd.find(";", bestIndex);
  if (bestIndex == string::npos || bestIndexEnd == string::npos) {
    cerr << "Bad epd: " << epd << " " << bestIndex << " " << bestIndexEnd << endl;
    assert (false);
  }

  bestIndex += 3; // Advance over "bm ".
  string bestMove = epd.substr(bestIndex, bestIndexEnd - bestIndex);

  // bestMove may contain multiple moves seperated by spaces.
  while (true) {
    size_t space = bestMove.find(" ");
    if (space == string::npos) {
      break;
    }
    string partial = bestMove.substr(0, space);
    moves.push_back(partial);
    bestMove = bestMove.substr(space + 1);
  }
  moves.push_back(bestMove);

  assert( verifyIsInLegal(b, moves) == true );
  return moves;
}


bool verifyIsInLegal(Board &b, vector<string> verify) {
  // Step 2a.
  vector<string> children;
  for (auto c : b.getLegalChildren()) {
    children.push_back(b.algebraicNotation_slow(c.getLastMove()));
  }

  sort(verify.begin(), verify.end());
  sort(children.begin(), children.end());

  vector<string> diff;
  set_difference(verify.begin(), verify.end(),
                 children.begin(),  children.end(),
                 inserter(diff, diff.end()));

  if (diff.empty()) {
    return true;
  }

  b.printBoard();

  cout << "getLegalChildren: ";
  for (auto move : children) {
    cout << move << (move == children.back() ? "" : ", ");
  }
  cout << endl;

  cout << "getLegalChildren didn't include: ";
  for (auto move : diff) {
    cout << move << (move == diff.back() ? "" : ", ");
  }
  cout << endl;

  return false;
}


// Small note tracking.
//    WAC 72/300 with Alpha/Beta | evaluation = materialDiff    (450s but not improved by adding time)
//    WAC 62     with Alpha/Beta | evaluation = material + PST  (160s)
//    WAC 91!    with Alpha/Beta | evaluation = material + PST  (1800s on work machine = 7M)
