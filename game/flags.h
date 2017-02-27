#ifndef FLAGS_H
#define FLAGS_H

#include <gflags/gflags.h>

// Compile time flags here.
#define IS_ANTICHESS false


// Declare runtime flags here.
DECLARE_int32(verbosity);

DECLARE_bool(use_t_table);
DECLARE_string(eval_test_size);
DECLARE_int32(eval_test_custom_size);

DECLARE_bool(test_perft);
DECLARE_bool(test_play);
DECLARE_bool(test_simple);
DECLARE_bool(test_endgame);


#endif // FLAGS_H
