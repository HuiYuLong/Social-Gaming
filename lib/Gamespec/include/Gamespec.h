#include <iostream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <limits>
#include <translator.h>
// Mot completed
class Gamespec {
    private:
    Configuration configuration;
    Constants constants;
    Variables variables;
    PerPlayer perplayer;
    //PerAudience peraudience;
    Rule rule;
    IndividualPlayer player;
    public:
    void play_game();
    void play_game(IndividualPlayer& player1, IndividualPlayer& player2);
    void play_with_others();
    void play_with_computer();
    void print_winner(IndividualPlayer& player1, IndividualPlayer& player2);
    bool is_game_over(IndividualPlayer& player1, IndividualPlayer& player2);

};
enum PlayChoice { Play, Exit };
enum PlayMode { with_Computer, with_Other_Players };
PlayChoice get_choice();
PlayMode get_play_mode();
PlayChoice get_choice()
{
   
   std::cout << "Welcome to Rock, Paper, Scissors.\nTo play, type '1'. \nTo exit, type '2'.\n";
   
   int choice;
   std::cin >> choice;
   if ( std::cin ) // Reading was successful.
   {
      switch (choice)
      {
         case 1:
            return Play;
         case 2:
            return Exit;
         default:
            std::cout << "You made an invalid choice. Try again...\n";
      }
      return get_choice();
   }
   else
   {
      std::cout << "You made an invalid choice. Try again...\n";

      // Clear the stream
      std::cin.clear();

      // Skip bad input
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

      // Try again.
      return get_choice();
   }
}

