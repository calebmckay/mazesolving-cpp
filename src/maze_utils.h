#include "bitmap_image.hpp"

#include <forward_list>
#include <string>

#include <gtest/gtest_prod.h>

const unsigned long int MAX_DISTANCE_FROM_EXIT = 0xffffffff;

namespace mazeUtils {
  class MazeNetwork {
    public:
      enum Direction {
        north,
        south,
        east,
        west
      };

      class Node {
        public:
          Node();
          ~Node();
          std::size_t getX();
          std::size_t getY();
          Node* getNeighbor(Direction direction);
          unsigned long int getDistance();

          void setLocation(std::size_t x, std::size_t y);
          void setNeighbor(Node* node, Direction direction);
          void setDistance(unsigned long int distance);
          std::string toString();
        private:
          std::size_t x, y = 0;
          Node* north = NULL;
          Node* south = NULL;
          Node* east = NULL;
          Node* west = NULL;
          unsigned long int distanceFromExit = MAX_DISTANCE_FROM_EXIT;
      };

      MazeNetwork();
      MazeNetwork(std::string filePath);
      ~MazeNetwork();

      int parseImage(std::string filePath);
      std::string toString();
    private:
      FRIEND_TEST(MazeUtilTest, verifyShouldCreateNode);
      std::forward_list<Node*> nodeDb;
      Node* start = NULL;
      Node* end = NULL;

      static bool isWhite(rgb_t pixel);
      static bool shouldCreateNode(rgb_t n, rgb_t s, rgb_t e, rgb_t w);
      static bool shouldCreateNode(bool n, bool s, bool e, bool w);
      Node* addNode(std::size_t x, std::size_t y);
      void calculateDistances();
  };
}