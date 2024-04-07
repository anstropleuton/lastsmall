// Aplib includes... only one
#include "aplib.hpp"

// C includes
#ifndef _WIN32
#include <termios.h>
#include <unistd.h>
#else
typedef unsigned int tcflag_t;
struct termios {};
#define ECHO 1
#endif

// C++ includes
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using namespace aplib;
using namespace ansi;

// --------------------------------
// Possible addition to my collection (in aplib.hpp)
// --------------------------------

const std::string hide_cursor = "\033[?25l";
const std::string show_cursor = "\033[?25h";

// Globals
struct termios original_tty_state = {};
struct termios current_tty_state = {};
bool original_tty_state_set = false;

// Insert a flag to TTY
void insert_tty_flag(tcflag_t flag)
{
#ifndef _WIN32
    if (!original_tty_state_set)
    {
        tcgetattr(STDIN_FILENO, &original_tty_state);
        original_tty_state_set = true;
    }

    current_tty_state = original_tty_state;
    current_tty_state.c_lflag |= flag;
    tcsetattr(STDIN_FILENO, TCSANOW, &current_tty_state);
#endif
}

// Remove a flag from TTY
void remove_tty_flag(tcflag_t flag)
{
#ifndef _WIN32
    if (!original_tty_state_set)
    {
        tcgetattr(STDIN_FILENO, &original_tty_state);
        original_tty_state_set = true;
    }

    current_tty_state = original_tty_state;
    current_tty_state.c_lflag &= ~flag;
    tcsetattr(STDIN_FILENO, TCSANOW, &current_tty_state);
#endif
}

// Restore the default TTY flags
void restore_tty_flag()
{
#ifndef _WIN32
    if (original_tty_state_set)
    {
        current_tty_state = original_tty_state;
        tcsetattr(STDIN_FILENO, TCSANOW, &current_tty_state);
    }
#endif
}

// "Templates cannot be declared inside of a local class -- clang"
class Debugger {
public:
    bool yes;
    template <typename T>
    Debugger &operator<<(const T &data)
    {
        if (yes)
        {
            std::cout << data;
        }
        return *this;
    }
};

#ifdef DEBUG
#ifndef RANDOM_CRASH
#define RANDOM_CRASH false
#endif
#ifndef FRUSTRATION_MULTIPLIER
#define FRUSTRATION_MULTIPLIER 0.1
#endif
#ifndef YES_THING
#define YES_THING true
#endif
#else
#ifndef RANDOM_CRASH
#define RANDOM_CRASH true
#endif
#ifndef FRUSTRATION_MULTIPLIER
#define FRUSTRATION_MULTIPLIER 1
#endif
#ifndef YES_THING
#define YES_THING false
#endif
#endif

int main(int argc, char **argv)
{
    // --------------------------------
    // Flags setup
    // --------------------------------

    enum class Flags {
        help = 0,
        debug
    };
    std::vector<argp::Flag> flags = {
        argp::Flag { "Print this help message", { "help", "manual", "man" }, { 'h', 'm', '?' }, {}, 0 },
        argp::Flag { "Show each line ran", { "debug" }, { 'd' }, {}, 0 }
    };

    // --------------------------------
    // Program variables
    // --------------------------------

    std::vector<std::string> filenames;
    int debug = false;

    // --------------------------------
    // Command line flag handlers
    // --------------------------------

    auto Help = [&](const argp::Option &option) {
        (void)option;
        argp::print_help(flags, "Usage: Just use the program already... pass in the filename ok?\n       lastsmall filename.ls\nYou have some options tho:\n", "\nNo you don't report any bugs because I will never resolve them.\n");
        std::exit(0);
    };

    auto Debug = [&](const argp::Option &option) {
        (void)option;
        debug = !debug;
    };

    // --------------------------------
    // Command line parsing
    // --------------------------------

#ifndef DEBUG
    // Remove 5 seconds from their life expectancy every time they use this program
    std::string creepy_message = "You have to wait 5 seconds for the below pointless progress bar to finish counting";
    std::cout << red << creepy_message << reset << '\n';
#endif

    auto Pause = [](int milliseconds) {
        std::this_thread::sleep_for((std::chrono::milliseconds)(int)(milliseconds * FRUSTRATION_MULTIPLIER));
    };

    auto ProgressCity = [Pause](float width, float time) {
        std::cout << hide_cursor;
        remove_tty_flag(ECHO);
        for (float i = 0; i <= 100.0f; i++)
        {
            std::cout << "[";
            for (float j = 0; j < width; j++)
            {
                if (j < i / (100.0f / width)) std::cout << "#";
                else std::cout << " ";
            }
            std::cout << "] " << i << "%\r" << std::flush;
            Pause((int)time * 10);
        }
        insert_tty_flag(ECHO);
        std::cout << show_cursor << '\n';
    };

#ifndef DEBUG
    ProgressCity(creepy_message.size() - 7.0f, 5.0f);
#endif

    std::vector<argp::Option> options = argp::get_options_from_flags(argc, argv, flags);
    for (const argp::Option &option : options)
    {
        if (!option.unrecognized_option.empty())
        {
            if (option.unrecognized_option.substr(0, 2) == "--" || option.unrecognized_option.substr(0, 1) == "-" || option.unrecognized_option.substr(0, 1) == "/")
            {
                std::cout << "Idk what the heck are you talking about... What do you mean by " << red << option.unrecognized_option << reset << "??\n";
            }
            else
            {
                filenames.push_back(option.unrecognized_option);
            }
        }
        if (option.flag == &flags[(int)Flags::help]) Help(option);
        if (option.flag == &flags[(int)Flags::debug]) Debug(option);
    }

    // --------------------------------
    // Time wasting
    // --------------------------------

    std::srand(std::time(0));
    if (RANDOM_CRASH && std::rand() % 10 == 0)
    {
        std::cout << red << "You're too unlucky! The pointless timer just randomly failed!! " << reset << "This happens 1/10th of the times. Quitting please don't stop me I shall end my life (process) right now\n";
        std::cout << "Quitting...\n";
        ProgressCity(11 - 7.0f, 5.0f);
        return 0;
    }

    // Yeah we remind user if they forgot something, after they have ruined 5 seconds of their life
    if (filenames.empty())
    {
        std::cout << red << "You literally forgot the main thing... really??\n";
    }

    // --------------------------------
    // Program stuff
    // --------------------------------

    auto tokenize = [](const std::string &line) -> std::vector<std::string> {
        std::vector<std::string> result;
        auto Id = [](char c) -> bool {
            return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_') || (c >= '0' && c <= '9') || (c == '.');
        };
        if (line.empty()) return result;
        bool is_id = Id(line[0]);
        std::string temp;
        for (size_t i = 0; i < line.size(); i++)
        {
            char c = line[i];
            if (c == '\"')
            {
                while (line[++i] != '\"')
                {
                    if (line[i] == '\\')
                    {
                        i++;
                        if (line[i] == 'n')
                            temp += '\n';
                        else if (line[i] == 't')
                            temp += '\t';
                        else
                            temp += line[i];
                        continue;
                    }
                    temp += line[i];
                }
                result.push_back(temp);
                temp = "";
                continue;
            }
            if (c == '\\')
            {
                i++;
                if (line[i] == 'n')
                    temp += '\n';
                else if (line[i] == 't')
                    temp += '\t';
                else
                    temp += line[i];
                continue;
            }
            if (c == '#')
            {
                break;
            }
            if (c == ' ' || c == '\n' || c == '\t')
            {
                if (!temp.empty())
                {
                    result.push_back(temp);
                    temp = "";
                }
                continue;
            }
            bool id = Id(c);
            if (is_id == id)
            {
                // Remove the check and else part to allow operators with multiple symbols
                if (id) temp.push_back(c);
                else
                {
                    if (!temp.empty()) result.push_back(temp);
                    temp = c;
                }
            }
            else
            {
                is_id = id;
                if (!temp.empty()) result.push_back(temp);
                temp = c;
            }
        }
        if (!temp.empty())
        {
            result.push_back(temp);
        }
        return result;
    };

    auto ToInt = [](const std::string &string) -> int {
        std::stringstream ss = std::stringstream(string);
        int value;
        ss >> value;
        return value;
    };

    auto ToFloat = [](const std::string &string) -> float {
        std::stringstream ss = std::stringstream(string);
        float value;
        ss >> value;
        return value;
    };

    auto ToChar = [](const std::string &string) -> char {
        return string[0];
    };

    // --------------------------------
    // Actual program stuff
    // --------------------------------

    struct Variable {
        enum Type {
            _int,
            _float,
            _char,
            _string
        };
        std::string name;
        Type type;
        int value_int = 0;
        float value_float = 0.0f;
        char value_char = ' ';
        std::string value_string = "";
    };

    std::vector<Variable> variables;

    struct Jump {
        std::string name;
        size_t line_number;
    };

    std::vector<Jump> goneto_stack;

    for (const std::string &filename : filenames)
    {
        std::ifstream ifile = std::ifstream(filename);
        if (ifile.fail())
        {
            std::cout << "You idiot. You didn't realize that " << red << filename << reset << " does not exist... bruh moment\n";
        }
        std::string file_line;
        std::vector<std::string> lines;
        while (std::getline(ifile, file_line))
        {
            lines.push_back(file_line);
        }
        for (size_t i = 0; i < lines.size(); i++)
        {
            const std::string &line = lines[i];
            if (debug)
            {
                std::cout << green << filename << reset << ": # " << std::setw((int)std::log10(lines.size()) + 1) << green << i + 1 << reset << " : " << line << std::endl;
            }
            std::vector<std::string> tokens = tokenize(line);
            Debugger yesbug;
            yesbug.yes = YES_THING;
            for (auto t : tokens) yesbug << "[" << t << "]\n";
            if (tokens.empty())
            {
                continue;
            }
            try
            {
                auto Diagnose = [&](size_t line, std::string what) {
                    std::string message = "Line #" + std::to_string(line + 1) + " has witnessed a witch. Diagnosing...";
                    std::cout << message << '\n';
                    ProgressCity(message.size() - 7.0f, 2.0f);
                    std::cout << "Skill issue. ";
                    std::flush(std::cout);
                    Pause(2000);
                    std::cout << what;
                    std::flush(std::cout);
                    Pause(2000);
                    std::cout << "Eh... whatever I guess...\n";
                    Pause(2000);
                };

                auto WhereVar = [&](std::string name) -> size_t {
                    size_t location = (size_t)-1;
                    for (size_t j = 0; j < variables.size(); j++)
                    {
                        if (variables[j].name == name)
                        {
                            location = j;
                        }
                    }
                    return location;
                };

                if (tokens[0] == "int" || tokens[0] == "whole_number")
                {
                    if (WhereVar(tokens[1]) == (size_t)-1)
                    {
                        int value = 0;
                        if (tokens[2] == "=")
                        {
                            size_t varloc = WhereVar(tokens[3]);
                            if (varloc == (size_t)-1)
                            {
                                value = ToInt(tokens[3]);
                            }
                            else
                            {
                                value = variables[varloc].value_int;
                            }
                        }
                        variables.push_back(Variable { .name = tokens[1], .type = Variable::_int, .value_int = value });
                        yesbug << "You defined int named " << green << tokens[1] << reset << " with value " << value << '\n';
                    }
                    else
                    {
                        Diagnose(i, "Variable " + red + tokens[1] + reset + " already exists\n");
                    }
                }
                else if (tokens[0] == "float" || tokens[0] == "fake_or_real_number")
                {
                    if (WhereVar(tokens[1]) == (size_t)-1)
                    {
                        float value = 0;
                        if (tokens[2] == "=")
                        {
                            size_t varloc = WhereVar(tokens[3]);
                            if (varloc == (size_t)-1)
                            {
                                value = ToFloat(tokens[3]);
                            }
                            else
                            {
                                value = variables[varloc].value_float;
                            }
                        }
                        variables.push_back(Variable { .name = tokens[1], .type = Variable::_float, .value_float = value });
                        yesbug << "You defined float named " << green << tokens[1] << reset << " with value " << value << '\n';
                    }
                    else
                    {
                        Diagnose(i, "Variable " + red + tokens[1] + reset + " already exists\n");
                    }
                }
                else if (tokens[0] == "char" || tokens[0] == "idk_ascii_character")
                {
                    if (WhereVar(tokens[1]) == (size_t)-1)
                    {
                        char value = ' ';
                        if (tokens[2] == "=")
                        {
                            size_t varloc = WhereVar(tokens[3]);
                            if (varloc == (size_t)-1)
                            {
                                value = ToChar(tokens[3]);
                            }
                            else
                            {
                                value = variables[varloc].value_char;
                            }
                        }
                        variables.push_back(Variable { .name = tokens[1], .type = Variable::_char, .value_char = value });
                        yesbug << "You defined char named " << green << tokens[1] << reset << " with value " << value << '\n';
                    }
                    else
                    {
                        Diagnose(i, "Variable " + red + tokens[1] + reset + " already exists\n");
                    }
                }
                else if (tokens[0] == "string" || tokens[0] == "letters")
                {
                    if (WhereVar(tokens[1]) == (size_t)-1)
                    {
                        std::string value = "";
                        if (tokens[2] == "=")
                        {
                            size_t varloc = WhereVar(tokens[3]);
                            if (varloc == (size_t)-1)
                            {
                                value = tokens[3];
                            }
                            else
                            {
                                value = variables[varloc].value_string;
                            }
                        }
                        variables.push_back(Variable { .name = tokens[1], .type = Variable::_string, .value_string = value });
                        yesbug << "You defined string named " << green << tokens[1] << reset << " with value " << value << '\n';
                    }
                    else
                    {
                        Diagnose(i, "Variable " + red + tokens[1] + reset + " already exists\n");
                    }
                }
                else if (tokens[0] == "call" || tokens[0] == "literally_just_call")
                {
                    bool found_label = false;
                    for (size_t j = 0; j < lines.size(); j++)
                    {
                        std::vector<std::string> jump_tokens = tokenize(lines[j]);
                        if (jump_tokens.empty()) continue;
                        if (jump_tokens[1] == ":")
                        {
                            if (tokens[1] == jump_tokens[0])
                            {
                                found_label = true;
                                goneto_stack.push_back(Jump { tokens[1], i });
                                i = j;
                            }
                        }
                    }
                    if (found_label)
                    {
                        yesbug << "Jumping to " << green << tokens[1] << reset << '\n';
                    }
                    else
                    {
                        Diagnose(i, "Label " + red + tokens[1] + reset + " was not found in the entire file at all... what are you doing??\n");
                    }
                }
                else if (tokens[0] == "goto" || tokens[0] == "literally_just_go")
                {
                    bool found_label = false;
                    for (size_t j = 0; j < lines.size(); j++)
                    {
                        std::vector<std::string> jump_tokens = tokenize(lines[j]);
                        if (jump_tokens.empty()) continue;
                        if (jump_tokens[1] == ":")
                        {
                            if (tokens[1] == jump_tokens[0])
                            {
                                found_label = true;
                                i = j;
                            }
                        }
                    }
                    if (found_label)
                    {
                        yesbug << "Jumping to " << green << tokens[1] << reset << '\n';
                    }
                    else
                    {
                        Diagnose(i, "Label " + red + tokens[1] + reset + " was not found in the entire file at all... what are you doing??\n");
                    }
                }
                else if (tokens[0] == "jmp")
                {
                    i = ToInt(tokens[1]);
                }
                else if (tokens[0] == "return" || tokens[0] == "goback")
                {
                    if (goneto_stack.empty())
                    {
                        Diagnose(i, "You have not gone anywhere before you go back... idiot\n");
                    }
                    else
                    {
                        Jump last_jump = goneto_stack.back();
                        goneto_stack.erase(goneto_stack.end() - 1);
                        i = last_jump.line_number;
                        yesbug << "Jumping back to #" << green << i + 2 << reset << " (after " << green << last_jump.name << reset << ")" << '\n';
                    }
                }
                else if (tokens[0] == "scan" || tokens[0] == "beg")
                {
                    size_t varloc = WhereVar(tokens[1]);
                    if (varloc == (size_t)-1)
                    {
                        Diagnose(i, "Well how many freaking times do I have to tell you that variable " + red + tokens[1] + reset + " does not exist for scanning?? What a jerk...\n");
                    }
                    else
                    {
                        Variable &var = variables[varloc];
                        int value_int = 0;
                        float value_float = 0;
                        char value_char = 0;
                        switch (var.type)
                        {
                            case Variable::_int:
                                std::cin >> value_int;
                                var.value_int = value_int;
                                break;
                            case Variable::_float:
                                std::cin >> value_float;
                                var.value_float = value_float;
                                break;
                            case Variable::_char:
                                std::cin >> value_char;
                                var.value_char = value_char;
                                break;
                            case Variable::_string:
                                std::getline(std::cin, var.value_string);
                                break;
                        }
                    }
                }
                else if (tokens[0] == "print" || tokens[0] == "seg")
                {
                    size_t varloc = WhereVar(tokens[1]);
                    if (varloc == (size_t)-1)
                    {
                        Diagnose(i, "Hell no I am not repeating this again... Variable " + red + tokens[1] + reset + " does not exist for printing\n");
                    }
                    else
                    {
                        Variable &var = variables[varloc];
                        switch (var.type)
                        {
                            case Variable::_int:
                                std::cout << var.value_int;
                                break;
                            case Variable::_float:
                                std::cout << var.value_float;
                                break;
                            case Variable::_char:
                                std::cout << var.value_char;
                                break;
                            case Variable::_string:
                                std::cout << var.value_string;
                                break;
                        }
                    }
                }
                else if (tokens[0] == "delete" || tokens[0] == "obliterate" || tokens[0] == "explode")
                {
                    size_t varloc = WhereVar(tokens[1]);
                    if (varloc == (size_t)-1)
                    {
                        Diagnose(i, "Damn... Variable " + tokens[1] + " does not exist for deletion\n");
                    }
                    else
                    {
                        variables.erase(variables.begin() + varloc);
                    }
                }
                else if (tokens[0] == "branch")
                {
                    size_t varloc = WhereVar(tokens[1]);
                    if (varloc == (size_t)-1)
                    {
                        Diagnose(i, "Oof... Variable " + tokens[1] + " does not exist for branching\n");
                    }
                    else
                    {
                        bool do_jump = false;
                        switch (variables[varloc].type)
                        {
                            case Variable::_int:
                                do_jump = variables[varloc].value_int != 0;
                                break;
                            case Variable::_float:
                                do_jump = variables[varloc].value_int != 0.0f;
                                break;
                            case Variable::_char:
                                do_jump = variables[varloc].value_int != ' ';
                                break;
                            case Variable::_string:
                                do_jump = variables[varloc].value_string != "";
                                break;
                        }
                        if (do_jump)
                        {
                            bool found_label = false;
                            for (size_t j = 0; j < lines.size(); j++)
                            {
                                std::vector<std::string> jump_tokens = tokenize(lines[j]);
                                if (jump_tokens.empty()) continue;
                                if (jump_tokens[1] == ":")
                                {
                                    if (tokens[2] == jump_tokens[0])
                                    {
                                        found_label = true;
                                        i = j;
                                    }
                                }
                            }
                            if (found_label)
                            {
                                yesbug << "Branching to " << green << tokens[2] << reset << '\n';
                            }
                            else
                            {
                                Diagnose(i, "Label " + red + tokens[2] + reset + " was not found in the entire file at all to be branched... like how the heck are you...\n");
                            }
                        }
                    }
                }
                else if (tokens[0] == "exists")
                {
                    size_t varloc = WhereVar(tokens[1]);
                    if (varloc != (size_t)-1)
                    {
                        bool found_label = false;
                        for (size_t j = 0; j < lines.size(); j++)
                        {
                            std::vector<std::string> jump_tokens = tokenize(lines[j]);
                            if (jump_tokens.empty()) continue;
                            if (jump_tokens[1] == ":")
                            {
                                if (tokens[2] == jump_tokens[0])
                                {
                                    found_label = true;
                                    i = j;
                                }
                            }
                        }
                        if (found_label)
                        {
                            yesbug << "Branching to " << green << tokens[2] << reset << '\n';
                        }
                        else
                        {
                            Diagnose(i, "Label " + red + tokens[2] + reset + " was not found in the entire file at all to be branched... like how the heck are you...\n");
                        }
                    }
                }
                else if (tokens[0] == "exists")
                {
                    size_t varloc = WhereVar(tokens[1]);
                    if (varloc != (size_t)-1)
                    {
                        bool found_label = false;
                        for (size_t j = 0; j < lines.size(); j++)
                        {
                            std::vector<std::string> jump_tokens = tokenize(lines[j]);
                            if (jump_tokens.empty()) continue;
                            if (jump_tokens[1] == ":")
                            {
                                if (tokens[2] == jump_tokens[0])
                                {
                                    found_label = true;
                                    i = j;
                                }
                            }
                        }
                        if (found_label)
                        {
                            yesbug << "Branching to " << green << tokens[2] << reset << '\n';
                        }
                        else
                        {
                            Diagnose(i, "Label " + red + tokens[2] + reset + " was not found in the entire file at all to be branched... like how the heck are you...\n");
                        }
                    }
                }
                else if (tokens[0] == "exit" || tokens[0] == "escape_the_torture")
                {
                    if (!variables.empty()) variables.clear();
                    break;
                }
                else if (tokens[1] != ":")
                {
                    size_t varloc = WhereVar(tokens[0]);
                    if (varloc == (size_t)-1)
                    {
                        Diagnose(i, "That's it. I am done. Variable " + red + bold + underline + tokens[0] + reset + " never existed (or is deleted now) but you decided to use it anyways. I am gone\n");
                        std::cout << "Quitting...\n";
                        ProgressCity(11 - 7.0f, 5.0f);
                        break;
                    }
                    else
                    {
                        if (tokens[1] != "=")
                        {
                            Diagnose(i, "Wdym by that??\n");
                        }
                        else
                        {
                            size_t var_left = (size_t)-1;
                            size_t var_right = (size_t)-1;
                            size_t var_mid = (size_t)-1;
                            if (tokens.size() > 2) var_left = WhereVar(tokens[2]);
                            if (tokens.size() > 4) var_right = WhereVar(tokens[4]);
                            if (tokens.size() > 3) var_mid = WhereVar(tokens[3]);
                            auto RequestL = [&]() -> bool {
                                if (var_left == (size_t)-1)
                                {
                                    Diagnose(i, "Variable... uff, " + red + tokens[2] + reset + " does not exist... yey");
                                    return false;
                                }
                                return true;
                            };
                            auto RequestR = [&]() -> bool {
                                if (var_right == (size_t)-1)
                                {
                                    Diagnose(i, "Variable... uff, " + red + tokens[4] + reset + " does not exist... yey");
                                    return false;
                                }
                                return true;
                            };
                            auto RequestM = [&]() -> bool {
                                if (var_mid == (size_t)-1)
                                {
                                    Diagnose(i, "Variable... uff, " + red + tokens[3] + reset + " does not exist... yey");
                                    return false;
                                }
                                return true;
                            };
                            auto RequestLR = [&]() -> bool {
                                return RequestL() && RequestR();
                            };

                            if (tokens.size() == 3)
                            {
                                switch (variables[varloc].type)
                                {
                                    case Variable::_int:
                                        variables[varloc].value_int = variables[var_left].value_int;
                                        break;
                                    case Variable::_float:
                                        variables[varloc].value_float = variables[var_left].value_float;
                                        break;
                                    case Variable::_char:
                                        variables[varloc].value_char = variables[var_left].value_char;
                                        break;
                                    case Variable::_string:
                                        variables[varloc].value_string = variables[var_left].value_string;
                                        break;
                                }
                            }
                            else if (tokens[2] == "!")
                            {
                                if (RequestM())
                                {
                                    switch (variables[varloc].type)
                                    {
                                        case Variable::_int:
                                            variables[varloc].value_int = !variables[var_mid].value_int;
                                            break;
                                        case Variable::_float:
                                            variables[varloc].value_float = !variables[var_mid].value_float;
                                            break;
                                        case Variable::_char:
                                            variables[varloc].value_char = !variables[var_mid].value_char;
                                            break;
                                        case Variable::_string:
                                            Diagnose(i, "What do you mean by noting a string from another string??");
                                            break;
                                    }
                                }
                            }
                            else if (tokens[3] == "+")
                            {
                                if (RequestLR())
                                {
                                    switch (variables[varloc].type)
                                    {
                                        case Variable::_int:
                                            variables[varloc].value_int = variables[var_left].value_int + variables[var_right].value_int;
                                            break;
                                        case Variable::_float:
                                            variables[varloc].value_float = variables[var_left].value_float + variables[var_right].value_float;
                                            break;
                                        case Variable::_char:
                                            variables[varloc].value_char = variables[var_left].value_char + variables[var_right].value_char;
                                            break;
                                        case Variable::_string:
                                            variables[varloc].value_string = variables[var_left].value_string + variables[var_right].value_string;
                                            break;
                                    }
                                }
                            }
                            else if (tokens[3] == "-")
                            {
                                if (RequestLR())
                                {
                                    switch (variables[varloc].type)
                                    {
                                        case Variable::_int:
                                            variables[varloc].value_int = variables[var_left].value_int - variables[var_right].value_int;
                                            break;
                                        case Variable::_float:
                                            variables[varloc].value_float = variables[var_left].value_float - variables[var_right].value_float;
                                            break;
                                        case Variable::_char:
                                            variables[varloc].value_char = variables[var_left].value_char - variables[var_right].value_char;
                                            break;
                                        case Variable::_string:
                                            variables[varloc].value_string = variables[var_left].value_string;
                                            size_t pos = 0;
                                            while ((pos = variables[varloc].value_string.find(variables[var_right].value_string, pos)) != std::string::npos)
                                            {
                                                variables[varloc].value_string.erase(pos, variables[var_right].value_string.length());
                                            }
                                            break;
                                    }
                                }
                            }
                            else if (tokens[3] == "*")
                            {
                                if (RequestLR())
                                {
                                    switch (variables[varloc].type)
                                    {
                                        case Variable::_int:
                                            variables[varloc].value_int = variables[var_left].value_int * variables[var_right].value_int;
                                            break;
                                        case Variable::_float:
                                            variables[varloc].value_float = variables[var_left].value_float * variables[var_right].value_float;
                                            break;
                                        case Variable::_char:
                                            variables[varloc].value_char = variables[var_left].value_char * variables[var_right].value_char;
                                            break;
                                        case Variable::_string:
                                            Diagnose(i, "What do you mean by multiplying a string from another string??");
                                            break;
                                    }
                                }
                            }
                            else if (tokens[3] == "/")
                            {
                                if (RequestLR())
                                {
                                    switch (variables[varloc].type)
                                    {
                                        case Variable::_int:
                                            variables[varloc].value_int = variables[var_left].value_int / variables[var_right].value_int;
                                            break;
                                        case Variable::_float:
                                            variables[varloc].value_float = variables[var_left].value_float / variables[var_right].value_float;
                                            break;
                                        case Variable::_char:
                                            variables[varloc].value_char = variables[var_left].value_char / variables[var_right].value_char;
                                            break;
                                        case Variable::_string:
                                            Diagnose(i, "What do you mean by dividing a string from another string??");
                                            break;
                                    }
                                }
                            }
                            else if (tokens[3] == "%")
                            {
                                if (RequestLR())
                                {
                                    switch (variables[varloc].type)
                                    {
                                        case Variable::_int:
                                            variables[varloc].value_int = variables[var_left].value_int % variables[var_right].value_int;
                                            break;
                                        case Variable::_float:
                                            variables[varloc].value_float = std::fmod(variables[var_left].value_float, variables[var_right].value_float);
                                            break;
                                        case Variable::_char:
                                            variables[varloc].value_char = variables[var_left].value_char % variables[var_right].value_char;
                                            break;
                                        case Variable::_string:
                                            Diagnose(i, "What do you mean by modulating a string from another string??");
                                            break;
                                    }
                                }
                            }
                            else if (tokens[3] == "^")
                            {
                                if (RequestLR())
                                {
                                    switch (variables[varloc].type)
                                    {
                                        case Variable::_int:
                                            variables[varloc].value_int = std::pow(variables[var_left].value_int, variables[var_right].value_int);
                                            break;
                                        case Variable::_float:
                                            variables[varloc].value_float = std::pow(variables[var_left].value_float, variables[var_right].value_float);
                                            break;
                                        case Variable::_char:
                                            variables[varloc].value_char = std::pow(variables[var_left].value_char, variables[var_right].value_char);
                                            break;
                                        case Variable::_string:
                                            Diagnose(i, "What do you mean by exponentiating a string from another string??");
                                            break;
                                    }
                                }
                            }
                            else if (tokens[3] == "&")
                            {
                                if (RequestLR())
                                {
                                    switch (variables[varloc].type)
                                    {
                                        case Variable::_int:
                                            variables[varloc].value_int = variables[var_left].value_int && variables[var_right].value_int;
                                            break;
                                        case Variable::_float:
                                            variables[varloc].value_float = variables[var_left].value_float && variables[var_right].value_float;
                                            break;
                                        case Variable::_char:
                                            variables[varloc].value_char = variables[var_left].value_char && variables[var_right].value_char;
                                            break;
                                        case Variable::_string:
                                            Diagnose(i, "What do you mean by anding a string from another string??");
                                            break;
                                    }
                                }
                            }
                            else if (tokens[3] == "|")
                            {
                                if (RequestLR())
                                {
                                    switch (variables[varloc].type)
                                    {
                                        case Variable::_int:
                                            variables[varloc].value_int = variables[var_left].value_int || variables[var_right].value_int;
                                            break;
                                        case Variable::_float:
                                            variables[varloc].value_float = variables[var_left].value_float || variables[var_right].value_float;
                                            break;
                                        case Variable::_char:
                                            variables[varloc].value_char = variables[var_left].value_char || variables[var_right].value_char;
                                            break;
                                        case Variable::_string:
                                            Diagnose(i, "What do you mean by oring a string from another string??");
                                            break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            catch (std::exception e)
            {
                yesbug << "Invalid syntax or smth, " << red << e.what() << reset << '\n';
            }
        }
        ifile.close();
    }
}
