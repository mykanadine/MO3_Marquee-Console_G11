#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <chrono>
#include<mutex>


using namespace std;


/*
todo: 
fix threading
stop_marquee
exit
*/
//Shared states
string marquee_text = "Hello World, Hello Universe!";
bool marquee_running = false;
bool program_running = true;
int speed = 100;
int consoleWidth = 50;

mutex state_mutex; 
// Guards marquee_text, marquee_running, speed, and program_running

void header() {
  std::cout << "Welcome to CSOPESY!\n";
  std::cout << "\n";
  std::cout << "Group developers:\n";
  std::cout << "Chua, Myka Nadine\n";
  std::cout << "Lim, Julienne Skye\n";
  std::cout << "Ong, Eiress Bassey\n";
  std::cout << "Xu, Kai Wen\n";
  std::cout << "\n";
  std::cout << "Version date: 2026-09-25";
  std::cout << "\n";
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

void set_text(const std::string& input) {
 {
        std::lock_guard<std::mutex> lock(state_mutex);
        marquee_text = input;
    }
    std::cout << "Text saved for marquee: " + input + "\n\n";
}

void set_speed(const std::string& input) {
 try {
        int parsed_speed = std::stoi(input);
        if (parsed_speed > 0) {
            {
                lock_guard<std::mutex> lock(state_mutex);
                speed = parsed_speed;
            }
            cout << "Speed saved for marquee: " + input + " ms\n\n";
        } else {
            cout << "Speed must be greater than 0.\n\n";
        }
    } catch (...) {
        cout << "Invalid speed value. Please enter a number.\n\n";
    }
}

void start_marquee() {
  {
  lock_guard<mutex> lock(state_mutex);
  marquee_running = true;
  }
  cout << "Marquee started. \n\n";
}
void stop_marquee() {
  {
    lock_guard<mutex> lock(state_mutex);
    marquee_running = false;
  }
  cout << "Marquee stopped.\n\n";
}

void run_marquee_logic(){
  int pos = consoleWidth;
  while(true){
    string text;
    bool is_running;
    bool keep_program_alive;
    int delay;
    {
      lock_guard<mutex> lock(state_mutex);
      text = marquee_text;
      is_running = marquee_running;
      keep_program_alive = program_running;
      delay = speed;
    }
    if (!keep_program_alive) break; // Exit loop on program exit
    if (is_running) {

      // Render single marquee frame
      marquee(text, pos, consoleWidth);
      //move left
      pos--;
      if (pos <= -static_cast<int>(text.length())) {
        pos = consoleWidth;
      }

      this_thread::sleep_for(chrono::milliseconds(delay));
    } else {
      // Idle pause delay
      this_thread::sleep_for(chrono::milliseconds(50));
    

    }
      

  }


}

int main() {
  
 

  header();
  thread t1(run_marquee_logic);

  while(true) {
  
    std::cout << "Command> ";
    std::string input;
    if (!std::getline(cin >> std::ws, input)) break;

    std::stringstream input_stream(input);
    std::string command;
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
        start_marquee();
        std::cout << "\n\n";
      break;
      case 3:
        stop_marquee();
        break;
      case 4:
        if (argument.empty()) {
          std::cout << "Usage: set_text [text for marquee]\n";
        } else {
          set_text(argument);
        }
        break;
      case 5:
        if (argument.empty()) {
          std::cout << "Usage: set_speed [milliseconds]\n";
        } else {
          set_speed(argument);
        }
        break;
      case 6:
        std::cout << "Terminating console...";
        {
          lock_guard<mutex> lock(state_mutex);
          marquee_running = false;
          program_running = false;
        }
        if (t1.joinable()) {
          t1.join(); // Clean shut down of thread
        }
        return 0;
      default:
        break;
    }
  }
  // Safety fallback thread join
  {
    lock_guard<mutex> lock(state_mutex);
    program_running = false;
  }
  if (t1.joinable()) {
    t1.join();
  }

  return 0;
}
