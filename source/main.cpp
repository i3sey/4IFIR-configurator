#include <switch.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <vector>
#include <cstdint>

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

void changing(fstream& file, const int address, string clocks, int offset = 0) {
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
    int newVoltage;
    memcpy(&number, buffer, sizeof(number));
    cout.setf(ios::dec);

    // Write the new value
    if (offset == 0)
    {
        cout << "Enter new " << clocks << " voltage, default is " << number << ": " << endl;
        cin >> newVoltage;
    }
    else {
        newVoltage = number + offset;
    };
    file.write(reinterpret_cast<const char*>(&newVoltage), sizeof(newVoltage));
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

    std::ifstream src("/switch/4IFIRcofigurator/backup.kip", std::ios::binary);
    if (!src.good()) {
        if (copyFile("/atmosphere/kips/loader.kip", "/switch/4IFIRcofigurator/backup.kip"))
        {
            std::cout << "loader.kip has been successfully reserved!\n\n";
            std::cout << "Welcome to 4IFIR GPU voltage configurator 0.5\nSelect your memory template:\n";
            std::cout << "Y. 4IFIR base memory\n";
            std::cout << "X. 4IFIR stage memory\n";
            std::cout << "A. 4IFIR stage plus memory\n";
            std::cout << "L. Restore backup\n";
        }
        else {
            std::cout << "ERROR (is folder /switch/4IFIRcofigurator/ exists?)\n";
            curScreen = -1;
        }
        
    }
    else {
        std::cout << "Welcome to 4IFIR GPU voltage configurator 0.5\nSelect your memory template:\n";
        std::cout << "Y. 4IFIR base memory\n";
        std::cout << "X. 4IFIR stage memory\n";
        std::cout << "A. 4IFIR stage plus memory\n";
        std::cout << "L. Restore backup\n";
    }

    int memType = 0;
    int rev = 0;
    int offset = 10000;

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
                std::cout << "X. Mariko (Oled, Lite and v2)\n";
                std::cout << "Y. Erista (Old)\n";
            }
            else if (kDown & HidNpadButton_X) {
                Clear();
                memType = 2;
                curScreen = 2;
                std::cout << "Select your revision: \n";
                std::cout << "X. Mariko (Oled, Lite and v2)\n";
                std::cout << "Y. Erista (Old)\n";
            }
            else if (kDown & HidNpadButton_A) {
                Clear();
                memType = 3;
                curScreen = 2;
                std::cout << "Select your revision: \n";
                std::cout << "X. Mariko (Oled, Lite and v2)\n";
                std::cout << "Y. Erista (Old)\n";
            }
            else if (kDown & HidNpadButton_L) {
                std::cout << "Restoring backup...\n";
                copyFile("/switch/4IFIRcofigurator/backup.kip", "/atmosphere/kips/loader.kip");
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
            }
            else if (kDown & HidNpadButton_X) {
                Clear();
                rev = 2;
                curScreen = 3;
            }
        }
        else if (curScreen == 3) {
            Clear();
            cout << "Enter offset to all (L - -10000, R - +10000): \n";
            cout << "\t\t" << offset;
            if (kDown & HidNpadButton_L)
            {
                offset -= 10000;
            }
            else if (kDown & HidNpadButton_R) {
                offset += 10000;
            }
            else if (kDown & HidNpadButton_A)
            {
                Clear();
                curScreen = 4;
                copyFile("/switch/4IFIRcofigurator/backup.kip", "/switch/4IFIRcofigurator/loader.kip");
                cout << "Processing " << offset << "mv, " << rev << " rev, " << memType << " memtype.\n";

            }
        }
        else if (curScreen == 4) {
            switch (memType) {
            case 1:
            {
                switch (rev) {
                case 1: {
                    curScreen = 5;
                    // Erista stock
                    // Find adresses -> make array -> changing
                    std::ifstream input( "loader.kip", std::ios::binary);
                    if (!input.is_open()){
                        Logger("Error opening file");
                        std::cerr << "Error opening file" << std::endl;
                        return 1;
                    }
                    // searching
                    int addresses[5]; // count of Mariko voltages
                    addresses[4] = find_addresses(input, 1149425)[2]; // 998400
                    cout << "Found " << "1149425" << " address: " << hex << addresses[4] << endl;
                    addresses[3] = find_addresses(input, 1149425)[1]; // 921600
                    cout << "Found " << "1149425" << " address: " << hex << addresses[3] << endl;
                    addresses[2] = find_addresses(input, 1117534)[1]; // 844800
                    cout << "Found " << "1117534" << " address: " << hex << addresses[2] << endl;
                    addresses[1] = find_addresses(input, 1085642)[1]; //  768000
                    cout << "Found " << "1085642" << " address: " << hex << addresses[1] << endl;
                    addresses[0] = find_addresses(input, 1023751)[1]; //  691200
                    cout << "Found " << "1023751" << " address: " << hex << addresses[0] << endl;
                    input.close();
                    // changing 
                    cout << "All voltages are found, changing";
                    fstream file("loader.kip", ios::binary | ios::in | ios::out);
                    changing(file, addresses[4], "998MHz", offset);
                    changing(file, addresses[3], "921MHz", offset);
                    changing(file, addresses[2], "844MHz", offset);
                    changing(file, addresses[1], "768MHz", offset);
                    changing(file, addresses[0], "691MHz", offset);
                    file.close();
                    break;
                }
                case 2: {
                    curScreen = 5;
                    // Mariko stock
                    // Find adresses -> make array -> changing
                    std::ifstream input( "loader.kip", std::ios::binary);
                    if (!input.is_open()){
                        Logger("Error opening file");
                        std::cerr << "Error opening file" << std::endl;
                        return 1;
                    }
                    // searching
                    int addresses[10]; // count of Mariko voltages
                    addresses[9] = find_addresses(input, 1204812)[2]; // 1305600
                    cout << "Found " << "1204812" << " address: " << hex << addresses[9] << endl;
                    addresses[8] = find_addresses(input, 1204812)[1]; // 1267200
                    cout << "Found " << "1204812" << " address: " << hex << addresses[8] << endl;
                    addresses[7] = find_addresses(input, 1163644)[1]; // 1228800
                    cout << "Found " << "1163644" << " address: " << hex << addresses[7] << endl;
                    addresses[6] = find_addresses(input, 1098475)[1]; //  1152000
                    cout << "Found " << "1098475" << " address: " << hex << addresses[6] << endl;
                    addresses[5] = find_addresses(input, 986765)[1]; //  1075200
                    cout << "Found " << "986765" << " address: " << hex << addresses[5] << endl;
                    addresses[4] = find_addresses(input, 940071)[1]; //  998400
                    cout << "Found " << "940071" << " address: " << hex << addresses[4] << endl;
                    addresses[3] = find_addresses(input, 891575)[1]; //  921600
                    cout << "Found " << "891575" << " address: " << hex << addresses[3] << endl;
                    addresses[2] = find_addresses(input, 848830)[1]; //  844800
                    cout << "Found " << "848830" << " address: " << hex << addresses[2] << endl;
                    addresses[1] = find_addresses(input, 824214)[1]; //  768000
                    cout << "Found " << "824214" << " address: " << hex << addresses[1] << endl;
                    addresses[0] = find_addresses(input, 801688)[1]; //  691200
                    cout << "Found " << "801688" << " address: " << hex << addresses[0] << endl << endl;
                    input.close();
                    // changing 
                    cout << "All voltages are found, changing";
                    fstream file("loader.kip", ios::binary | ios::in | ios::out);
                    changing(file, addresses[9], "1305MHz", offset);
                    changing(file, addresses[8], "1267MHz", offset);
                    changing(file, addresses[7], "1228MHz", offset);
                    changing(file, addresses[6], "1152MHz", offset);
                    changing(file, addresses[5], "1075MHz", offset);
                    changing(file, addresses[4], "998MHz", offset);
                    changing(file, addresses[3], "921MHz", offset);
                    changing(file, addresses[2], "844MHz", offset);
                    changing(file, addresses[1], "768MHz", offset);
                    changing(file, addresses[0], "691MHz", offset);
                    file.close();
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
                    // Find adresses -> make array -> changing
                    std::ifstream input( "loader.kip", std::ios::binary);
                    if (!input.is_open()){
                        Logger("Error opening file");
                        std::cerr << "Error opening file" << std::endl;
                        return 1;
                    }
                    // searching
                    int addresses[6]; // count of Erista ST voltages
                    addresses[5] = find_addresses(input, 1159425)[1]; // 998400
                    cout << "Found " << "1159425" << " address: " << hex << addresses[5] << endl;
                    addresses[4] = find_addresses(input, 1159425)[0]; //  921600
                    cout << "Found " << "1159425" << " address: " << hex << addresses[4] << endl;
                    addresses[3] = find_addresses(input, 1137534)[1]; //  844800
                    cout << "Found " << "1137534" << " address: " << hex << addresses[3] << endl;
                    addresses[2] = find_addresses(input, 1095642)[1]; //  768000
                    cout << "Found " << "1095642" << " address: " << hex << addresses[2] << endl;
                    addresses[1] = find_addresses(input, 1069751)[1]; //  691200
                    cout << "Found " << "1069751" << " address: " << hex << addresses[1] << endl;
                    addresses[0] = find_addresses(input, 1023751)[1]; //  614400
                    cout << "Found " << "1023751" << " address: " << hex << addresses[0] << endl << endl;
                    input.close();
                    // changing 
                    cout << "All voltages are found, changing";
                    fstream file("loader.kip", ios::binary | ios::in | ios::out);
                    changing(file, addresses[5], "998MHz", offset);
                    changing(file, addresses[4], "921MHz", offset);
                    changing(file, addresses[3], "844MHz", offset);
                    changing(file, addresses[2], "768MHz", offset);
                    changing(file, addresses[1], "691MHz", offset);
                    changing(file, addresses[0], "614MHz", offset);
                    file.close();
                    break;
                }
                case 2: {
                    curScreen = 5;
                    // Mariko ST
                    // Find adresses -> make array -> changing
                    std::ifstream input( "loader.kip", std::ios::binary);
                    if (!input.is_open()){
                        Logger("Error opening file");
                        std::cerr << "Error opening file" << std::endl;
                        return 1;
                    }
                    // searching
                    int addresses[8]; // count of Mariko ST voltages
                    addresses[7] = find_addresses(input, 1163644)[1]; // 1305600
                    cout << "Found " << "1163644" << " address: " << hex << addresses[7] << endl;
                    addresses[6] = find_addresses(input, 1131060)[0]; // 1267200
                    cout << "Found " << "1131060" << " address: " << hex << addresses[6] << endl;
                    addresses[5] = find_addresses(input, 1098475)[1]; // 1228800
                    cout << "Found " << "1098475" << " address: " << hex << addresses[5] << endl;
                    addresses[4] = find_addresses(input, 986765)[1]; //  1152000
                    cout << "Found " << "986765" << " address: " << hex << addresses[4] << endl;
                    addresses[3] = find_addresses(input, 940071)[1]; //  1075200
                    cout << "Found " << "940071" << " address: " << hex << addresses[3] << endl;
                    addresses[2] = find_addresses(input, 891575)[1]; //  998400
                    cout << "Found " << "891575" << " address: " << hex << addresses[2] << endl;
                    addresses[1] = find_addresses(input, 848830)[1]; //  921600
                    cout << "Found " << "848830" << " address: " << hex << addresses[1] << endl;
                    addresses[0] = find_addresses(input, 801688)[1]; //  844800
                    cout << "Found " << "801688" << " address: " << hex << addresses[0] << endl << endl;
                    input.close();
                    // changing 
                    cout << "All voltages are found, changing";
                    fstream file("loader.kip", ios::binary | ios::in | ios::out);
                    changing(file, addresses[7], "1305MHz", offset);
                    changing(file, addresses[6], "1267MHz", offset);
                    changing(file, addresses[5], "1228MHz", offset);
                    changing(file, addresses[4], "1152MHz", offset);
                    changing(file, addresses[3], "1075MHz", offset);
                    changing(file, addresses[2], "998MHz", offset);
                    changing(file, addresses[1], "921MHz", offset);
                    changing(file, addresses[0], "844MHz", offset);
                    file.close();
                    break;
                }
                }
                break;
            }
            case 3: {
                switch (rev) {
                case 1: {
                    curScreen = 5;
                    // Erista ST+
                    // Find adresses -> make array -> changing
                    std::ifstream input( "loader.kip", std::ios::binary);
                    if (!input.is_open()){
                        Logger("Error opening file");
                        std::cerr << "Error opening file" << std::endl;
                        return 1;
                    }
                    // searching
                    int addresses[6]; // count of Erista ST voltages
                    addresses[5] = find_addresses(input, 1159425)[1]; // 998400
                    cout << "Found " << "1159425" << " address: " << hex << addresses[5] << endl;
                    addresses[4] = find_addresses(input, 1159425)[0]; //  921600
                    cout << "Found " << "1159425" << " address: " << hex << addresses[4] << endl;
                    addresses[3] = find_addresses(input, 1137534)[1]; //  844800
                    cout << "Found " << "1137534" << " address: " << hex << addresses[3] << endl;
                    addresses[2] = find_addresses(input, 1095642)[1]; //  768000
                    cout << "Found " << "1095642" << " address: " << hex << addresses[2] << endl;
                    addresses[1] = find_addresses(input, 1069751)[1]; //  691200
                    cout << "Found " << "1069751" << " address: " << hex << addresses[1] << endl;
                    addresses[0] = find_addresses(input, 1023751)[1]; //  614400
                    cout << "Found " << "1023751" << " address: " << hex << addresses[0] << endl << endl;
                    input.close();
                    // changing 
                    cout << "All voltages are found, changing";
                    fstream file("loader.kip", ios::binary | ios::in | ios::out);
                    changing(file, addresses[5], "998MHz", offset);
                    changing(file, addresses[4], "921MHz", offset);
                    changing(file, addresses[3], "844MHz", offset);
                    changing(file, addresses[2], "768MHz", offset);
                    changing(file, addresses[1], "691MHz", offset);
                    changing(file, addresses[0], "614MHz", offset);
                    file.close();
                    break;
                }
                case 2: {
                    curScreen = 5;
                    // Mariko ST+
                    // Find adresses -> make array -> changing
                    std::ifstream input( "loader.kip", std::ios::binary);
                    if (!input.is_open()){
                        Logger("Error opening file");
                        std::cerr << "Error opening file" << std::endl;
                        return 1;
                    }
                    // searching
                    int addresses[8]; // count of Mariko ST voltages
                    addresses[7] = find_addresses(input, 1163644)[1]; // 1305600
                    cout << "Found " << "1163644" << " address: " << hex << addresses[7] << endl;
                    addresses[6] = find_addresses(input, 1131060)[0]; // 1267200
                    cout << "Found " << "1131060" << " address: " << hex << addresses[6] << endl;
                    addresses[5] = find_addresses(input, 1098475)[1]; // 1228800
                    cout << "Found " << "1098475" << " address: " << hex << addresses[5] << endl;
                    addresses[4] = find_addresses(input, 986765)[1]; //  1152000
                    cout << "Found " << "986765" << " address: " << hex << addresses[4] << endl;
                    addresses[3] = find_addresses(input, 940071)[1]; //  1075200
                    cout << "Found " << "940071" << " address: " << hex << addresses[3] << endl;
                    addresses[2] = find_addresses(input, 891575)[1]; //  998400
                    cout << "Found " << "891575" << " address: " << hex << addresses[2] << endl;
                    addresses[1] = find_addresses(input, 848830)[1]; //  921600
                    cout << "Found " << "848830" << " address: " << hex << addresses[1] << endl;
                    addresses[0] = find_addresses(input, 801688)[1]; //  844800
                    cout << "Found " << "801688" << " address: " << hex << addresses[0] << endl << endl;
                    input.close();
                    // changing 
                    cout << "All voltages are found, changing";
                    fstream file("loader.kip", ios::binary | ios::in | ios::out);
                    changing(file, addresses[7], "1305MHz", offset);
                    changing(file, addresses[6], "1267MHz", offset);
                    changing(file, addresses[5], "1228MHz", offset);
                    changing(file, addresses[4], "1152MHz", offset);
                    changing(file, addresses[3], "1075MHz", offset);
                    changing(file, addresses[2], "998MHz", offset);
                    changing(file, addresses[1], "921MHz", offset);
                    changing(file, addresses[0], "844MHz", offset);
                    file.close();
                    break;
                }
                }
                break;
            }
            }
        } else if (curScreen == 5){
            cout << "\nDone, moving loader.kip...\n";
            copyFile("loader.kip", "/atmosphere/kips/loader.kip");
            // Clear();
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