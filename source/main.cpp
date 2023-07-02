#include <switch.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <vector>
#include <cstdint>
#include <cstdio>

using namespace std;


inline string getCurrentDateTime( string s ){
    time_t now = time(0);
    struct tm  tstruct;
    char  buf[80];
    tstruct = *localtime(&now);
    if(s=="now")
        strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
    else if(s=="date")
        strftime(buf, sizeof(buf), "%Y-%m-%d", &tstruct);
    return string(buf);
};
inline void Logger( string logMsg ){

    string filePath = "log_"+getCurrentDateTime("date")+".txt";
    string now = getCurrentDateTime("now");
    ofstream ofs(filePath.c_str(), std::ios_base::out | std::ios_base::app );
    ofs << now << '\t' << logMsg << '\n';
    ofs.close();
}

vector<int> find_addresses(ifstream& file, int num) {
    vector<int> addresses; // создаем вектор для хранения адресов
    file.seekg(0, ios::end); // переходим в конец файла
    int fileSize = file.tellg(); // получаем размер файла
    // Logger("File size is "+fileSize);
    file.seekg(0, ios::beg); // переходим в начало файла
    char buffer[4]; // буфер для чтения 4-х байтов
    int pos = 0; // переменная для хранения текущей позиции в файле

    while (pos < fileSize) { // пока не достигнут конец файла
        file.read(buffer, 4); // читаем 4 байта
        // преобразуем байты в 32-битное число с little endian
        int val = (buffer[0] & 0xFF) | ((buffer[1] & 0xFF) << 8) |
                  ((buffer[2] & 0xFF) << 16) | ((buffer[3] & 0xFF) << 24);

        if (val == num) { // если число найдено
            addresses.push_back(pos); // добавляем текущий адрес в вектор
        }

        pos += 4; // увеличиваем текущую позицию на 4 байта
    }
    return addresses; // возвращаем вектор найденных адресов
}

void changing(fstream& file, const int address, int newVal) {
    // Set the position in the file to the desired location
    file.seekg(address, ios::beg);

    // Read the desired number of bytes and store them in an array
    const int length = 4; // Value length in bytes
    char buffer[length];
    string voltage;
    file.read(buffer, length);

    // LE moment
    char reversed[length]{};
    for (int i = 0; i < length; i++) {
        reversed[i] = buffer[length - i - 1];
    }

    // Convert the byte array to a number
    int value;
    memcpy(&value, reversed, length);

    // Output the value to the console
    cout << "Value: " << hex << setw(length * 2) << setfill('0') << value << endl;

    // Set the position in the file to the desired address to write
    file.seekp(address, ios::beg);

    // Convert the byte array to a 32-bit integer (int)
    int number = 0;
    memcpy(&number, buffer, sizeof(number));
    cout.setf(ios::dec);
    file.write(reinterpret_cast<const char*>(&newVal), sizeof(newVal));
}

bool copyFile(const std::string& from, const std::string& to)
{
    std::ifstream src(from, std::ios::binary);
    std::ofstream dst(to, std::ios::binary);

    if (src.good() && dst.good()) {
        dst << src.rdbuf();
        return true;
    }
    return false;
}

void Clear()
{
    std::cout<< u8"\033[2J\033[1;1H"; //Using ANSI Escape Sequences 
}

int main(int argc, char* argv[])
{
    consoleInit(NULL);
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    PadState pad;
    padInitializeDefault(&pad);
    int curScreen = 1;
    socketInitializeDefault();              // Initialize sockets
    nxlinkStdio();                          // Redirect stdout and stderr over the network to nxlink

    std::ifstream src("/switch/4IFIR-configurator/backup.kip", std::ios::binary);
    if (!src.good()) {
        if (copyFile("/atmosphere/kips/loader.kip", "/switch/4IFIR-configurator/backup.kip"))
        {
            std::cout << "loader.kip has been successfully reserved!\n\n";
            std::cout << "Welcome to 4IFIR GPU voltage table changer 0.6\n\nSelect desired voltages:\n";
            std::cout << "Y. 4IFIR stock voltages\n";
            std::cout << "X. 4IFIR stage voltages\n";
            std::cout << "L. Restore backup\n";
        }
        else {
            std::cout << "ERROR (is folder /switch/4IFIR-configurator/ exists?)\n";
            curScreen = -1;
        }
        
    }
    else {
        std::cout << "Welcome to 4IFIR GPU voltage table changer 0.6\n\nSelect desired voltages:\n";
            std::cout << "Y. 4IFIR stock voltages\n";
            std::cout << "X. 4IFIR stage voltages\n";
            std::cout << "L. Restore backup\n";
    }

    int memType = 0;
    int rev = 0;

    // Main loop
    while (appletMainLoop())
    {
        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);

        if (kDown & HidNpadButton_Plus)
            break; // break in order to return to hbmenu
            
        if (curScreen == 1) {
            if (kDown & HidNpadButton_Y)
            {
                Clear();
                memType = 1;
                curScreen = 2;
                std::cout << "Select your revision: \n";
                std::cout << "X. Mariko (v2, Lite and Oled)\n";
                std::cout << "Y. Erista (Old)\n";
            }
            else if (kDown & HidNpadButton_X) {
                Clear();
                memType = 2;
                curScreen = 2;
                std::cout << "Select your revision: \n";
                std::cout << "X. Mariko (v2, Lite and Oled)\n";
                std::cout << "Y. Erista (Old)\n";
            }
            else if (kDown & HidNpadButton_L) {
                std::cout << "Restoring backup...\n";
                copyFile("/switch/4IFIR-configurator/backup.kip", "/atmosphere/kips/loader.kip");
                Clear();
                cout << "All is ok, restart your console";
            }
            
        }
        else if (curScreen == 2) {
            if (kDown & HidNpadButton_Y)
            {
                Clear();
                rev = 1;
                curScreen = 3;
                copyFile("/switch/4IFIR-configurator/backup.kip", "/switch/4IFIR-configurator/loader.kip");
            }
            else if (kDown & HidNpadButton_X) {
                Clear();
                rev = 2;
                curScreen = 3;
                copyFile("/switch/4IFIR-configurator/backup.kip", "/switch/4IFIR-configurator/loader.kip");
            }
        }
        else if (curScreen == 3) {
            switch (memType) {
            case 1:
            {
                switch (rev) {
                case 1: {
                    curScreen = 5;
                    // Erista stock
                    std::ifstream input("loader.kip", std::ios::binary);
                    if (!input.is_open()){
                        Logger("Error opening file");
                        std::cerr << "Error opening file" << std::endl;
                        return 1;
                    }
                    int trigger = find_addresses(input, 1414747459)[0] + 28;
                    input.close();
                    fstream file("loader.kip", ios::binary | ios::in | ios::out);
                    changing(file, trigger, 0);
                    input.close();
                    break;
                }
                case 2: {
                    curScreen = 5;
                    // Mariko stock
                    std::ifstream input( "loader.kip", std::ios::binary);
                    if (!input.is_open()){
                        Logger("Error opening file");
                        std::cerr << "Error opening file" << std::endl;
                        return 1;
                    }
                    int trigger = find_addresses(input, 1414747459)[0] + 44;
                    input.close();
                    fstream file("loader.kip", ios::binary | ios::in | ios::out);
                    changing(file, trigger, 0);
                    input.close();
                    break;
                }
                }
                break;
            }
            case 2: {
                switch (rev) {
                case 1: {
                    curScreen = 5;
                    // Erista ST
                    std::ifstream input( "loader.kip", std::ios::binary);
                    if (!input.is_open()){
                        Logger("Error opening file");
                        std::cerr << "Error opening file" << std::endl;
                        return 1;
                    }
                    int trigger = find_addresses(input, 1414747459)[0] + 28;
                    input.close();
                    fstream file("loader.kip", ios::binary | ios::in | ios::out);
                    changing(file, trigger, 1);
                    input.close();
                    break;
                }
                case 2: {
                    curScreen = 5;
                    // Mariko ST
                    std::ifstream input( "loader.kip", std::ios::binary);
                    if (!input.is_open()){
                        Logger("Error opening file");
                        std::cerr << "Error opening file" << std::endl;
                        return 1;
                    }
                    int trigger = find_addresses(input, 1414747459)[0] + 44;
                    input.close();
                    fstream file("loader.kip", ios::binary | ios::in | ios::out);
                    changing(file, trigger, 1);
                    input.close();
                    break;
                }
                }
                break;
            }
            }
        } else if (curScreen == 5){
            cout << "\nDone, moving loader.kip...\n";
            copyFile("loader.kip", "/atmosphere/kips/loader.kip");
            remove("loader.kip");
            Clear();
            cout << "All is ok, restart your console";
            curScreen = -2;
        }
        
        // Update the console, sending a new frame to the display
        consoleUpdate(NULL);
    }

    // Deinitialize and clean up resources used by the console (important!)
    consoleExit(NULL);
    socketExit();
    return 0;
}