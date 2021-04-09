#include "gtest/gtest.h"

#include "maze_utils.h"

class SampleTest : public ::testing::Test {
    protected:
    // You can remove any or all of the following functions if their bodies would
    // be empty.

    SampleTest() {
        // You can do set-up work for each test here.
    }

    ~SampleTest() override {
        // You can do clean-up work that doesn't throw exceptions here.
    }

    // If the constructor and destructor are not enough for setting up
    // and cleaning up each test, you can define the following methods:

    void SetUp() override {
        // Code here will be called immediately after the constructor (right
        // before each test).
    }

    void TearDown() override {
        // Code here will be called immediately after each test (right
        // before the destructor).
    }

    // Class members declared here can be used by all tests in the test suite
    // for Foo.
};

TEST(SampleTest, GTestIsConfiguredCorrectly) {
    ASSERT_TRUE(true);
}

namespace mazeUtils {
    class MazeUtilTest : public ::testing::Test {};
    TEST(MazeUtilTest, verifyNodeGettersAndSetters) {
        mazeUtils::MazeNetwork::Node node1, node2;

        node1.setLocation(123, 456);
        node1.setDistance(54321);
        node1.setNeighbor(&node2, MazeNetwork::north);

        EXPECT_EQ(node1.getX(), 123);
        EXPECT_EQ(node1.getY(), 456);
        EXPECT_EQ(node1.getDistance(), 54321);
        EXPECT_EQ(node1.getNeighbor(MazeNetwork::north), &node2);
    }

    TEST(MazeUtilTest, verifyShouldCreateNode) {
        mazeUtils::MazeNetwork mazeNetwork;
        typedef bool truthValues[4];
        typedef std::pair<unsigned char, bool> testCase;
        std::vector<testCase> testCases = {
            //  NSEW  return
            { 0b0000, false },
            { 0b0001,  true },
            { 0b0010,  true },
            { 0b0011, false },
            { 0b0100,  true },
            { 0b0101,  true },
            { 0b0110,  true },
            { 0b0111,  true },
            { 0b1000,  true },
            { 0b1001,  true },
            { 0b1010,  true },
            { 0b1011,  true },
            { 0b1100, false },
            { 0b1101,  true },
            { 0b1110,  true },
            { 0b1111,  true }
        };
        for (auto it = testCases.begin(); it != testCases.end(); it++) {
            unsigned int values = it->first;
            bool n = (values & 0b1000) >> 3;
            bool s = (values & 0b0100) >> 2;
            bool e = (values & 0b0010) >> 1;
            bool w = (values & 0b0001) >> 0;
            EXPECT_EQ(mazeNetwork.shouldCreateNode(n,s,e,w), it->second) << "n" << n << " s" << s << " e" << e << " w" << w;
        }
    }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}