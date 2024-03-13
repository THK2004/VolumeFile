#include "menu.h"

int main() {
	menu();
	/*
	pair<string, string> hashedPasswordAndSalt;
	hashedPasswordAndSalt = argon2PasswordHashing32("987654321", 16);
	
	for (unsigned char byte : hashedPasswordAndSalt.first) {
		printf("%02x", byte);
	}
	cout << endl << hashedPasswordAndSalt.second << endl;
	*/
	/*
	char entry[64];
	char filename[24] = "Hello";
	char extentsion[3];
	for (int i = 0; i < 3; i++) {
		extentsion[i] = 'K';
	}
	entryCreate(entry, filename, 5, extentsion, 10, 10000);

	cout << "Entry: ";
	for (unsigned char byte : entry) {
		printf("%02x", byte);
	}
	cout << endl;
	*/

	std::cout << endl;
	std::cout << "Program exited.\n";
	std::system("pause");
	return 0;
}