#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <chrono>
#include <mutex>
#include <cstdlib>
#include <cctype>
#include <climits>

#include <conio.h>
#include <windows.h>

using namespace std;

// Shared state, all guarded by state_mutex
mutex state_mutex;
string marquee_text = "Hello world!";
bool marquee_running = false;
bool program_running = true;
int speed = 100;
int consoleWidth = 50;
int marquee_pos = consoleWidth;
string last_message = "";
string current_input = "";

void enable_terminal() {
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD dwMode = 0;
  GetConsoleMode(hOut, &dwMode);
  dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  SetConsoleMode(hOut, dwMode);
}

// Returns true and fills c if a key is waiting; never blocks.
bool try_read_char(char& c) {
  if (_kbhit()) {
    c = static_cast<char>(_getch());
    return true;
  }
  return false;
}

const char BACKSPACE_CODE = 8; // what _getch() sends for backspace

void clear_screen() {
#ifdef _WIN32
  system("cls");
#else
  system("clear");
#endif
}

string help_text() {
  return
    "help - displays the commands and its description\n"
    "start_marquee - starts the marquee \"animation\"\n"
    "stop_marquee - stops the marquee \"animation\"\n"
    "set_text - accepts a text input and displays it as a marquee\n"
    "set_speed - sets the marquee animation refresh in milliseconds\n"
    "exit - terminates the console";
}

int choice_map(const string& command) {
  if (command == "help") return 1;
  if (command == "start_marquee") return 2;
  if (command == "stop_marquee") return 3;
  if (command == "set_text") return 4;
  if (command == "set_speed") return 5;
  if (command == "exit") return 6;
  return 0;
}

// Executes one submitted command line, updating shared state.
// Returns false only when the program should exit.
bool execute_command(const string& line) {
  stringstream ss(line);
  string command;
  ss >> command;
  string argument;
  getline(ss >> ws, argument);

  int choice = choice_map(command);
  lock_guard<mutex> lock(state_mutex);

  switch (choice) {
    case 1:
      last_message = help_text();
      break;
    case 2:
      marquee_running = true;
      last_message = "Marquee started.";
      break;
    case 3:
      marquee_running = false;
      last_message = "Marquee stopped.";
      break;
    case 4:
      if (argument.empty()) {
        last_message = "Usage: set_text [text for marquee]";
      } else {
        marquee_text = argument;
        last_message = "Text saved for marquee: " + argument;
      }
      break;
    case 5:
      if (argument.empty()) {
        last_message = "Usage: set_speed [milliseconds]";
      } else {
        try {
          int parsed = stoi(argument);
          if (parsed > 0) {
            speed = parsed;
            last_message = "Speed saved for marquee: " + argument + " ms";
          } else {
            last_message = "Speed must be greater than 0.";
          }
        } catch (...) {
          last_message = "Invalid speed value. Please enter a number.";
        }
      }
      break;
    case 6:
      last_message = "Terminating console...";
      program_running = false;
      return false;
    default:
      last_message = "No command \"" + command + "\"";
      break;
  }
  return true;
}

// mutates marquee position
void run_marquee_logic() {
  while (true) {
    int delay;
    bool running, alive;
    {
      lock_guard<mutex> lock(state_mutex);
      running = marquee_running;
      alive = program_running;
      delay = speed;
    }
    if (!alive) break;

    if (running) {
      lock_guard<mutex> lock(state_mutex);
      marquee_pos--;
      if (marquee_pos <= -static_cast<int>(marquee_text.length())) {
        marquee_pos = consoleWidth;
      }
    }
    this_thread::sleep_for(chrono::milliseconds(running ? delay : 50));
  }
}

// Builds the marquee display line from current shared state.
string build_marquee_line() {
  string display(consoleWidth, ' ');
  for (int i = 0; i < (int)marquee_text.length(); i++) {
    int p = marquee_pos + i;
    if (p >= 0 && p < consoleWidth) display[p] = marquee_text[i];
  }
  return display;
}

void render() {
  string text, msg, input;
  {
    lock_guard<mutex> lock(state_mutex);
    text = build_marquee_line();
    msg = last_message;
    input = current_input;
  }

  ostringstream oss;
  oss << "\x1b[H";
  oss << "Welcome to CSOPESY!" << "\x1b[K\n";
  oss << "\x1b[K\n";
  oss << "Group developers:" << "\x1b[K\n";
  oss << "Chua, Myka Nadine" << "\x1b[K\n";
  oss << "Lim, Julienne Skye" << "\x1b[K\n";
  oss << "Ong, Eiress Bassey" << "\x1b[K\n";
  oss << "Xu, Kai Wen" << "\x1b[K\n";
  oss << "\x1b[K\n";
  oss << "Version date: 2026-09-25" << "\x1b[K\n";
  oss << "\x1b[K\n";
  oss << "[" << text << "]" << "\x1b[K\n";
  oss << "\x1b[K\n";

  if (!msg.empty()) {
    stringstream msg_stream(msg);
    string line;
    while (getline(msg_stream, line)) {
      oss << line << "\x1b[K\n";
    }
    oss << "\x1b[K\n";
  }

  oss << "Command> " << input << "\x1b[K";
  oss << "\x1b[J";

  cout << oss.str() << flush;
}

int main() {
  enable_terminal();
  clear_screen();

  thread t1(run_marquee_logic);

  int last_pos = INT_MIN;
  string last_msg = "";
  string last_input = "";

  while (true) {
    string submitted_line;
    bool submitted = false;
    bool something_happened = false;

    char c;
    if (try_read_char(c)) {
      lock_guard<mutex> lock(state_mutex);
      if (c == '\r' || c == '\n') {
        submitted_line = current_input;
        current_input.clear();
        submitted = true;
      } else if (c == BACKSPACE_CODE) {
        if (!current_input.empty()) current_input.pop_back();
      } else if (isprint(static_cast<unsigned char>(c))) {
        current_input.push_back(c);
      }
      something_happened = true;
    }

    if (submitted) {
      bool keep_going = execute_command(submitted_line);
      something_happened = true;
      if (!keep_going) {
        render();
        break;
      }
    }

    if (!something_happened) {
      int cur_pos;
      string cur_msg, cur_input;
      {
        lock_guard<mutex> lock(state_mutex);
        cur_pos = marquee_pos;
        cur_msg = last_message;
        cur_input = current_input;
      }
      if (cur_pos != last_pos || cur_msg != last_msg || cur_input != last_input) {
        something_happened = true;
      }
    }

    if (something_happened) {
      render();
      lock_guard<mutex> lock(state_mutex);
      last_pos = marquee_pos;
      last_msg = last_message;
      last_input = current_input;
    } else {
      this_thread::sleep_for(chrono::milliseconds(5));
    }
  }

  {
    lock_guard<mutex> lock(state_mutex);
    program_running = false;
  }

  if (t1.joinable()) {
    t1.join();
  }

  cout << "\n";
  return 0;
}
