#include <iostream>

using namespace std;

class Cell {
public:
	Cell(int number, bool isHidden): number(number), isHidden(isHidden){}
	int getNumber() const { return number;}
	bool getIsHidden() const { return isHidden;}
	void setNumber(const int& number) {this->number = number;}
	void setIsHidden(const bool& isHidden) {this->isHidden = isHidden;}

private:
	int number; // number can be -1,0,1,2,3,4,5,6,7,8. -1 means the cell has a bomb.
	bool isHidden; // Is the cell selected by the user
};
 
int main() {
	
	return 0;
}