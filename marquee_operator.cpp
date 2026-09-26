#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <chrono>

using namespace std;

/*
todo: 
stop_marquee
exit
*/

void header() {
  std::cout << "Welcome to CSOPESY!\n";
  std::cout << "\n";
  std::cout << "Group developers:\n";
  std::cout << "Ab, Cidi\n";
  std::cout << "De, Fig\n";
  std::cout << "\n";
  std::cout << "Version date: 2030-09-19";
  std::cout << "\n";
}

void marquee(const std::string& text, int pos, int width) {
  std::string display(width, ' ');

  for(int i = 0; i < text.length(); i++) {
    int current_pos = pos + i;
    if (current_pos >= 0 && current_pos < width) {
      display[current_pos] = text[i];
    }
  }

  std::cout << "\r" << display << std::flush;
}

void help() {
  std::cout << "help - displays the commands and its description\n";
  std::cout << "start_marquee - starts the marquee \"animation\"\n";
  std::cout << "stop_marquee - stops the marquee \"animation\"\n";
  std::cout << "set_text - accepts a text input and displays it as a marquee\n";
  std::cout << "set_speed - sets the marquee animation refresh in milliseconds\n";
  std::cout << "exit - terminates the console\n\n";
}

int choice_map(std::string& command) {
  if (command == "help") {
    return 1;
  } else if (command == "start_marquee") {
    return 2;
  } else if (command == "stop_marquee") {
    return 3;
  } else if (command == "set_text") {
    return 4;
  } else if (command == "set_speed") {
    return 5;
  } else if (command == "exit") {
    return 6;
  } else {
    return 0;
  }
}

void set_text(std::string& input, std::string& text) {
  text = input;
  std::cout << "Text saved for marquee: " + input + "\n\n";
}

int set_speed(std::string& input, int* speed) {
  *speed = std::stoi(input);
  std::cout << "Speed saved for marquee: " + input + "\n\n";
  return *speed;
}

void start_marquee(int pos, std::string& text, int speed) {
  for (int i = pos; i > -(int)text.length(); --i) {
    marquee(text, i, pos);
    std::this_thread::sleep_for(std::chrono::milliseconds(speed));
  }
}

int main() {
  std::string text = "Hello World!";
  std::string command = "";
  int consoleWidth = 50; //width of window
  int speed = 100;

  header();

  while(true) {
  
    std::cout << "Command> ";
    std::string input;
    std::getline(cin >> std::ws, input);

    std::stringstream input_stream(input);
    input_stream >> command;

    std::string argument;
    std::getline(input_stream >> std::ws, argument);

    int choice = choice_map(command);

    if (choice == 0) {
      std::cout << "No command \"" + command + "\"\n\n";
    }

    switch(choice) {
      case 1:
        help();
        break;
      case 2:
        start_marquee(consoleWidth, text, speed);
        std::cout << "\n\n";
      break;
      case 3:
        break;
      case 4:
        if (argument.empty()) {
          std::cout << "Usage: set_text [text for marquee]\n";
        } else {
          set_text(argument, text);
        }
        break;
      case 5:
        if (argument.empty()) {
          std::cout << "Usage: set_speed [milliseconds]\n";
        } else {
          set_speed(argument, &speed);
        }
        break;
      case 6:
        std::cout << "Terminating console...";
        return 0;
      default:
        break;
    }
  }

  return 0;
}
