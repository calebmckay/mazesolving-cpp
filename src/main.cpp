#include "maze_utils.h"

#include <iostream>

int main() {
  mazeUtils::MazeNetwork maze("./maze.bmp");
  std::cout << maze.toString() << std::endl;
  return 0;
}