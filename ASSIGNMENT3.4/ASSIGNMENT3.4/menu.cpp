#include "menu.h"

void menu() {
	string welcome =
		"Welcome to \"kkk\" volume file management program.\n"
		"What is your action?\n"
		"1. Create/Full format a \"kkk\" volume file\n"
		"2. Quick format an existing \"kkk\" volume file\n"
		"3. Open a \"kkk\" volume file\n"
		"4. Exit the program\n"
		"Entering your choice: ";
	std::cout << welcome;
	bool isContinue = 1;

	while (isContinue) {
		char c = _getch();
		if (c == '4') {
			std::cout << "4\n";
			isContinue = 0;
		}
		else if (c == '1') {
			std::cout << "1\n";
			std::cout << "Creating a new \"kkk\" volume file.\n";
			std::cout << "(Press Enter if you want to use defaut filepath)\n";
			std::cout << "Enter the file path: ";
			string filepath;
			getline(cin, filepath);
			std::cout << "Enter the file name: ";
			string filename;
			getline(cin, filename);
			filename += ".dat";

			std::cout << "Enter volume size (from 8MB to 1GiB (1073741824B)) in bytes: ";
			int filesize;
			cin >> filesize;

			createVolume(filepath + filename, filesize);

			std::cout << welcome;
		}
		else if (c == '2') {
			std::cout << "2\n";
			std::cout << "Quick format a \"kkk\" volume file.\n";
			std::cout << "(Press Enter if you want to use defaut filepath)\n";
			std::cout << "Enter the file path: ";
			string filepath;
			getline(cin, filepath);
			std::cout << "Enter the file name: ";
			string filename;
			getline(cin, filename);
			filename += ".dat";

			quickFormat(filepath + filename);

			std::cout << welcome;
		}
		else if (c == '3') {
			std::cout << "3\n";
			std::cout << "Opening \"kkk\" volume file.\n";
			std::cout << "(Press Enter if you want to use defaut filepath)\n";
			std::cout << "Enter the file path: ";
			string filepath;
			getline(cin, filepath);
			std::cout << "Enter the file name: ";
			string filename;
			getline(cin, filename);
			filename += ".dat";

			openVolume(filepath + filename);

			std::cout << welcome;
		}
		
	}
}