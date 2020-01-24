#include <iostream>
using namespace std;

int bomb = 10;

class Cell {
public:
	Cell(): number(0), isRevealed(false), isMarked(false), isBomb(false) {}
	// Cell(bool isRevealed, bool isMarked): number(0), isRevealed(isRevealed), isMarked(isMarked){}
	int getNumber() const { return number;}
	bool getIsRevealed() const { return isRevealed;}
	bool getIsMarked() const { return isMarked;}
	bool getIsBomb() const {return isBomb;}
	void setNumber(const int& number) {this->number = number;}
	void setisRevealed(const bool& isRevealed) {this->isRevealed = isRevealed;}
	void setIsMarked(const bool& isMarked) {this->isMarked = isMarked;}

private:
	int number; // number can be -1,0,1,2,3,4,5,6,7,8. -1 means the cell has a bomb.
	bool isRevealed; // Is the cell selected by the user
	bool isMarked; // Is the cell marked by the user
	bool isBomb; // Is the cell a bomb
};

Cell grid[8][8]; //Initialize 8x8 grid

//tested: worked for 8x8 grid and bomb <= 64
void generateBombs(){
	int mineCount = 0;
	int rrow, rcol;
	do{
		do{
			rrow = rand()%8;
			rcol = rand()%8;
		} while(grid[rrow][rcol].getNumber() == -1);
		grid[rrow][rcol].setNumber(-1);
		mineCount++;
	} while(mineCount < bomb);
}

//Need more data to test
void draw(){
	for (int i = 0; i < 8; i++){
		for(int j = 0; j < 8; j++){
			if(grid[i][j].getIsMarked()){
				std::cout << "M ";
			} else if (!grid[i][j].getIsRevealed()){
				std::cout << "* ";
			} else if (grid[i][j].getNumber() == -1){
				std::cout << "X ";
			} else {
				std::cout << grid[i][j].getNumber() << " ";
			}
		}
		std::cout << "\n";
	}
	std::cout << "Number of bombs left: " << bomb << "\n";
}


class Minesweeper {
public:
	int rowNum;
	int colNum;
	int numPlayers;

	Minesweeper(const int& rowNum, const int& colNum, const int& numPlayers) {
		this->rowNum = rowNum;
		this->colNum = colNum;
		this->numPlayers = numPlayers;

		this->gameGrid = new Cell*[rowNum];
		for (int i = 0; i < rowNum; i++) {
			gameGrid[i] = new Cell[colNum];
		}
	}

	void setHasWon(const bool& hasWon) {this->hasWon = hasWon;}
	bool getHasWon() const {return hasWon;}
	Cell** getGameGrid() const {return gameGrid;}

private:
	bool hasWon = false;
	Cell** gameGrid;

};

int main() {

	generateBombs();
	draw();

	// test constructor for MineSweeper
	// will introduce 2D arrays into 

	int rowNum = 8;
	int colNum = 8;
	int numPlayers = 2;

	Minesweeper* game = new Minesweeper(rowNum, colNum, numPlayers);

	cout << "RowNum: " << game->rowNum << endl;
	cout << "colNum: " << game->colNum << endl;
	cout << "numPlayers: " << game->numPlayers << endl;
	cout << "hasWon: " << game->getHasWon() << endl;
	cout << "gameGrid[0][0]: " << game->getGameGrid()[0][0].getNumber() << endl;

	return 0;
}