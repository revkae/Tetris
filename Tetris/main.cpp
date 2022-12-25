#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <time.h>
#include <stdlib.h>

#define WIDTH 400 * 2
#define HEIGHT 700

#define ROW_COUNT 28
#define COLUMN_COUNT 16

#define LINE_COUNT 14

#define UP -16
#define DOWN 16
#define RIGHT 1
#define LEFT -1

void SetWalls(std::vector<sf::RectangleShape>& grids, std::vector<int>& walls);
void SetGrounds(std::vector<sf::RectangleShape>& grounds);
void SetGrids(std::vector<sf::RectangleShape>& grids);

int rotationLevel = 0;

sf::Color BLOCK_COLOR = sf::Color(32, 82, 149);
sf::Color STOPPED_BLOCK_COLOR = sf::Color(20, 66, 114);
sf::Color BORDER_COLOR = sf::Color(10, 38, 71);
sf::Color GROUND_COLOR = sf::Color(0, 43, 91);
sf::Color NOTHING_COLOR = sf::Color::Transparent;

sf::Music theme;
sf::Sound success;
sf::Sound blockdrop;
sf::Sound borderhit;
sf::Sound rotateSound;
sf::Sound gameover;
sf::Sound buttonPressSound;
sf::Sound buttonReleaseSound;

struct Score {
    int LINE_SCORE = 0;
    int BLOCK_SCORE = 0;
};
Score scoreKeper;

enum GameState
{
    MAIN_MENU,
    GAME,
    END_SCREEN
};
GameState state = GameState::MAIN_MENU;

class Blocks
{
public:
    int lineScore = 0;
    int blockScore = 0;
    
    float speed = 0.9f;
private:
    enum BlockType
    {
        O_type,
        I_type,
        S_type,
        Z_type,
        L_type,
        J_type,
        T_type
    };
    BlockType blockType;

    std::array<int, 14> groundNums;
    std::array<int, 14> topNums;
    std::array<int, ROW_COUNT> leftWall;
    std::array<int, ROW_COUNT> rightWall;
    std::array<int, ROW_COUNT * 2> walls;

public:
    Blocks(std::vector<sf::RectangleShape>& grids) {
       
        LoadGroundNums(grids);
        LoadTopNums(grids);
    };

    void Restart(std::vector<sf::RectangleShape>& grids) {
        /* restarting scores */
        lineScore = 0;
        blockScore = 0;
        /* restaring direction */
        dir = 0;
        /* restarting blocks */
        {
        blocks.clear();     
        O.clear();
        I.clear();
        S.clear();
        Z.clear();
        L.clear();
        J.clear();
        T.clear();
        }
        /* restarting clocks */
        clock.restart();
        settleClock.restart();
        /* Clearing blocks */
        for (auto& stopped : stoppedBlocks)
        {
            for (auto& i : stopped)
            {
                grids.at(i).setFillColor(NOTHING_COLOR);
            }
        }
        for (auto& grid : grids) {
            if (grid.getFillColor() == BLOCK_COLOR) {
                grid.setFillColor(NOTHING_COLOR);
            }
        }
        stoppedBlocks.clear();
    }

    void input(int newDir) {
        this->dir = newDir;
    }

    void setUpWalls(std::vector<int>& walls) {
        for (size_t i = 0; i < ROW_COUNT * 2; i++)
        {
            this->walls[i] = walls.at(i);
            if (i > 27) {
                rightWall[i - ROW_COUNT] = walls.at(i);
            }
            else {
                leftWall[i] = walls.at(i);
            }
        }
    }
   
    bool canRotate(std::vector<int>& newPoses, std::vector<sf::RectangleShape>& grids) {
        for (auto& j : newPoses)
        {
            if (j < 0) {
                return false;
            }
            for (auto& i : rightWall) {
                if (i == j) {
                    borderhit.play();
                    return false;
                }
            }
            for (auto& i : leftWall) {
                if (i == j) {
                    borderhit.play();
                    return false;
                }
            }
            if (grids.at(j).getFillColor() == STOPPED_BLOCK_COLOR) {
                borderhit.play();
                return false;
            }

            if (grids.at(j).getFillColor() == GROUND_COLOR) {
                borderhit.play();
                return false;
            }
        }
        return true;
    }
    
    void rotate(std::vector<sf::RectangleShape>& grids) {
        /* T */
        rotateSound.play();
        if (blockType == T_type) {
            /* Checking if rotating is possible based on walls */
            std::vector<int> newPoses;
            for (auto& block : blocks) {
                if (rotationLevel == 0) {
                    /* Controlling next rotation - 1 rotation*/
                    newPoses.push_back(block[0] - 16);
                    newPoses.push_back(block[0] + 1);
                    newPoses.push_back(block[0] + 16);
                }
                else if (rotationLevel == 1) {
                    /* Controlling next rotation - 2 rotation*/
                    newPoses.push_back(block[0] - 1);
                    newPoses.push_back(block[0] + 1);
                    newPoses.push_back(block[0] + 16);
                }
                else if (rotationLevel == 2) {
                    /* Controlling next rotation - 3 rotation*/
                    newPoses.push_back(block[0] - 16);
                    newPoses.push_back(block[0] - 1);
                    newPoses.push_back(block[0] + 16);
                }
                else if (rotationLevel == 3) {
                    /* Controlling next rotation - 0 rotation*/
                    newPoses.push_back(block[0] - 16);
                    newPoses.push_back(block[0] + 1);
                    newPoses.push_back(block[0] - 1);
                }
            }
            if (canRotate(newPoses, grids)) {
                if (rotationLevel == 3) {
                    rotationLevel = -1;
                }
                rotationLevel += 1;
                newPoses.clear();
                
            }
            else {
                newPoses.clear();
            }
            
            /* Rotating based on level */
            if (rotationLevel == 0) {
                for (auto& block : blocks) {
                    for (int i = 0; i < block.size(); i++) {
                        grids.at(block[i]).setFillColor(NOTHING_COLOR);
                    }
                    for (int i = 0; i < block.size(); i++) {
                        block[1] = block[0] - 16;
                        block[2] = block[0] + 1;
                        block[3] = block[0] - 1;
                        grids.at(block[0]).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] - 16).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] + 1).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] - 1).setFillColor(BLOCK_COLOR);
                    }
                }
            }
            else if (rotationLevel == 1) {
                for (auto& block : blocks) {
                    for (int i = 0; i < block.size(); i++) {
                        grids.at(block[i]).setFillColor(NOTHING_COLOR);
                    }
                    for (int i = 0; i < block.size(); i++) {
                        block[1] = block[0] - 16;
                        block[2] = block[0] + 1;
                        block[3] = block[0] + 16;
                        grids.at(block[0]).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] - 16).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] + 1).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] + 16).setFillColor(BLOCK_COLOR);
                    }      
                }
            }
            else if (rotationLevel == 2) {
                for (auto& block : blocks) {
                    for (int i = 0; i < block.size(); i++) {
                        grids.at(block[i]).setFillColor(NOTHING_COLOR);
                    }
                    for (int i = 0; i < block.size(); i++) {
                        block[1] = block[0] - 1;
                        block[2] = block[0] + 1;
                        block[3] = block[0] + 16;
                        grids.at(block[0]).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] - 1).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] + 1).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] + 16).setFillColor(BLOCK_COLOR);
                    }
                }
            }
            else if (rotationLevel == 3) {
                for (auto& block : blocks) {
                    for (int i = 0; i < block.size(); i++) {
                        grids.at(block[i]).setFillColor(NOTHING_COLOR);
                    }
                    for (int i = 0; i < block.size(); i++) {
                        block[1] = block[0] - 16;
                        block[2] = block[0] - 1;
                        block[3] = block[0] + 16;
                        grids.at(block[0]).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] - 16).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] - 1).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] + 16).setFillColor(BLOCK_COLOR);
                    }
                }
            }
        }
        /* J */
        if (blockType == J_type) {
            /* Checking if rotating is possible based on walls */
            std::vector<int> newPoses;
            for (auto& block : blocks) {
                if (rotationLevel == 0) {
                    /* Controlling next rotation - 1 rotation*/
                    newPoses.push_back(block[0] - 17);
                    newPoses.push_back(block[0] + 1);
                    newPoses.push_back(block[0] - 1);
                }
                else if (rotationLevel == 1) {
                    /* Controlling next rotation - 2 rotation*/
                    newPoses.push_back(block[0] - 16);
                    newPoses.push_back(block[0] - 15);
                    newPoses.push_back(block[0] + 16);
                }
                else if (rotationLevel == 2) {
                    /* Controlling next rotation - 3 rotation*/
                    newPoses.push_back(block[0] + 1);
                    newPoses.push_back(block[0] - 1);
                    newPoses.push_back(block[0] + 17);
                }
                else if (rotationLevel == 3) {
                    /* Controlling next rotation - 0 rotation*/
                    newPoses.push_back(block[0] - 32);
                    newPoses.push_back(block[0] - 16);
                    newPoses.push_back(block[0] - 1);
                }
            }
            if (canRotate(newPoses, grids)) {
                rotationLevel += 1;
                newPoses.clear();
            }
            else {
                newPoses.clear();
            }

            if (rotationLevel == 0) {
                for (auto& block : blocks) {
                    for (int i = 0; i < block.size(); i++) {
                        grids.at(block[i]).setFillColor(NOTHING_COLOR);
                    }
                    for (int i = 0; i < block.size(); i++) {
                        block[1] = block[0] - 32;
                        block[2] = block[0] - 16;
                        block[3] = block[0] - 1;
                        grids.at(block[0]).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] - 32).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] - 16).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] - 1).setFillColor(BLOCK_COLOR);
                    }
                }
            }
            else if (rotationLevel == 1) {
                for (auto& block : blocks) {
                    for (int i = 0; i < block.size(); i++) {
                        grids.at(block[i]).setFillColor(NOTHING_COLOR);
                    }
                    for (int i = 0; i < block.size(); i++) {
                        block[1] = block[0] - 17;
                        block[2] = block[0] + 1;
                        block[3] = block[0] - 1;
                        grids.at(block[0]).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] - 17).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] + 1).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] - 1).setFillColor(BLOCK_COLOR);
                    }
                }
            }
            else if (rotationLevel == 2) {
                for (auto& block : blocks) {
                    for (int i = 0; i < block.size(); i++) {
                        grids.at(block[i]).setFillColor(NOTHING_COLOR);
                    }
                    for (int i = 0; i < block.size(); i++) {
                        block[1] = block[0] - 16;
                        block[2] = block[0] - 15;
                        block[3] = block[0] + 16;
                        grids.at(block[0]).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] - 16).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] - 15).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] + 16).setFillColor(BLOCK_COLOR);
                    }
                }
            }
            else if (rotationLevel == 3) {
                for (auto& block : blocks) {
                    for (int i = 0; i < block.size(); i++) {
                        grids.at(block[i]).setFillColor(NOTHING_COLOR);
                    }
                    for (int i = 0; i < block.size(); i++) {
                        block[1] = block[0] + 1;
                        block[2] = block[0] - 1;
                        block[3] = block[0] + 17;
                        grids.at(block[0]).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] + 1).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] - 1).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] + 17).setFillColor(BLOCK_COLOR);
                    }
                }
                rotationLevel = -1;
            }
        }
        /* L */
        if (blockType == L_type) {
            /* Checking if rotating is possible based on walls */
            std::vector<int> newPoses;
            for (auto& block : blocks) {
                if (rotationLevel == 0) {
                    /* Controlling next rotation - 1 rotation*/
                    newPoses.push_back(block[0] - 1);
                    newPoses.push_back(block[0] + 1);
                    newPoses.push_back(block[0] + 15);
                }
                else if (rotationLevel == 1) {
                    /* Controlling next rotation - 2 rotation*/
                    newPoses.push_back(block[0] - 16);
                    newPoses.push_back(block[0] - 17);
                    newPoses.push_back(block[0] + 16);
                }
                else if (rotationLevel == 2) {
                    /* Controlling next rotation - 3 rotation*/
                    newPoses.push_back(block[0] + 1);
                    newPoses.push_back(block[0] - 1);
                    newPoses.push_back(block[0] - 15);
                }
                else if (rotationLevel == 3) {
                    /* Controlling next rotation - 0 rotation*/
                    newPoses.push_back(block[0] + 16);
                    newPoses.push_back(block[0] - 16);
                    newPoses.push_back(block[0] + 17);
                }
            }
            if (canRotate(newPoses, grids)) {
                rotationLevel += 1;
                newPoses.clear();
            }
            else {
                newPoses.clear();
            }

            if (rotationLevel == 0) {
                for (auto& block : blocks) {
                    for (int i = 0; i < block.size(); i++) {
                        grids.at(block[i]).setFillColor(NOTHING_COLOR);
                    }
                    for (int i = 0; i < block.size(); i++) {
                        block[1] = block[0] + 16;
                        block[2] = block[0] - 16;
                        block[3] = block[0] + 17;
                        grids.at(block[0]).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] + 16).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] - 16).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] + 17).setFillColor(BLOCK_COLOR);
                    }
                }
            }
            else if (rotationLevel == 1) {
                for (auto& block : blocks) {
                    for (int i = 0; i < block.size(); i++) {
                        grids.at(block[i]).setFillColor(NOTHING_COLOR);
                    }
                    for (int i = 0; i < block.size(); i++) {
                        block[1] = block[0] - 1;
                        block[2] = block[0] + 1;
                        block[3] = block[0] + 15;
                        grids.at(block[0]).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] - 1).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] + 1).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] + 15).setFillColor(BLOCK_COLOR);
                    }
                }
            }
            else if (rotationLevel == 2) {
                for (auto& block : blocks) {
                    for (int i = 0; i < block.size(); i++) {
                        grids.at(block[i]).setFillColor(NOTHING_COLOR);
                    }
                    for (int i = 0; i < block.size(); i++) {
                        block[1] = block[0] - 16;
                        block[2] = block[0] - 17;
                        block[3] = block[0] + 16;
                        grids.at(block[0]).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] - 16).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] - 17).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] + 16).setFillColor(BLOCK_COLOR);
                    }
                }
            }
            else if (rotationLevel == 3) {
                for (auto& block : blocks) {
                    for (int i = 0; i < block.size(); i++) {
                        grids.at(block[i]).setFillColor(NOTHING_COLOR);
                    }
                    for (int i = 0; i < block.size(); i++) {
                        block[1] = block[0] + 1;
                        block[2] = block[0] - 1;
                        block[3] = block[0] - 15;
                        grids.at(block[0]).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] + 1).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] - 1).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] - 15).setFillColor(BLOCK_COLOR);
                    }
                }
                rotationLevel = -1;
            }
        }
        /* I */
        if (blockType == I_type) {
            /* Checking if rotating is possible based on walls */
            std::vector<int> newPoses;
            for (auto& block : blocks) {
                if (rotationLevel == 0) {
                    /* Controlling next rotation - 1 rotation*/
                    newPoses.push_back(block[0] + 1);
                    newPoses.push_back(block[0] - 1);
                    newPoses.push_back(block[0] - 2);
                }
                else if (rotationLevel == 1) {
                    /* Controlling next rotation - 0 rotation*/
                    newPoses.push_back(block[0] - 16);
                    newPoses.push_back(block[0] + 16);
                    newPoses.push_back(block[0] + 32);
                }
            }
            if (canRotate(newPoses, grids)) {
                rotationLevel += 1;
                newPoses.clear();
            }
            else {
                newPoses.clear();
            }

            if (rotationLevel == 0) {
                for (auto& block : blocks) {
                    for (int i = 0; i < block.size(); i++) {
                        grids.at(block[i]).setFillColor(NOTHING_COLOR);
                    }
                    for (int i = 0; i < block.size(); i++) {
                        block[1] = block[0] - 16;
                        block[2] = block[0] + 16;
                        block[3] = block[0] + 32;
                        grids.at(block[0]).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] - 16).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] + 16).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] + 32).setFillColor(BLOCK_COLOR);
                    }
                }
            }
            else if (rotationLevel == 1) {
                for (auto& block : blocks) {
                    for (int i = 0; i < block.size(); i++) {
                        grids.at(block[i]).setFillColor(NOTHING_COLOR);
                    }
                    for (int i = 0; i < block.size(); i++) {
                        block[1] = block[0] + 1;
                        block[2] = block[0] - 1;
                        block[3] = block[0] - 2;
                        grids.at(block[0]).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] + 1).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] - 1).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] - 2).setFillColor(BLOCK_COLOR);
                    }
                }
                rotationLevel = -1;
            }
        }
        /* S */
        if (blockType == S_type) {
            /* Checking if rotating is possible based on walls */
            std::vector<int> newPoses;
            for (auto& block : blocks) {
                if (rotationLevel == 0) {
                    /* Controlling next rotation - 1 rotation*/
                    newPoses.push_back(block[0] - 1);
                    newPoses.push_back(block[0] - 17);
                    newPoses.push_back(block[0] + 16);
                }
                else if (rotationLevel == 1) {
                    /* Controlling next rotation - 0 rotation*/
                    newPoses.push_back(block[0] + 1);
                    newPoses.push_back(block[0] + 16);
                    newPoses.push_back(block[0] + 15);
                }
            }
            if (canRotate(newPoses, grids)) {
                if (rotationLevel == 1) {
                    rotationLevel = -1;
                }
                rotationLevel += 1;
                newPoses.clear();
            }
            else {
                newPoses.clear();
            }

            if (rotationLevel == 0) {
                for (auto& block : blocks) {
                    for (int i = 0; i < block.size(); i++) {
                        grids.at(block[i]).setFillColor(NOTHING_COLOR);
                    }
                    for (int i = 0; i < block.size(); i++) {
                        block[1] = block[0] + 1;
                        block[2] = block[0] + 16;
                        block[3] = block[0] + 15;
                        grids.at(block[0]).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] + 1).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] + 16).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] + 15).setFillColor(BLOCK_COLOR);
                    }
                }
            }
            else if (rotationLevel == 1) {
                for (auto& block : blocks) {
                    for (int i = 0; i < block.size(); i++) {
                        grids.at(block[i]).setFillColor(NOTHING_COLOR);
                    }
                    for (int i = 0; i < block.size(); i++) {
                        block[1] = block[0] - 1;
                        block[2] = block[0] - 17;
                        block[3] = block[0] + 16;
                        grids.at(block[0]).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] - 1).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] - 17).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] + 16).setFillColor(BLOCK_COLOR);
                    }
                }
            }
        }
        /* Z */
        if (blockType == Z_type) {
            /* Checking if rotating is possible based on walls */
            std::vector<int> newPoses;
            for (auto& block : blocks) {
                if (rotationLevel == 0) {
                    /* Controlling next rotation - 1 rotation*/
                    newPoses.push_back(block[0] - 16);
                    newPoses.push_back(block[0] - 1);
                    newPoses.push_back(block[0] + 15);
                }
                else if (rotationLevel == 1) {
                    /* Controlling next rotation - 0 rotation*/
                    newPoses.push_back(block[0] - 1);
                    newPoses.push_back(block[0] + 16);
                    newPoses.push_back(block[0] + 17);
                }
            }
            if (canRotate(newPoses, grids)) {
                if (rotationLevel == 1) {
                    rotationLevel = -1;
                }
                rotationLevel += 1;
                newPoses.clear();
            }
            else {
                newPoses.clear();
            }

            if (rotationLevel == 0) {
                for (auto& block : blocks) {
                    for (int i = 0; i < block.size(); i++) {
                        grids.at(block[i]).setFillColor(NOTHING_COLOR);
                    }
                    for (int i = 0; i < block.size(); i++) {
                        block[1] = block[0] - 1;
                        block[2] = block[0] + 16;
                        block[3] = block[0] + 17;
                        grids.at(block[0]).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] - 1).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] + 16).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] + 17).setFillColor(BLOCK_COLOR);
                    }
                }
            }
            else if (rotationLevel == 1) {
                for (auto& block : blocks) {
                    for (int i = 0; i < block.size(); i++) {
                        grids.at(block[i]).setFillColor(NOTHING_COLOR);
                    }
                    for (int i = 0; i < block.size(); i++) {
                        block[1] = block[0] - 16;
                        block[2] = block[0] - 1;
                        block[3] = block[0] + 15;
                        grids.at(block[0]).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] - 16).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] - 1).setFillColor(BLOCK_COLOR);
                        grids.at(block[0] + 15).setFillColor(BLOCK_COLOR);
                    }
                }
            }
        }
    }
    
    bool isOnBorder(std::vector<sf::RectangleShape>& grids, int dir) {
        for (auto& i : leftWall) 
        {
            if (grids.at(i + 1).getFillColor() == BLOCK_COLOR && dir == LEFT) {
                borderhit.play();
                return true;
            }
        }
        for (auto& i : rightWall)
        {
            if (grids.at(i - 1).getFillColor() == BLOCK_COLOR && dir == RIGHT) {
                borderhit.play();
                return true;
            }
        }

        /* Checking if is there any blocks on left and right */
        for (auto& block : blocks) {
            for (auto& i : block) {
                if (grids.at(i + 1).getFillColor() == STOPPED_BLOCK_COLOR && dir == RIGHT) {
                    return true;
                }
                if (grids.at(i - 1).getFillColor() == STOPPED_BLOCK_COLOR && dir == LEFT) {
                    return true;
                }
            }
        }

        return false;
    }

    bool isLineFull(std::vector<sf::RectangleShape>& grids, int i) {
        int count = 0;
        for (size_t j = 0; j < groundNums.size(); j++) {
            if (grids.at(groundNums[j] - (16 * i)).getFillColor() != STOPPED_BLOCK_COLOR) {
                return false;
            }
            count += 1;
        }
        if (count == LINE_COUNT) {
            return true;
        } 
        else {
            return false;
        }
    } 

    int LineChecker(std::vector<sf::RectangleShape>& grids) {
        /* Check if blocks reaches top if yes stop the game */
        for (auto& stoppedBlock : stoppedBlocks) {
            for (auto& i : stoppedBlock) {
                for (auto& j : topNums)
                {
                    if (i <= j && grids.at(i).getFillColor() == STOPPED_BLOCK_COLOR) {
                        state = GameState::END_SCREEN;
                        gameover.play();
                    }
                }
            }
        }

        /* Checking if blocks reached ground if yes stop it */
        for (auto& num : groundNums)
        {
            if (grids.at(num - 16).getFillColor() == BLOCK_COLOR) {
                if (dir == DOWN) {
                    for (auto& block : blocks) {
                        for (auto& i : block) {
                            grids.at(i).setFillColor(STOPPED_BLOCK_COLOR);
                        }
                        blockdrop.play();
                        stoppedBlocks.push_back(block);
                    }
                    settleClock.restart();
                    settleTime = settleTime.Zero;
                    blocks.clear();
                    rotationLevel = 0;
                    blockScore += 10;
                    return 0;
                }
                settleTime = clock.getElapsedTime();
                if (settleTime.asSeconds() < 1.f) {
                    return 0;
                }
                for (auto& block : blocks) {
                    for (auto& i : block) {
                        grids.at(i).setFillColor(STOPPED_BLOCK_COLOR);
                    }
                    blockdrop.play();
                    stoppedBlocks.push_back(block);
                }
                settleClock.restart();
                settleTime = settleTime.Zero;
                blocks.clear();
                rotationLevel = 0;
                blockScore += 10;
                return 0;
            }
        }

        /* Checking if blocks reached any block if yes stop it */
        for (auto& block : blocks) {
            for (auto& i : block) {
                if (grids.at(i + 16).getFillColor() == STOPPED_BLOCK_COLOR) {
                    if (dir == DOWN) {
                        for (auto& j : block) {
                            grids.at(j).setFillColor(STOPPED_BLOCK_COLOR);
                        }
                        blockdrop.play();
                        stoppedBlocks.push_back(block);
                        settleClock.restart();
                        settleTime = settleTime.Zero;
                        blocks.clear();
                        rotationLevel = 0;
                        blockScore += 10;
                        return 0;
                    }
                    settleTime = clock.getElapsedTime();
                    if (settleTime.asSeconds() < 1.f) {
                        return 0;
                    }
                    for (auto& j : block) {
                        grids.at(j).setFillColor(STOPPED_BLOCK_COLOR);
                    }
                    blockdrop.play();
                    stoppedBlocks.push_back(block);
                    settleClock.restart();
                    settleTime = settleTime.Zero;
                    blocks.clear();
                    rotationLevel = 0;
                    blockScore += 10;
                    return 0;
                }
                else if (grids.at(i + 16).getFillColor() == NOTHING_COLOR) {
                    settleClock.restart();
                    settleTime = settleTime.Zero;
                }
            }
        }
        
        /* Checking all rows if any line is full delete it and make arrangements */
        for (int i = 1; i < 26; i++) {
            if (isLineFull(grids, i)) { 
                for (auto& num : groundNums) {
                    grids.at(num - (16 * i)).setFillColor(NOTHING_COLOR);
                }
                success.setVolume(20);
                success.play();
                lineScore += 100; /* ADD SCORE IF DELETED LINE */
                speed -= 0.05f; /* Speeding the blocks movement */
                std::vector<int> cyans;
                for (int j = 0; j < grids.size(); j++) {
                    if (groundNums[0] - (16 * i) > j) {
                        if (grids.at(j).getFillColor() == STOPPED_BLOCK_COLOR) {
                            grids.at(j).setFillColor(NOTHING_COLOR);
                            cyans.push_back(j);
                        }
                    }
                }
                for (auto& cyan : cyans) {
                    grids.at(cyan + 16).setFillColor(STOPPED_BLOCK_COLOR);
                }
            }
        }
    } 
    
    int spawner(std::vector<sf::RectangleShape>& grids) {
        srand((unsigned)std::time(0));
        for (auto& block : blocks) {
            if (!block.empty())
                return 0;
        }
        int randNum = rand() % 7;
        switch (randNum)
        {
        case 0:
            initI(grids);
            blockType = I_type;
            break;
        case 1:
            initO(grids);
            blockType = O_type;
            break;
        case 2:
            initS(grids);
            blockType = S_type;
            break;
        case 3:
            initZ(grids);
            blockType = Z_type;
            break;
        case 4:
            initJ(grids);
            blockType = J_type;
            break;
        case 5:
            initL(grids);
            blockType = L_type;
            break;
        case 6:
            initT(grids);
            blockType = T_type;
            break;
        default:
            break;
        };
        return 0;
    }

    int movement(std::vector<sf::RectangleShape>& grids) { 
        if (!isOnBorder(grids, dir)) {
            for (auto& block : blocks) {
                for (auto& i : block) {
                    if (block.empty()) {
                        return 0;
                    }
                    if (settleTime.asSeconds() != 0.f && (dir == UP || dir == DOWN)) {
                        return 0;
                    }
                    grids.at(i).setFillColor(NOTHING_COLOR);
                    i += dir;
                    grids.at(i).setFillColor(BLOCK_COLOR);
                }
            }
        }
        dir = 0;
        if (settleTime.asSeconds() == 0.f) {
            if (time.asSeconds() > speed) {
                for (auto& block : blocks) {
                    for (int i = 0; i < block.size(); i++) {
                        grids.at(block[i]).setFillColor(NOTHING_COLOR);
                        block[i] += DOWN;
                    }
                }
                for (auto& block : blocks) {
                    for (auto& i : block) {
                        grids.at(i).setFillColor(BLOCK_COLOR);
                    }
                }
                clock.restart();
            }
        }
        
        return 0;
    }

    int update(std::vector<sf::RectangleShape>& grids) {
        time = clock.getElapsedTime();
        LineChecker(grids);
        spawner(grids);
        movement(grids);
        
        return 0;
    }
    
private:
    int dir;
    std::vector<int> O;
    std::vector<int> I;
    std::vector<int> S;
    std::vector<int> Z;
    std::vector<int> L;
    std::vector<int> J;
    std::vector<int> T;


    std::vector<std::vector<int>> blocks;

    std::vector<std::vector<int>> stoppedBlocks;
 
    sf::Time time;
    sf::Time settleTime;


    sf::Clock clock;
    sf::Clock settleClock;


    void initT(std::vector<sf::RectangleShape>& grids) {
        T.clear();
        if (grids.at(7).getFillColor() == STOPPED_BLOCK_COLOR ||
            grids.at(23).getFillColor() == STOPPED_BLOCK_COLOR ||
            grids.at(24).getFillColor() == STOPPED_BLOCK_COLOR ||
            grids.at(22).getFillColor() == STOPPED_BLOCK_COLOR) {
            state = GameState::END_SCREEN;
            gameover.play();
        }
        grids.at(7).setFillColor(BLOCK_COLOR);
        grids.at(23).setFillColor(BLOCK_COLOR);
        grids.at(24).setFillColor(BLOCK_COLOR);
        grids.at(22).setFillColor(BLOCK_COLOR);
        T.push_back(23);
        T.push_back(7);
        T.push_back(24);
        T.push_back(22);
        blocks.push_back(T);
    }
    void initJ(std::vector<sf::RectangleShape>& grids) {
        J.clear();
        if (grids.at(7).getFillColor() == STOPPED_BLOCK_COLOR ||
            grids.at(23).getFillColor() == STOPPED_BLOCK_COLOR ||
            grids.at(39).getFillColor() == STOPPED_BLOCK_COLOR ||
            grids.at(38).getFillColor() == STOPPED_BLOCK_COLOR) {
            state = GameState::END_SCREEN;
            gameover.play();
        }
        grids.at(7).setFillColor(BLOCK_COLOR);
        grids.at(23).setFillColor(BLOCK_COLOR);
        grids.at(39).setFillColor(BLOCK_COLOR);
        grids.at(38).setFillColor(BLOCK_COLOR);
        J.push_back(39);
        J.push_back(7);
        J.push_back(23);
        J.push_back(38);
        blocks.push_back(J);
    }
    void initL(std::vector<sf::RectangleShape>& grids) {
        L.clear();
        if (grids.at(6).getFillColor() == STOPPED_BLOCK_COLOR ||
            grids.at(22).getFillColor() == STOPPED_BLOCK_COLOR ||
            grids.at(38).getFillColor() == STOPPED_BLOCK_COLOR ||
            grids.at(39).getFillColor() == STOPPED_BLOCK_COLOR) {
            state = GameState::END_SCREEN;
            gameover.play();
        }
        grids.at(6).setFillColor(BLOCK_COLOR);
        grids.at(22).setFillColor(BLOCK_COLOR);
        grids.at(38).setFillColor(BLOCK_COLOR);
        grids.at(39).setFillColor(BLOCK_COLOR);
        L.push_back(6);
        L.push_back(22);
        L.push_back(38);
        L.push_back(39);
        blocks.push_back(L);
    }
    void initZ(std::vector<sf::RectangleShape>& grids) {
        Z.clear();
        if (grids.at(6).getFillColor() == STOPPED_BLOCK_COLOR ||
            grids.at(7).getFillColor() == STOPPED_BLOCK_COLOR ||
            grids.at(23).getFillColor() == STOPPED_BLOCK_COLOR ||
            grids.at(24).getFillColor() == STOPPED_BLOCK_COLOR) {
            state = GameState::END_SCREEN;
            gameover.play();
        }
        grids.at(6).setFillColor(BLOCK_COLOR);
        grids.at(7).setFillColor(BLOCK_COLOR);
        grids.at(23).setFillColor(BLOCK_COLOR);
        grids.at(24).setFillColor(BLOCK_COLOR);
        Z.push_back(6);
        Z.push_back(7);
        Z.push_back(23);
        Z.push_back(24);
        blocks.push_back(Z);
    }
    void initS(std::vector<sf::RectangleShape>& grids) {
        S.clear();
        if (grids.at(7).getFillColor() == STOPPED_BLOCK_COLOR ||
            grids.at(8).getFillColor() == STOPPED_BLOCK_COLOR ||
            grids.at(22).getFillColor() == STOPPED_BLOCK_COLOR ||
            grids.at(23).getFillColor() == STOPPED_BLOCK_COLOR) {
            state = GameState::END_SCREEN;
            gameover.play();
        }
        grids.at(7).setFillColor(BLOCK_COLOR);
        grids.at(8).setFillColor(BLOCK_COLOR);
        grids.at(22).setFillColor(BLOCK_COLOR);
        grids.at(23).setFillColor(BLOCK_COLOR);
        S.push_back(7);
        S.push_back(8);
        S.push_back(22);
        S.push_back(23);
        blocks.push_back(S);
    }
    void initI(std::vector<sf::RectangleShape>& grids) {
        I.clear();
        if (grids.at(7).getFillColor() == STOPPED_BLOCK_COLOR ||
            grids.at(23).getFillColor() == STOPPED_BLOCK_COLOR ||
            grids.at(39).getFillColor() == STOPPED_BLOCK_COLOR ||
            grids.at(55).getFillColor() == STOPPED_BLOCK_COLOR) {
            state = GameState::END_SCREEN;
            gameover.play();
        }
        grids.at(7).setFillColor(BLOCK_COLOR);
        grids.at(23).setFillColor(BLOCK_COLOR);
        grids.at(39).setFillColor(BLOCK_COLOR);
        grids.at(55).setFillColor(BLOCK_COLOR);
        I.push_back(7);
        I.push_back(23);
        I.push_back(39);
        I.push_back(55);
        blocks.push_back(I);
    }
    void initO(std::vector<sf::RectangleShape>& grids) {
        O.clear();
        if (grids.at(6).getFillColor() == STOPPED_BLOCK_COLOR ||
            grids.at(7).getFillColor() == STOPPED_BLOCK_COLOR ||
            grids.at(22).getFillColor() == STOPPED_BLOCK_COLOR ||
            grids.at(23).getFillColor() == STOPPED_BLOCK_COLOR) {
            state = GameState::END_SCREEN;
            gameover.play();
        }
        grids.at(6).setFillColor(BLOCK_COLOR);
        grids.at(7).setFillColor(BLOCK_COLOR);
        grids.at(22).setFillColor(BLOCK_COLOR);
        grids.at(23).setFillColor(BLOCK_COLOR);
        O.push_back(6);
        O.push_back(7);
        O.push_back(22);
        O.push_back(23);
        blocks.push_back(O);
    }
    void LoadGroundNums(std::vector<sf::RectangleShape>& grids) {
        int j = 0;
        for (size_t i = grids.size() - 15; i < grids.size() - 1; i++)
        {
            groundNums[j] = i;
            grids.at(i).setFillColor(GROUND_COLOR);
            j++;
        }
    }
    void LoadTopNums(std::vector<sf::RectangleShape>& grids) {
        int j = 0;
        for (size_t i = 1; i < 15; i++)
        {
            topNums[j] = i;
            grids.at(i).setFillColor(NOTHING_COLOR);
            j++;
            
        }
    }
};

class GUI
{
public:
    GUI() {
        initText();
        initButtons();
        InitEndScreen();
    }

    void UpdateScore(int lScore, int bScore) {
        if (state == GameState::GAME) {
            lineScore.setString("Line Score: " + std::to_string(lScore));
            blockScore.setString("Block Score: " + std::to_string(bScore));
        }
        if (state == GameState::END_SCREEN) {
            if (bScore > scoreKeper.BLOCK_SCORE) {
                scoreKeper.BLOCK_SCORE = bScore;
            }
            if (lScore > scoreKeper.LINE_SCORE) {
                scoreKeper.LINE_SCORE = lScore;
            }
            int sum = scoreKeper.BLOCK_SCORE + scoreKeper.LINE_SCORE;
            HighestScore.setString("Highscore: " + std::to_string(sum));
        }
    }

    int ButtonInteraction(sf::RenderWindow* window, Blocks& blocks, std::vector<sf::RectangleShape>& grids) {
        if (state == GameState::MAIN_MENU) {
            /* start button interaction */
            {
                if (sf::Mouse::isButtonPressed(sf::Mouse::Left) &&
                    !isPressed &&
                    StartButton.getGlobalBounds().contains(sf::Mouse::getPosition(*window).x, sf::Mouse::getPosition(*window).y)
                    )
                {
                    buttonPressSound.play();
                    isPressed = true;
                    StartButton.setFillColor(STOPPED_BLOCK_COLOR);
                    return 0;
                }
                else if (isPressed &&
                    !sf::Mouse::isButtonPressed(sf::Mouse::Left) &&
                    StartButton.getGlobalBounds().contains(sf::Mouse::getPosition(*window).x, sf::Mouse::getPosition(*window).y))
                {
                    state = GameState::GAME;
                    buttonReleaseSound.play();
                    isPressed = false;
                    StartButton.setFillColor(sf::Color(43, 72, 101));
                    return 0;
                }
                else if (!sf::Mouse::isButtonPressed(sf::Mouse::Left) &&
                    !StartButton.getGlobalBounds().contains(sf::Mouse::getPosition(*window).x, sf::Mouse::getPosition(*window).y)) {
                    isPressed = false;
                    StartButton.setFillColor(sf::Color(43, 72, 101));
                }
            }

            /* credits button interaction */
            {
                if (!isCreditPressed &&
                    sf::Mouse::isButtonPressed(sf::Mouse::Left) &&
                    CreditsButton.getGlobalBounds().contains(sf::Mouse::getPosition(*window).x, sf::Mouse::getPosition(*window).y))
                {
                    buttonPressSound.play();
                    isCreditPressed = true;
                    CreditsButton.setFillColor(STOPPED_BLOCK_COLOR);
                    return 0;
                }
                else if (isCreditPressed && !sf::Mouse::isButtonPressed(sf::Mouse::Left))
                {
                    isCreditPressed = false;
                    CreditsButton.setFillColor(sf::Color(43, 72, 101));
                    return 0;
                }
            }
        }

        if (state == GameState::END_SCREEN) {
            {
                if (!isRestartPressed &&
                    sf::Mouse::isButtonPressed(sf::Mouse::Left) &&
                    RestartButton.getGlobalBounds().contains(sf::Mouse::getPosition(*window).x, sf::Mouse::getPosition(*window).y))
                {
                    buttonPressSound.play();
                    isRestartPressed = true;
                    RestartButton.setFillColor(sf::Color(135, 65, 65));
                }
                else if (isRestartPressed &&
                    !sf::Mouse::isButtonPressed(sf::Mouse::Left) &&
                    RestartButton.getGlobalBounds().contains(sf::Mouse::getPosition(*window).x, sf::Mouse::getPosition(*window).y))
                {
                    buttonReleaseSound.play();
                    isRestartPressed = false;
                    blocks.Restart(grids);
                    state = GameState::GAME;
                    RestartButton.setFillColor(sf::Color(161, 157, 157, 220));
                }
                else if (isRestartPressed &&
                    !sf::Mouse::isButtonPressed(sf::Mouse::Left) &&
                    !RestartButton.getGlobalBounds().contains(sf::Mouse::getPosition(*window).x, sf::Mouse::getPosition(*window).y))
                {
                    isRestartPressed = false;
                    RestartButton.setFillColor(sf::Color(161, 157, 157, 220));
                }
            }
            {
                if (!isMenuPressed &&
                    sf::Mouse::isButtonPressed(sf::Mouse::Left) &&
                    MainMenu.getGlobalBounds().contains(sf::Mouse::getPosition(*window).x, sf::Mouse::getPosition(*window).y))
                {
                    buttonPressSound.play();
                    isMenuPressed = true;
                    MainMenu.setFillColor(sf::Color(135, 65, 65));
                }
                else if (isMenuPressed &&
                    !sf::Mouse::isButtonPressed(sf::Mouse::Left) &&
                    MainMenu.getGlobalBounds().contains(sf::Mouse::getPosition(*window).x, sf::Mouse::getPosition(*window).y))
                {
                    buttonReleaseSound.play();
                    isMenuPressed = false;
                    state = GameState::MAIN_MENU;
                    blocks.Restart(grids);
                    MainMenu.setFillColor(sf::Color(161, 157, 157, 220));
                }
                else if (isMenuPressed &&
                    !sf::Mouse::isButtonPressed(sf::Mouse::Left) &&
                    !MainMenu.getGlobalBounds().contains(sf::Mouse::getPosition(*window).x, sf::Mouse::getPosition(*window).y))
                {
                    isMenuPressed = false;
                    MainMenu.setFillColor(sf::Color(161, 157, 157, 220));
                }
            }
        }
    }

    void EndScreenRender(sf::RenderWindow* window) {
        window->draw(EmptyBlock);
        window->draw(RestartButton);
        window->draw(MainMenu);
        window->draw(RestartText);
        window->draw(MainMenuText);
        window->draw(HighestScore);
    }

    void MainMenuRender(sf::RenderWindow* window) {
        window->draw(Title);
        window->draw(StartButton);
        window->draw(CreditsButton);
        window->draw(StartText);
        window->draw(CreditsText);
    }

    void Update(sf::RenderWindow* window, Blocks& blocks, std::vector<sf::RectangleShape>& grids) {
        ButtonInteraction(window, blocks, grids);
    }

    int Render(sf::RenderWindow* window) {

        if (state == GameState::GAME) {
            window->draw(lineScore);
            window->draw(blockScore);
            return 0;
        }
        if (state == GameState::MAIN_MENU) {
            MainMenuRender(window);
        }
        if (state == GameState::END_SCREEN) {
            EndScreenRender(window);
        }

        return 0;
    }

private:
    bool isRestartPressed = false;
    bool isMenuPressed = false;

private:
    sf::RectangleShape EmptyBlock;
    sf::RectangleShape RestartButton;
    sf::RectangleShape MainMenu;
    sf::Text MainMenuText;
    sf::Text RestartText;
    sf::Text HighestScore;

    void InitEndScreen() {
        /* Empty Block */
        EmptyBlock.setFillColor(sf::Color(46, 45, 45, 100));
        EmptyBlock.setOutlineThickness(3.f);
        EmptyBlock.setOutlineColor(sf::Color(87, 31, 31, 160));
        EmptyBlock.setSize(sf::Vector2f(300.f, 500.f));
        EmptyBlock.setOrigin(150.f, 250.f);
        EmptyBlock.setPosition(400.f, 350.f);
        /* Restart Button and Restart Text */
        RestartButton.setFillColor(sf::Color(161, 157, 157, 220));
        RestartButton.setOutlineThickness(3.f);
        RestartButton.setOutlineColor(sf::Color(87, 31, 31, 245));
        RestartButton.setSize(sf::Vector2f(200.f, 50.f));
        RestartButton.setOrigin(100.f, 25.f);
        RestartButton.setPosition(400.f, 300.f);
        // text
        RestartText.setFont(arial);
        RestartText.setString("Restart");
        RestartText.setCharacterSize(24);
        RestartText.setFillColor(sf::Color(204, 204, 204));
        RestartText.setStyle(sf::Text::Bold);
        RestartText.setOrigin(sf::Vector2f(42.f, 12.f));
        RestartText.setPosition(sf::Vector2f(400, 300));
        /* MainMenu Button and MainMenu Text */
        MainMenu.setFillColor(sf::Color(161, 157, 157, 220));
        MainMenu.setOutlineThickness(3.f);
        MainMenu.setOutlineColor(sf::Color(87, 31, 31, 245));
        MainMenu.setSize(sf::Vector2f(200.f, 50.f));
        MainMenu.setOrigin(100.f, 25.f);
        MainMenu.setPosition(400.f, 400.f);
        // text
        MainMenuText.setFont(arial);
        MainMenuText.setString("Menu");
        MainMenuText.setCharacterSize(24);
        MainMenuText.setFillColor(sf::Color(204, 204, 204));
        MainMenuText.setStyle(sf::Text::Bold);
        MainMenuText.setOrigin(sf::Vector2f(32.f, 12.f));
        MainMenuText.setPosition(sf::Vector2f(400, 400));
        /* Highscore text */
        HighestScore.setFont(arial);
        HighestScore.setString("Highscore");
        HighestScore.setCharacterSize(32);
        HighestScore.setFillColor(sf::Color(204, 204, 204));
        HighestScore.setStyle(sf::Text::Bold);
        HighestScore.setOrigin(sf::Vector2f(32.f, 12.f));
        HighestScore.setPosition(sf::Vector2f(330, 200));
    }

private:
    /* Score Thingies */
    sf::Text lineScore;
    sf::Text blockScore;
    sf::Text Title;
    sf::Font arial;

    /* Buttons */
    sf::RectangleShape StartButton;
    sf::Text StartText;
    sf::RectangleShape CreditsButton;
    sf::Text CreditsText;

    /* Mouse thingie */
    bool isPressed = false;
    bool isCreditPressed = false;

    void initButtons() {
        /* Start button and button text config */
        StartButton.setFillColor(sf::Color(43, 72, 101));
        StartButton.setOutlineThickness(2.f);
        StartButton.setOutlineColor(STOPPED_BLOCK_COLOR);
        StartButton.setSize(sf::Vector2f(300.f, 70.f));
        StartButton.setOrigin(150.f, 35.f);
        StartButton.setPosition(StartText.getPosition());
        /* Credits button and button text config */
        CreditsButton.setFillColor(sf::Color(43, 72, 101));
        CreditsButton.setOutlineThickness(2.f);
        CreditsButton.setOutlineColor(STOPPED_BLOCK_COLOR);
        CreditsButton.setSize(sf::Vector2f(300.f, 70.f));
        CreditsButton.setOrigin(150.f, 35.f);
        CreditsButton.setPosition(CreditsText.getPosition());
    }

    void initText() {
        if (!arial.loadFromFile("assets/arial.ttf"))
        {
            std::cout << "ERROR::FONT_LOAD" << std::endl;
        }
        /* lineScore */
        lineScore.setFont(arial);
        lineScore.setString("Hello score");
        lineScore.setCharacterSize(36);

        auto center = sf::Vector2f(lineScore.getGlobalBounds().width / 2.f, lineScore.getGlobalBounds().height / 2.f);
        auto localBounds = center + sf::Vector2f(lineScore.getLocalBounds().left, lineScore.getLocalBounds().top);
        auto rounded = sf::Vector2f(round(localBounds.x), round(localBounds.y));

        lineScore.setOrigin(rounded);
        lineScore.setFillColor(BLOCK_COLOR);
        lineScore.setStyle(sf::Text::Bold);
        lineScore.setPosition(sf::Vector2f(WIDTH / 1.42, HEIGHT / 2 - 50));
        /* blockScore */
        blockScore.setFont(arial);
        blockScore.setString("Hello block");
        blockScore.setCharacterSize(36);

        center = sf::Vector2f(blockScore.getGlobalBounds().width / 2.f, blockScore.getGlobalBounds().height / 2.f);
        localBounds = center + sf::Vector2f(blockScore.getLocalBounds().left, blockScore.getLocalBounds().top);
        rounded = sf::Vector2f(round(localBounds.x), round(localBounds.y));

        blockScore.setOrigin(rounded);
        blockScore.setFillColor(BLOCK_COLOR);
        blockScore.setStyle(sf::Text::Bold);
        blockScore.setPosition(sf::Vector2f(WIDTH / 1.42, HEIGHT / 2 + 50));
        /* Title */
        Title.setFont(arial);
        Title.setString("TETRIS");
        Title.setCharacterSize(116);

        center = sf::Vector2f(Title.getGlobalBounds().width / 2.f, Title.getGlobalBounds().height / 2.f);
        localBounds = center + sf::Vector2f(Title.getLocalBounds().left, Title.getLocalBounds().top);
        rounded = sf::Vector2f(round(localBounds.x), round(localBounds.y));

        Title.setOrigin(rounded);
        Title.setFillColor(BLOCK_COLOR);
        Title.setStyle(sf::Text::Bold);
        Title.setPosition(sf::Vector2f(WIDTH / 2, 120));
        /* StartButton text */
        StartText.setFont(arial);
        StartText.setString("start");
        StartText.setCharacterSize(36);

        center = sf::Vector2f(StartText.getGlobalBounds().width / 2.f, StartText.getGlobalBounds().height / 2.f);
        localBounds = center + sf::Vector2f(StartText.getLocalBounds().left, StartText.getLocalBounds().top);
        rounded = sf::Vector2f(round(localBounds.x), round(localBounds.y));

        StartText.setOrigin(rounded);
        StartText.setFillColor(sf::Color::White);
        StartText.setStyle(sf::Text::Bold);
        StartText.setPosition(sf::Vector2f(WIDTH / 2, 290.f));
        /* CreditsButton text */
        CreditsText.setFont(arial);
        CreditsText.setString("credits");
        CreditsText.setCharacterSize(36);

        center = sf::Vector2f(CreditsText.getGlobalBounds().width / 2.f, CreditsText.getGlobalBounds().height / 2.f);
        localBounds = center + sf::Vector2f(CreditsText.getLocalBounds().left, CreditsText.getLocalBounds().top);
        rounded = sf::Vector2f(round(localBounds.x), round(localBounds.y));

        CreditsText.setOrigin(rounded);
        CreditsText.setFillColor(sf::Color::White);
        CreditsText.setStyle(sf::Text::Bold);
        CreditsText.setPosition(sf::Vector2f(WIDTH / 2, 400.f));
    }
};

int main()
{
    sf::RenderWindow* window = new sf::RenderWindow(sf::VideoMode(WIDTH, HEIGHT), "Tetris!");
    window->setFramerateLimit(120);
    
    /* Setting up ground, wall, and grid */
    std::vector<sf::RectangleShape> grounds;
    std::vector<sf::RectangleShape> grids;
    std::vector<int> walls;
    
    SetGrounds(grounds);
    SetGrids(grids);
    SetWalls(grids, walls);
    
    /* Block thingies */
    Blocks blocks(grids);
    blocks.setUpWalls(walls);

    /* GUI thingies */
    GUI gui;


    /* music and soundthingie */
    sf::SoundBuffer buttonReleaseBuffer;
    if (!buttonReleaseBuffer.loadFromFile("assets/buttonrelease.wav"))
        return -1;

    buttonReleaseSound.setBuffer(buttonReleaseBuffer);
    buttonReleaseSound.setVolume(20);

    sf::SoundBuffer buttonPressBuffer;
    if (!buttonPressBuffer.loadFromFile("assets/buttonpress.wav"))
        return -1;

    buttonPressSound.setBuffer(buttonPressBuffer);
    buttonPressSound.setPitch(1.f);
    buttonPressSound.setVolume(3);

    sf::SoundBuffer gameOverBuffer;
    if (!gameOverBuffer.loadFromFile("assets/gameover.wav"))
        return -1;

    gameover.setBuffer(gameOverBuffer);


    sf::SoundBuffer successBuffer;
    if (!successBuffer.loadFromFile("assets/success.wav"))
        return -1;

    success.setBuffer(successBuffer);

    sf::SoundBuffer dropBuffer;
    if (!dropBuffer.loadFromFile("assets/blockdrop.wav"))
        return -1;

    blockdrop.setBuffer(dropBuffer);
    blockdrop.setPitch(0.5f);
    blockdrop.setVolume(15);

    sf::SoundBuffer borderHitBuffer;
    if (!borderHitBuffer.loadFromFile("assets/borderhit.wav"))
        return -1;

    borderhit.setBuffer(borderHitBuffer);
    borderhit.setPitch(0.5f);

    sf::SoundBuffer rotateBuffer;
    if (!rotateBuffer.loadFromFile("assets/rotate.wav"))
        return -1;

    
    rotateSound.setBuffer(rotateBuffer);
    rotateSound.setPitch(0.5f);

    if (!theme.openFromFile("assets/theme.ogg"))
        return -1;
    theme.setLoop(true);
    theme.setVolume(1.3f);
    theme.play();

    while (window->isOpen())
    {
        sf::Event event;
        while (window->pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window->close();
            if (event.type == sf::Event::KeyPressed) {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
                    blocks.input(1);
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
                    blocks.input(-1);
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
                    blocks.input(-16);
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
                    blocks.input(16);
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
                    blocks.rotate(grids);
                }
            }
        }

        window->clear();
        
        gui.UpdateScore(blocks.lineScore, blocks.blockScore);
        gui.Update(window, blocks, grids);
        gui.Render(window);
        if (state == GameState::GAME) {
            blocks.update(grids);

            for (auto& ground : grounds)
            {
                window->draw(ground);
            }
            for (auto& grid : grids)
            {
                window->draw(grid);
            }
        }
        
        window->display();
    }

    return 0;
}

void SetWalls(std::vector<sf::RectangleShape>& grids, std::vector<int>& walls) {
    for (size_t i = 0; i < 28; i++)
    {
        walls.push_back(0 + (i * 16));
        grids.at(0 + (i * 16)).setFillColor(BORDER_COLOR);
    }
    for (size_t i = 0; i < 28; i++)
    {
        walls.push_back(15 + (i * 16));
        grids.at(15 + (i * 16)).setFillColor(BORDER_COLOR);
    }
}

void SetGrounds(std::vector<sf::RectangleShape>& grounds) {
    for (size_t i = 0; i < 15; i++)
    {
        sf::RectangleShape ground;
        ground.setFillColor(NOTHING_COLOR); // just testing 
        ground.setSize(sf::Vector2f(25.f, 25.f));
        ground.setOrigin(sf::Vector2f(12.5f, 12.5f));
        ground.setPosition(37.5f + (25 * (i - 1)), (700 - 12.5f));
        grounds.emplace_back(ground);
    }
}

void SetGrids(std::vector<sf::RectangleShape>& grids) {
    for (size_t y = 0; y < 28; y++)
    {
        for (size_t x = 0; x < 16; x++)
        {
            sf::RectangleShape grid;
            grid.setFillColor(NOTHING_COLOR);
            grid.setSize(sf::Vector2f(23.f, 23.f));
            grid.setOutlineThickness(2.f);
            grid.setOutlineColor(sf::Color(43, 52, 103));
            grid.setOrigin(sf::Vector2f(12.5f, 12.5f));
            grid.setPosition(12.5f + (25 * x), (12.f + (25.f  * y)));
            grids.emplace_back(grid);
        }
    }
}