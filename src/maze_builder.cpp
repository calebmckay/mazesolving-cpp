#include <chrono>
#include <cstdlib>
#include <string>
#include <thread>

#include "maze_builder.h"

#include "bitmap_image.hpp"

#ifdef DEBUG
#define LOG(x) std::cout << x << std::endl;
#else
#define LOG(x)
#endif

namespace mazeBuilder {
    DepthFirstBuilder::DepthFirstBuilder(
        unsigned long seed,
        unsigned int xSize,
        unsigned int ySize
    )
    : xSize(xSize),
    ySize(ySize)              
    {
        buildMaze(seed);
    }

    void DepthFirstBuilder::makeImage(std::string fileName) {
        auto t1 = std::chrono::high_resolution_clock::now();
        typedef std::vector<std::vector<bool>> pixels; // true = white, false = black
        pixels mazePixels;
        // Initialize pixel map
        for (unsigned int x = 0; x < xSize; x++) {
            std::vector<bool> tmpRow;
            for (unsigned int y = 0; y < ySize; y++) {
                tmpRow.push_back(false);
            }
            mazePixels.push_back(tmpRow);
        }

        // First row - add start of maze
        mazePixels[xStart][0] = true;

        // Maze rows
        for (unsigned int xCell = 0; xCell < xCells; xCell++) {
            for (unsigned int yCell = 0; yCell < yCells; yCell++) {
                // Cells are technically 2x2 pixel squares. Start with the top left
                // pixel of each square, and derive the others based on that cell's
                // connections to adjacent cells.
                unsigned int xPixel = (xCell * 2) + 1;
                unsigned int yPixel = (yCell * 2) + 1;
                if (mazeCells[xCell][yCell].visited)
                {
                    // If it is not a wall, it is part of the maze path
                    mazePixels[xPixel][yPixel] = true;

                    // Now check the connections below and to the right of this cell,
                    // and color the pixels accordingly. Don't do this on the last
                    // cell row/column.
                    if (xCell < xCells - 1) {
                        // Not the last column - check to the right
                        if (mazeCells[xCell][yCell].connections[east] == &mazeCells[xCell+1][yCell]
                        && mazeCells[xCell+1][yCell].connections[west] == &mazeCells[xCell][yCell]) {
                            mazePixels[xPixel+1][yPixel] = true;
                        }
                    }
                    if (yCell < yCells - 1) {
                        // Not the last row - check below
                        if (mazeCells[xCell][yCell].connections[south] == &mazeCells[xCell][yCell+1]
                        && mazeCells[xCell][yCell+1].connections[north] == &mazeCells[xCell][yCell]) {
                            mazePixels[xPixel][yPixel+1] = true;
                        }
                    }
                }
            }
        }

        // Last row(s) (this will hit the same row twice if there is only one border row)
        mazePixels[xEnd][2*yCells] = true; // Set the row just below the maze
        mazePixels[xEnd][ySize-1] = true; // Set last row


        // Draw our pixel map to an image
        bitmap_image image(xSize, ySize);
        image.set_all_channels(0,0,0);

        image_drawer draw(image);
        draw.pen_color(255,255,255);

        for (std::size_t x = 0; x < xSize; x++) {
            for (std::size_t y = 0; y < ySize; y++) {
                if (mazePixels[x][y]) {
                    draw.plot_pixel(x,y);
                }
            }
        }

        image.save_image(fileName);
        auto t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count() / 1000000000.0;
        std::cout << "Created image: " << duration << " seconds" << std::endl;
    }

    DepthFirstBuilder::Cell::Cell(unsigned int x, unsigned int y) 
    : x(x),
    y(y),
    wall(true),
    visited(false),
    prev(NULL)
    {
        connections[north] = NULL;
        connections[east] = NULL;
        connections[south] = NULL;
        connections[west] = NULL;
    }
            
    void DepthFirstBuilder::buildMaze(unsigned long seed) {
        std::cout << "==========================" << std::endl;
        std::cout << " Depth-First Maze Builder" << std::endl;
        std::cout << "==========================" << std::endl;
        auto t1 = std::chrono::high_resolution_clock::now();
        // Set our random seed
        std::srand(seed);

        LOG("buildMaze - initializing");
        // Create our structure of cells. Since walls are 1 pixel wide, we set up
        // a cell every 2 pixels. The border of the maze is also 1 pixel wide (except
        // if a size variable is even, then we'll make it 2 pixels so the maze size comes
        // out correctly)
        const unsigned int CELL_BORDER_TOTAL_WIDTH = 2;
        if(xSize <= CELL_BORDER_TOTAL_WIDTH || ySize <= CELL_BORDER_TOTAL_WIDTH) {
            throw std::invalid_argument("Maze is too small to generate");
        }
        xCells = std::ceil((xSize - CELL_BORDER_TOTAL_WIDTH)/2) + 1;
        yCells = std::ceil((ySize - CELL_BORDER_TOTAL_WIDTH)/2) + 1;
        xPixels = (2 * xCells) - 1;
        yPixels = (2 * yCells) - 1;

        // Set values for start/end column. These are not stored as cells, just pixel values.
        // Values are +1 because the maze starts at x=1.
        xStart = (std::rand() % xCells) * 2 + 1;
        xEnd = (std::rand() % xCells) * 2 + 1;

        // Initialize our grid of cells
        for (unsigned int x = 0; x < xCells; x++) {
            std::vector<Cell> tmpColumn;
            for (unsigned int y = 0; y < yCells; y++) {
                tmpColumn.push_back(Cell(x,y));
            }
            mazeCells.push_back(tmpColumn);
        }

        // Now start traversing via depth-first search
        Cell* next = &(mazeCells[0][0]);
        Cell* prev = NULL;
        std::vector<directions> options;
        next->visited = true;

        do {
            // DEBUG
            LOG("buildMaze - main loop")
            if (prev != NULL) {
                LOG("prev: x " << prev->x << " y " << prev->y);
            }
            LOG("next: x " << next->x << " y " << next->y);
            // Decide which directions we can go
            options.clear();
            if (next->y > 0 && !mazeCells[next->x][next->y - 1].visited) {
                options.push_back(north);
            }
            if (next->x < xCells - 1 && !mazeCells[next->x + 1][next->y].visited) {
                options.push_back(east);
            }
            if (next->y < yCells - 1 && !mazeCells[next->x][next->y + 1].visited) {
                options.push_back(south);
            }
            if (next->x > 0 && !mazeCells[next->x - 1][next->y].visited) {
                options.push_back(west);
            }
            LOG("choices - " << options.size());

            // Check if we're at a dead end
            if (options.size() == 0) {
                // Backtrack to the previous cell
                LOG("Dead end - backtracking...");
                next = prev;
                if (prev != NULL) {
                   prev = prev->prev;
                }
#ifdef DEBUG
                printMaze(next->x, next->y);
                std::this_thread::sleep_for (std::chrono::seconds(1));
#endif
                continue;
            }

            // Pick a random direction
            directions choice = options[std::rand() % options.size()];
            // And now go in that direction
            prev = next;
            switch (choice) {
                case north:
                    next = &mazeCells[next->x][next->y - 1];
                    LOG("going north");
                    break;
                case east:
                    next = &mazeCells[next->x + 1][next->y];
                    LOG("going east");
                    break;
                case south:
                    next = &mazeCells[next->x][next->y + 1];
                    LOG("going south");
                    break;
                case west:
                    next = &mazeCells[next->x - 1][next->y];
                    LOG("going west");
                    break;
                default:
                    throw std::out_of_range("Unexpected value for direction");
                    break;
            }
            prev->connections[choice] = next;
            next->connections[invertDirection(choice)] = prev;
            next->prev = prev;
            next->visited = true;
#ifdef DEBUG
            printMaze(next->x, next->y);
            std::this_thread::sleep_for (std::chrono::seconds(1));
#endif
        } while (prev != NULL);
        auto t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count() / 1000000000.0;
        std::cout << "Built maze: " << duration << " seconds" << std::endl;
        std::cout << "  Seed: " << seed << std::endl;
        std::cout << "  Size: " << xSize << " x " << ySize << std::endl;
        std::cout << "  Pixels: " << xSize * ySize << std::endl;
        std::cout << "  Cells: " << xCells * yCells << std::endl;
    }

    void DepthFirstBuilder::printMaze(int currXCell, int currYCell) {
        typedef std::vector<std::vector<bool>> pixels; // true = path, false = wall
        pixels mazePixels;
        // Initialize pixel map
        for (unsigned int x = 0; x < xPixels; x++) {
            std::vector<bool> tmpRow;
            for (unsigned int y = 0; y < yPixels; y++) {
                tmpRow.push_back(false);
            }
            mazePixels.push_back(tmpRow);
        }

        // Maze rows
        for (unsigned int xCell = 0; xCell < xCells; xCell++) {
            for (unsigned int yCell = 0; yCell < yCells; yCell++) {
                // Cells are technically 2x2 pixel squares. Start with the top left
                // pixel of each square, and derive the others based on that cell's
                // connections to adjacent cells.
                unsigned int xPixel = (xCell * 2);
                unsigned int yPixel = (yCell * 2);
                if (mazeCells[xCell][yCell].visited)
                {
                    // If it is not a wall, it is part of the maze path
                    mazePixels[xPixel][yPixel] = true;

                    // Now check the connections below and to the right of this cell,
                    // and color the pixels accordingly. Don't do this on the last
                    // cell row/column.
                    if (xCell < xCells - 1) {
                        // Not the last column - check to the right
                        if (mazeCells[xCell][yCell].connections[east] == &mazeCells[xCell+1][yCell]
                        && mazeCells[xCell+1][yCell].connections[west] == &mazeCells[xCell][yCell]) {
                            mazePixels[xPixel+1][yPixel] = true;
                        }
                    }
                    if (yCell < yCells - 1) {
                        // Not the last row - check below
                        if (mazeCells[xCell][yCell].connections[south] == &mazeCells[xCell][yCell+1]
                        && mazeCells[xCell][yCell+1].connections[north] == &mazeCells[xCell][yCell]) {
                            mazePixels[xPixel][yPixel+1] = true;
                        }
                    }
                }
            }
        }

        // Print the maze path 
        for (std::size_t y = 0; y < yPixels; y++) {
            for (std::size_t x = 0; x < xPixels; x++) {
                if (x != -1 && y != -1 && x == currXCell*2 && y == currYCell*2) {
                    std::cout << "X";
                } else if (mazePixels[x][y]) {
                    std::cout << "#";
                } else {
                    std::cout << " ";
                }
            }
            std::cout << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    // Parse arguments
    unsigned long seed = time(NULL);
    unsigned int width = 21;
    unsigned int height = 21;

    for (int i = 1; i < argc; i++) {
        std::string arg(argv[i]);
        try {
            if (arg == "-s" || arg == "--seed") {
                seed = std::stol(std::string(argv[++i]));
            }
            else if (arg == "-w" || arg == "--width") {
                width = std::stoi(std::string(argv[++i]));
            }
            else if (arg == "-h" || arg == "--height") {
                height = std::stoi(std::string(argv[++i]));
            }
        } catch (std::invalid_argument const& e) {
            std::cerr << "Invalid number: " << argv[i] << std::endl;
        } catch (std::out_of_range const& e) {
            std::cerr << "Number out of range: " << argv[i] << std::endl;
        }
    }

    mazeBuilder::DepthFirstBuilder maze(seed, width, height);
    maze.makeImage("maze.bmp");
    std::cout << "Maze created!" << std::endl;
    return 0;
}