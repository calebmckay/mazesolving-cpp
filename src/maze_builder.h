#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace mazeBuilder {
    class IMazeBuilder {
        public:
            IMazeBuilder() {}
            IMazeBuilder(unsigned long seed) {}
            IMazeBuilder(unsigned long seed,
                        unsigned int xSize,
                        unsigned int ySize) {}

            virtual ~IMazeBuilder() {}

            virtual void makeImage(std::string fileName) = 0;
    };

    class DepthFirstBuilder : public IMazeBuilder {
        public:
            DepthFirstBuilder(
                unsigned long seed = time(NULL),
                unsigned int xSize = 21,
                unsigned int ySize = 21
            );

            virtual ~DepthFirstBuilder() {}

            virtual void makeImage(std::string fileName = "maze.bmp");
        private:
            unsigned int xSize, ySize; // Width of maze in pixels (including border)
            unsigned int xPixels, yPixels; // Width of maze in pixels (without border)
            unsigned int xCells, yCells; // Width of maze in cells
            unsigned int xStart, xEnd; // Column number for start/end of maze
            enum directions {
                north,
                east,
                south,
                west
            };

            inline directions invertDirection(directions dir) {
                switch(dir) {
                    case north:
                        return south;
                    case east:
                        return west;
                    case south:
                        return north;
                    case west:
                        return east;
                    default:
                        throw std::out_of_range("Unexpected value for direction");
                        break;
                }
            }

            class Cell {
                public:
                    Cell();
                    Cell(unsigned int x = 0, unsigned int y = 0);
                
                    unsigned int x, y;
                    bool wall;
                    bool visited;
                    Cell* prev;
                    std::map<directions, Cell*> connections;
            };

            typedef std::vector<std::vector<Cell>> mazeType;
            mazeType mazeCells;
            
            void buildMaze(unsigned long seed);
            void printMaze(int currXCell = -1, int currYCell = -1);
    };
}