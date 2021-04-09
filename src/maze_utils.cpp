#include "maze_utils.h"

#include "bitmap_image.hpp"

#include <forward_list>
#include <map>
#include <sstream>

#ifdef DEBUG
#define LOG(x) std::cout << x << std::endl;
#else
#define LOG(x)
#endif

namespace mazeUtils {
  MazeNetwork::Node::Node() {}
  MazeNetwork::Node::~Node() {}

  std::size_t MazeNetwork::Node::getX() {
    return this->x;
  }

  std::size_t MazeNetwork::Node::getY() {
    return this->y;
  }

  MazeNetwork::Node* MazeNetwork::Node::getNeighbor(Direction direction) {
    switch (direction) {
      case MazeNetwork::north:
        return this->north;
      case MazeNetwork::south:
        return this->south;
      case MazeNetwork::east:
        return this->east;
      case MazeNetwork::west:
        return this->west;
    }
    return NULL;
  }

  unsigned long int MazeNetwork::Node::getDistance() {
    return this->distanceFromExit;
  }

  void MazeNetwork::Node::setLocation(std::size_t x, std::size_t y) {
    this->x = x;
    this->y = y;
  }

  void MazeNetwork::Node::setNeighbor(MazeNetwork::Node* node, MazeNetwork::Direction direction) {
    switch (direction) {
      case MazeNetwork::north:
        this->north = node;
        break;
      case MazeNetwork::south:
        this->south = node;
        break;
      case MazeNetwork::east:
        this->east = node;
        break;
      case MazeNetwork::west:
        this->west = node;
        break;
    }
  }

  void MazeNetwork::Node::setDistance(unsigned long int distance) {
    this->distanceFromExit = distance;
  }

  std::string MazeNetwork::Node::toString() {
    std::ostringstream oss;
    oss << "Node " << std::hex << this << std::endl;
    oss << "x:" << std::dec << this->x << " y:" << this->y << std::endl;
    if (north != NULL) {
      oss << "north: " << std::hex << north << std::endl;
    }
    if (south != NULL) {
      oss << "south: " << std::hex << south << std::endl;
    }
    if (east != NULL) {
      oss << "east: " << std::hex << east << std::endl;
    }
    if (west != NULL) {
      oss << "west: " << std::hex << west << std::endl;
    }
    oss << "a*: " << std::dec << distanceFromExit << std::endl;
    return oss.str();
  }

  MazeNetwork::MazeNetwork() {}

  MazeNetwork::~MazeNetwork() {
    // Iterate through every node and delete it from the heap
    for (auto it = nodeDb.begin(); it != nodeDb.end(); it++) {
      delete *it;
    }
    // Now that the nodes have been deleted, clear the list
    nodeDb.clear();
  }

  MazeNetwork::MazeNetwork(std::string filePath) {
    this->parseImage(filePath);
  }

  int MazeNetwork::parseImage(std::string filePath) {
    // TODO - Import and parse maze image
    bitmap_image image(filePath);
    if (!image)
    {
      std::cout << "Error - Failed to open: " << filePath << std::endl;
      return 1;
    }

    const unsigned int height = image.height();
    const unsigned int width  = image.width();

    // As we parse from left-to-right, we keep track of
    // the last node to our left that has an open space
    // to its right (a potential connection to our west).
    Node* westNeighbor = NULL;
    // As we next parse top-to-bottom, we keep track of
    // any nodes in any column that have open spaces
    // beneath them (a potential connection to our north).
    // This will be a map of column number (x) to the Node
    // itself.
    typedef std::map<std::size_t, Node*> northNeighborMap;
    northNeighborMap northNeighbors;

    // RGB values of this pixel and any neighboring pixels
    rgb_t thisPixel, nPixel, sPixel, ePixel, wPixel;

    for (std::size_t y = 0; y < height; ++y) {
      for (std::size_t x = 0; x < width; ++x) {
        // Look for a blank space
        image.get_pixel(x, y, thisPixel);
        if (!isWhite(thisPixel)) continue;

        // For the first row, set the entrance
        if (y == 0) {
          Node* thisNode = addNode(x,y);
          this->start = thisNode;
          northNeighbors[x] = thisNode;
          break;
        }

        // For the last row, set the exit
        if (y == height-1) {
          Node* thisNode = addNode(x,y);
          this->end = thisNode;
          break;
        }
        
        // For any other row, check the neighboring pixels
        image.get_pixel(x, y-1, nPixel);
        image.get_pixel(x, y+1, sPixel);
        image.get_pixel(x+1, y, ePixel);
        image.get_pixel(x-1, y, wPixel);
        if (shouldCreateNode(nPixel, sPixel, ePixel, wPixel)) {
          Node* thisNode = addNode(x,y);

          // If there's a space to the left and a previous neighbor, connect them.
          if (isWhite(wPixel) && westNeighbor != NULL) {
            thisNode->setNeighbor(westNeighbor, west);
            westNeighbor->setNeighbor(thisNode, east);
            westNeighbor = NULL;
          }
          // If there's a space to the north and a valid north neighbor, connect them.
          if (isWhite(nPixel)) {
            // Check for a north neighbor
            Node* northNeighbor = NULL;
            try {
              northNeighbor = northNeighbors.at(x);
            } catch (std::out_of_range&) {
              northNeighbor = NULL;
            }

            // If we found one, connect them.
            if (northNeighbor != NULL) {
              thisNode->setNeighbor(northNeighbor, north);
              northNeighbor->setNeighbor(thisNode, south);
            }
            // Regardless of whether we found one, make sure we clear this northNeighbor
            northNeighbors.erase(x);
          }
          // If there's a space to the east, set ourselves as a westNeighbor
          if (isWhite(ePixel)) {
            westNeighbor = thisNode;
          }
          // If there's a space to the south, set ourselves as a northNeighbor
          if (isWhite(sPixel)) {
            northNeighbors[x] = thisNode;
          }
        }
      }

      // At the end of the row, clear our westNeighbor
      // TODO - We might want an integrity check here in case we have any "open" rows
      // that are looking for an east connection but don't have one
      westNeighbor = NULL;
    }
    // Go back through all the nodes and calculate the distance from
    // the exit for each one.
    calculateDistances();

    // Return success
    return 0;
  }

  std::string MazeNetwork::toString() {
    std::ostringstream oss;
    unsigned long int nodeCount = 0;
    for (auto it = nodeDb.begin(); it != nodeDb.end(); it++) {
      oss << (*it)->toString() << std::endl;
      nodeCount++;
    }
    oss << "---------------------" << std::endl;
    oss << "Node count: " << nodeCount << std::endl;
    return oss.str();
  }

  bool MazeNetwork::isWhite(rgb_t pixel) {
    return (pixel.red == 255 && pixel.green == 255 && pixel.blue == 255);
  }

  bool MazeNetwork::shouldCreateNode(rgb_t n, rgb_t s, rgb_t e, rgb_t w) {
    return shouldCreateNode(isWhite(n), isWhite(s), isWhite(e), isWhite(w));
  }

  bool MazeNetwork::shouldCreateNode(bool n, bool s, bool e, bool w) {
    // There are three cases when we don't want to make a node:
    // 1. No neighboring spaces (we're in a hole? probably should be concerned in this case)
    if (!n && !s && !e && !w) return false;
    // 2. We're in a north-south passageway
    if (n && s && !e && !w) return false;
    // 3. We're in an east-west passageway
    if (!n && !s && e && w) return false;
    // Every other case (dead ends, corners, 3-way and 4-way intersections) need a node
    return true;
  }

  MazeNetwork::Node* MazeNetwork::addNode(std::size_t x, std::size_t y) {
    // Create a node and add it to the heap
    Node* myNode = new Node();
    // Add it to nodeDb for destruction later
    nodeDb.push_front(myNode);
    // Set the location specified by the args
    myNode->setLocation(x,y);

    return myNode;
  }

  void MazeNetwork::calculateDistances() {
    std::size_t endX = this->end->getX();
    std::size_t endY = this->end->getY();
    for (auto it = nodeDb.begin(); it != nodeDb.end(); it++) {
      std::size_t x = (*it)->getX();
      std::size_t y = (*it)->getY();

      // Get the difference in x and y values
      std::size_t xDiff = (x > endX ? x - endX : endX - x);
      std::size_t yDiff = (y > endY ? y - endY : endY - y);

      // Use the Pythagorean theorem to calculate the distance.
      // To avoid decimals, we don't do the final sqrt.
      unsigned long int distanceSquared = (xDiff * xDiff) + (yDiff * yDiff);
      (*it)->setDistance(distanceSquared);
    }
  }
}