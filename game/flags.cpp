#ifndef FLAGS_H
#define FLAGS_H

#include <string>
#include <gflags/gflags.h>

#include "flags.cpp"

using namespace std;

DEFINE_int32(verbosity, 0, "print lots of stuff (higher = more)");

DEFINE_int32(server_min_ply, 4, "min ply for findMove in server");
DEFINE_int32(server_min_nodes, 75000, "min nodes for findMove in server");

DEFINE_bool(use_ttable, false, "Use Transposition table in FindMove");

DEFINE_string(eval_test_size, "",
      "Predetermined limits (instant, small, medium, large)");

DEFINE_int32(eval_test_custom_size, 0,
      "Custom minimum number of nodes to per eval position");

DEFINE_bool(test_perft, false, "only test perft");
DEFINE_bool(test_play, false, "only test perft");
DEFINE_bool(test_simple, false, "only test update and hash");
DEFINE_bool(test_endgame, false, "only test endgame handling");

// Try to add a validator here.
static bool ValidateEvalTestSize(const char* flagname, const string& flagvalue) {
  return flagvalue == "" ||
         flagvalue == "instant" || flagvalue == "small" ||
         flagvalue == "medium" || flagvalue == "large";
}

static bool ValidateEvalTestCustomSize(const char* flagname, int flagvalue) {
  int K = 1000;
  return flagvalue == 0 ||
         (2 <= flagvalue && flagvalue <= 10) ||
         (K <= flagvalue && flagvalue <= 10 * K *K);
}

// Define validators in a block here.

DEFINE_validator(server_min_ply, &ValidateEvalTestCustomSize);
DEFINE_validator(server_min_nodes, &ValidateEvalTestCustomSize);

DEFINE_validator(eval_test_size, &ValidateEvalTestSize);
DEFINE_validator(eval_test_custom_size, &ValidateEvalTestCustomSize);


#endif // FLAGS_H
