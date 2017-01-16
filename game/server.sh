time g++ -std=c++14 -levent -fopenmp -O2 -o betachess-server board.h board.cpp server.cpp

# TODO fix this because time masks the failure of g++
if [[ $? ]]; then
  echo
  echo
  echo
  time ./betachess-server
fi
