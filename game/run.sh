time g++ -std=c++14 -fopenmp -O2 -o betachess board.h board.cpp main.cpp

# TODO fix this because time masks the failure of g++
if [[ $? ]]; then
  echo
  echo
  echo
  time ./betachess
fi
