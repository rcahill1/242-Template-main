#include <functional>
#include <string>
#include <vector>
#include "lcd/lcd.h"

namespace as {
    class Auton {
        public:
            std::string Name;
            std::string Description;
            std::function<void()> func;

            Auton(std::string Name, std::string Description, std::function<void()> func) {
                this->Name = Name;
                this->Description = Description;
                this->func = func;
            }
    };
    
    inline std::vector<Auton> autons = {};
    inline int current_auton = -1;

    inline void render_page() {
        lcd::clear();
        if (current_auton == -1) {
            lcd::print(0, "No autons tehe <3");
        } else {
            lcd::print(0, "Auton " + std::to_string(current_auton+1) + "/" + std::to_string(autons.size()));
            lcd::print(1, autons[current_auton].Name);
            lcd::print(2, autons[current_auton].Description);
        }
    }
    inline std::function<void()> increment = [](){
        as::current_auton++;
        if (current_auton == autons.size()) {current_auton = 0;}
        as::render_page();
    };
    inline std::function<void()> decrement = [](){
        as::current_auton++;
        if (current_auton < 0) {current_auton = autons.size()-1;}
        as::render_page();
    };
    inline void call_selected_auton() {
        if (current_auton != -1) { autons[current_auton].func(); }
    }
    inline void init() {
        lcd::clear();
        if (autons.size() != 0) {
            current_auton = 0;
        }
        as::render_page();
    }
    inline void add_auton(as::Auton auton) { as::autons.push_back(auton); }
}