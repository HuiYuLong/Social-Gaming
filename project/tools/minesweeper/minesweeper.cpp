#include <iostream>
using namespace std;

int bomb = 10;

class Cell {
public:
	Cell(): number(0), isRevealed(false), isMarked(false) {}
	// Cell(bool isRevealed, bool isMarked): number(0), isRevealed(isRevealed), isMarked(isMarked){}
	int getNumber() const { return number;}
	bool getisRevealed() const { return isRevealed;}
	bool getIsMarked() const { return isMarked;}
	void setNumber(const int& number) {this->number = number;}
	void setisRevealed(const bool& isRevealed) {this->isRevealed = isRevealed;}
	void setIsMarked(const bool& isMarked) {this->isMarked = isMarked;}

private:
	int number; // number can be -1,0,1,2,3,4,5,6,7,8. -1 means the cell has a bomb.
	bool isRevealed; // Is the cell selected by the user
	bool isMarked; // Is the cell marked by the user
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
			} else if (!grid[i][j].getisRevealed()){
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

int main() {
	generateBombs();
	draw();
	return 0;
}