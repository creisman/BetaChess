#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <set>
#include <vector>

#include "board.h"

using namespace std;
using namespace board;

// The general idea is that these test tricky problems.


// TODO: come up with some constants or enum for this.
int K_NODES = 1000;
int TEST_INSTANT =   100 * K_NODES;
int TEST_SHALLOW =  1000 * K_NODES;


vector<string> getBestMoves(Board &b, string epd);
bool verifyIsInLegal(Board &b, vector<string> moves);


int success = 0;
int missed = 0;

void printStats(string setName) {
  int total = success + missed;
  cout << success << " out of (" << total << ") for set \"" << setName << "\"" << endl;
}


bool eval(string epd) {
  // Extended Position Description.

  // Step 1: Parse position
  // Step 2: Parse best moves.
  // Step 2a: check if we consider this a legal move.
  // Step 3: Test with depth / count / time.


  // Step 1.
  // The epd's parsed here general start with "<FEN> bm Ke3; id set.012;".
  string fen = epd.substr(0, epd.find(" bm"));
  // They also tend to be missing move counts.
  fen += " 0 0";
  Board b(fen);

  // Step 2.
  vector<string> bestMoves = getBestMoves(b, epd);
  cout << epd << endl;
  cout << "\tbm: \"";
  for (string move : bestMoves) {
    cout << move << ((move == bestMoves.back()) ? "" : ", ");
  }
  cout << "\"" << endl;

  // Step 3.
  move_t move = get<1>(b.findMove(TEST_INSTANT));
  string moveName = b.algebraicNotation_slow(move);

  bool found = find(bestMoves.begin(), bestMoves.end(), moveName) != bestMoves.end();

  if (found) {
    success += 1;
    cout << "Found: " << moveName << endl;
  } else {
    missed += 1;
    cout << "Missed (" << moveName << " instead of " << bestMoves[0] << ")" << endl;
  }
  cout << endl;
}



float evalQuiteMoves(void) {
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
  //eval("6k1/1R5p/r2p2p1/2pN2B1/2bb4/P7/1P1K2PP/8 w - - bm Nf6+; id \"sbd.028\";");
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
  //eval("rn1q1rk1/pb3p2/1p5p/3n2P1/3p4/P4P2/1P1Q1BP1/R3KBNR b KQ - bm Re8+; id \"sbd.117\";");
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

  // With nodes = 1M and only the trivial heuristic 15 of 132.
  // This doesn't improve by increasing nodes up to 4M.
  printStats("silent but deadly");
}


int main(void) {
  evalQuiteMoves();

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
