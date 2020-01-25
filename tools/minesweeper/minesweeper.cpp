#include <iostream>
using namespace std;
#define NEW 0
#define EXPERT 1
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
	bool isBomb;//Is the cell bomb
	bool isValid;//Is the cell on the board.
};


//Cell grid[8][8]; //Initialize 8x8 grid

// A Function to choose the difficulty level 
void gameLevel () 
{ 
	 int difficulty; 
     cout << "Enter the Difficulty Level\n"; 
     cout << "Press 0 for BEGINNER (8 * 8 Cells and 10 Bombs)\n"; 
     cout << "Press 1 for EXPERT (8 * 8 Cells and 30 Bombs)\n"; 
     cin >> difficulty; 
  
     if (difficulty == NEW) 
     { 
         bomb = 10; 
     } 
  
     if (difficulty == EXPERT) 
     { 
         bomb = 30; 
     } 
     return; 
} 


//tested: worked for 8x8 grid and bomb <= 64
// integrated into Minesweeper class
// void generateBombs(){
// 	int mineCount = 0;
// 	int rrow, rcol;
// 	do{
// 		do{
// 			rrow = rand()%8;
// 			rcol = rand()%8;
// 			cout<<"rrow"<<rrow<<" rcol"<<rcol<<endl;
// 		} while(grid[rrow][rcol].getNumber() == -1);
// 		grid[rrow][rcol].setNumber(-1);
// 		mineCount++;
// 	} while(mineCount < bomb);
// }

//Need more data to test
// integrated into Minesweeper class
// void draw(){
// 	for (int i = 0; i < 8; i++){
// 		for(int j = 0; j < 8; j++){
// 			if(grid[i][j].getIsMarked()){
// 				std::cout << "M ";
// 			} else if (!grid[i][j].getIsRevealed()){
// 				std::cout << "* ";
// 			} else if (grid[i][j].getNumber() == -1){
// 				std::cout << "X ";
// 			} else {
// 				std::cout << grid[i][j].getNumber() << " ";
// 			}
// 		}
// 		std::cout << "\n";
// 	}
// 	std::cout << "Number of bombs left: " << bomb << "\n";
// }

class Minesweeper {
public:
	int rowNum;
	int colNum;
	int numPlayers;
	int mineCount;

	Minesweeper(const int& rowNum, const int& colNum, const int& mineCount, const int& numPlayers) {
		this->rowNum = rowNum;
		this->colNum = colNum;
		this->numPlayers = numPlayers;
		this->mineCount = mineCount;
		this->gameGrid = new Cell*[rowNum];
		for (int i = 0; i < rowNum; i++) {
			gameGrid[i] = new Cell[colNum];
		}
	}

	void setHasWon(const bool& hasWon) {this->hasWon = hasWon;}
	bool getHasWon() const {return hasWon;}
	Cell** getGameGrid() const {return gameGrid;}
	int getMineCount() const {return mineCount;}

	bool isBomb(int row, int col) {
		if (gameGrid[row][col].getIsBomb() == true) {
			return true;
		} else {
			return false;
		}
	}

	bool isValidMove(int row, int col) {
		return (row >= 0) && (row < rowNum) && (col >= 0) && (col < colNum);
	}


	// not sure about why the get number is being set to -1 // -1 is bomb ...
	void generateBombs(){
		int mineCount = 0;
		int rrow, rcol;
		do{
			do{
				rrow = rand()%rowNum;
				rcol = rand()%colNum;
				//cout<<"rrow"<<rrow<<" rcol"<<rcol<<endl; // 
			} while(gameGrid[rrow][rcol].getNumber() == -1);
			gameGrid[rrow][rcol].setNumber(-1);
			mineCount++;
		} while(mineCount < this->mineCount);
	}

	void draw(){
		for (int i = 0; i < this->rowNum; i++){
			for(int j = 0; j < this->colNum; j++){
				if(gameGrid[i][j].getIsMarked()){
					std::cout << "M ";
				} else if (!gameGrid[i][j].getIsRevealed()){
					std::cout << "* ";
				} else if (gameGrid[i][j].getNumber() == -1){
					std::cout << "X ";
				} else {
					std::cout << gameGrid[i][j].getNumber() << " ";
				}
			}

			std::cout << "\n";
		}
		std::cout << "Number of bombs left: " << mineCount << "\n";
	}

	void checkLose(int row, int col) {
		if (gameGrid[row][col].getNumber() == -1) {
			for (int i = 0; i < this->rowNum; i++) {
				for (int j = 0; j < this->colNum; j++) {
					gameGrid[i][j].setisRevealed(true);
				}
			}
			draw();
			std::cout << "You lose" << "\n";
			this->hasWon = true;
		}
	}

	void checkWin() {
		int remainCells = 0;
		for (int i = 0; i < this->rowNum; i++) {
			for (int j = 0; j < this->colNum; j++) {
				if (gameGrid[i][j].getNumber() != -1 && !gameGrid[i][j].getIsRevealed()) {
					remainCells++;
				}
			}
		}
		if (remainCells == 0) {
			for (int i = 0; i < this->rowNum; i++) {
				for (int j = 0; j < this->colNum; j++) {
					gameGrid[i][j].setisRevealed(true);
				}
			}
			draw();
			std::cout << "You win" << "\n";
			this->hasWon = true;
			//break;
		}
	}


//check adjacent bomb nearby,and return the number of bumb.
/* Count all the mines in the 8 adjacent 
        cells 
  
            N.W   N   N.E 
              \   |   / 
               \  |  / 
            W----Cell----E 
                 / | \ 
               /   |  \ 
            S.W    S   S.E 
  
        Cell-->Current Cell (row, col) 
        N -->  North        (row-1, col) 
        S -->  South        (row+1, col) 
        E -->  East         (row, col+1) 
        W -->  West            (row, col-1) 
        N.E--> North-East   (row-1, col+1) 
        N.W--> North-West   (row-1, col-1) 
        S.E--> South-East   (row+1, col+1) 
        S.W--> South-West   (row+1, col-1) */



int check_adjacent(int row, int col){
	
	int count = 0; 
	// N -->  North        (row-1, col) 
	if (isValidMove (row-1, col) == true) 
	{ 
		if (isBomb (row-1, col) == true) 
		count++; 
	} 
	//S -->  South        (row+1, col) 
	if (isValidMove(row+1, col) == true) 
	{ 
		if (isBomb (row+1, col) == true) 
		count++; 
	} 
	// E -->  East         (row, col+1)
	if (isValidMove(row, col+1) == true) 
	{ 
		if (isBomb (row, col+1) == true) 
		count++; 
	} 
	// W -->  West            (row, col-1) 
	if (isValidMove(row, col-1) == true) 
	{ 
		if (isBomb (row, col-1) == true) 
		count++; 
	} 
	// N.E--> North-East   (row-1, col+1) 
	if (isValidMove(row-1, col+1l) == true) 
	{ 
		if (isBomb (row-1, col+1) == true) 
		count++; 
	} 
	// N.W--> North-West   (row-1, col-1) 
	if (isValidMove(row-1, col-1) == true) 
	{ 
		if (isBomb (row-1, col-1) == true) 
		count++; 
	} 
	//S.E--> South-East   (row+1, col+1)
	if (isValidMove(row+1, col+1) == true) 
	{ 
		if (isBomb (row+1, col+1) == true) 
		count++; 
	} 

	//S.W--> South-West   (row+1, col-1)
	if (isValidMove(row+1, col-1) == true) 
	{ 


		if (isBomb (row+1, col-1) == true) 
		count++; 
	}

	return count;
	
	


	}




private:
	bool hasWon = false;
	Cell** gameGrid;

};

// int placeFlag(int row, int col,int realboard[][8]){
// 	bool mark[64]; 
  
//     memset (mark, false, sizeof (mark)); 
  
//     // Continue until all random mines have been created. 
//     for (int i = 0; i < bomb; ) 
//      { 
//         int random = rand() % (64); 
//         int temp1 = random / 8; 
//         int temp2 = random % 8; 
  
//         // Add the mine if no mine is placed at this 
//         // position on the board 
//         if (mark[random] == false) 
//         { 
//             // Row Index of the Mine 
//             row = temp1; 
//             // Column Index of the Mine 
//             col = temp2; 
  
//             // Place the mine 
//             realBoard[row][col] = '*'; 
//             mark[random] = true; 
//             i++; 
//         } 
//     } 
  
//     return; 
// }


// A Function to print the current gameplay board 

// duplicate of draw()?
void printB(char bombset[][8]) 
{ 
    
    cout << "    "; 
  
    for (int i = 0; i < 8; i++) 
        cout << i; 
  
    cout << "\n\n"; 
  
    for (int i = 0; i < 8; i++) 
    { 
        cout << i; 
  
        for (int j = 0; j < 8; j++) 
            cout << bombset[i][j]; 
        cout << "\n\n"; 
    } 
    return; 
} 
// A Function to cheat by revealing where the mines are 
// under constructioni
void showBomb(char bombset[][8]) 
{ 
    cout << "The bombs locations are-\n"; 
    printB(bombset); 
    return; 
} 

int main() {

	//generateBombs();
	//draw();

	//// test code for Minesweeper class////
	
	int rowNum = 20;
	int colNum = 20;
	int mineNum = 10;
	int numPlayers = 2; // game doesn't deal with multiple players yet

	Minesweeper* game = new Minesweeper(rowNum, colNum, mineNum, numPlayers);
	game->generateBombs();

	// bool testIsBomb = game->isBomb(0,0);
	// bool testIsValid1 = game->isValidMove(10,10);
	// cout << "RowNum: " << game->rowNum << endl; // test rowNum is correct
	// cout << "colNum: " << game->colNum << endl;
	// cout << "numPlayers: " << game->numPlayers << endl;
	// cout << "hasWon: " << game->getHasWon() << endl;
	// cout << "gameGrid[0][0]: " << game->getGameGrid()[0][0].getNumber() << endl;
	// cout << "isBomb result: " << testIsBomb << endl;
	// cout << "isValid result: " << testIsValid1 << endl;
	// cout << "isValid result: " << testIsValid2 << endl;

	while(game->getHasWon() == false) {
		game->draw();
		int row, column;
		std::cout << "Please enter the cell you want to access ex) 1 1: ";
		std::cin >> row >> column;
		char markOrReveal;
		std::cout << "Enter m for mark, r to reveal it: ";
		std::cin >> markOrReveal;

		if (markOrReveal == 'm') {
			game->getGameGrid()[row][column].setIsMarked(true);
			game->mineCount--;
		} else if (markOrReveal == 'r') {
			game->getGameGrid()[row][column].setisRevealed(true);
			game->checkLose(row, column);
			//set it to a number of bomb nearby.
			std::cout<<"It has "<<game->check_adjacent<<"bomb near it ";

			
		} else {
			game->checkWin();
		}
	}

	return 0;
}
