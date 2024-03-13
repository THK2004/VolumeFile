#include "open.h"

void openVolume(string filepath) {
	fstream volume(filepath, std::ios::out | std::ios::in | std::ios::binary);

	if (!volume.is_open()) {
		std::cout << "Fail to open volume file.\n";
		return;
	}

	char buffer[BLOCKSIZE];
	readBlock(volume, 0, buffer);

	if (buffer[0] != 'K' && buffer[1] != 'K' && buffer[2] != 'K') {
		volume.close();
		std::cout << "Not \"kkk\" volume file.";
		return;
	}

	bool passwordCheck = volumePasswordVerify(volume);
	if (passwordCheck == 0) {
		volume.close();
		std::cout << "Open volume cancelled.\n";
		return;
	}

	int si = 1;
	int nb = 2;
	int sr = 32;
	int sv = 0;
	int sb = 0;
	int sc = 0;
	int sd = 0;

	readVolumeInfo(buffer, sb, sc, sv);
	sd = sv - si - nb * sb - sr;
	cout << sb << " " << sc << " " << sv << " " << sd << endl;


	string welcome =
		"Successfully open this \"kkk\" volume file.\n"
		"What is your action?\n"
		"A. Change password\n"
		"B. List all files in this volume.\n"
		"E. Exit volume\n"
		"Enter your choice: ";
	std::cout << welcome;

	bool isContinue = 1;
	while (isContinue) {
		char c = _getch();
		if (c == 'E' || c == 'e') {
			std::cout << "E\n";
			std::cout << "Volume exited.\n";
			isContinue = 0;
		}
		else if (c == 'a' || c == 'A') {
			std::cout << "A\n";
			std::cout << "Setting a new password for the volume.\n";

			string password;
			while (true) {
				std::cout << "Enter your password from 8 to 16 characters: ";
				getline(cin, password);
				if (password.size() < 8 || password.size() > 16) {
					std::cout << "Wrong password lenght.\n";
				}
				else {
					break;
				}
			}

			pair<string, string> hashedPassAndSalt;
			hashedPassAndSalt = argon2PasswordHashing32(password, 16);

			volume.seekp(3);
			volume.write(hashedPassAndSalt.first.c_str(), 32);
			volume.write(hashedPassAndSalt.second.c_str(), 16);

			std::cout << "Change password successfully\n";
			std::cout << welcome;
		}
		else if (c == 'b' || c == 'B') {
			
			/*
			char entry[64];
			char filename[24] = "HelloHowAreYou";
			char extentsion[3];
			for (int i = 0; i < 3; i++) {
				extentsion[i] = 'K';
			}
			entryCreate(entry, filename, int(strlen(filename)), extentsion, 12, 2000);
			writeEntry(volume, entry, 7, (si + nb * sb) * BLOCKSIZE);
			*/
			
			std::cout << "B\n";
			std::cout << "Here is the list of all files:\n";
			vector<int> fileNumberList;
			int firstEntryPos = (si + nb * sb) * BLOCKSIZE;
			for (int i = 0; i < sr * BLOCKSIZE / ENTRYSIZE; i++) {	//There are 32 * 512 / 64 = 256 entries in total
				volume.seekg(firstEntryPos + i * ENTRYSIZE);
				char byte[1];
				volume.read(byte, 1);
				if (static_cast<unsigned char>(byte[0]) != 0x00 && static_cast<unsigned char>(byte[0]) != 0xE5) {
					fileNumberList.push_back(i);
				}
			}
			if (fileNumberList.empty()) {
				std::cout << "There is no file in this volume.\n";
				std::cout << welcome;
			}
			else {
				for (int i = 0; i < fileNumberList.size(); i++) {
					char buffer[ENTRYSIZE];
					readEntry(volume, buffer, fileNumberList[i], firstEntryPos);

					char hashedPassword[16];
					for (int i = 0; i < 16; i++) {
						hashedPassword[i] = buffer[i + 2];
					}

					int nameSize = convertLittleEndianToInt(buffer, 26, 1);

					string filename;
					for (int j = 27; j < 27 + nameSize; j++) {
						filename += buffer[j];
					}
					string extension;
					for (int j = 51; j < 54; j++) {
						extension += buffer[j];
					}

					unsigned short packedTime = 
						static_cast<unsigned char>(buffer[54]) |
						(static_cast<unsigned char>(buffer[55]) << 8);

					unsigned short packedDate = 
						static_cast<unsigned char>(buffer[56]) |
						(static_cast<unsigned char>(buffer[57]) << 8);

					unsigned int unpackedHour = (packedTime >> 11) & 0x1F;
					unsigned int unpackedMinute = (packedTime >> 5) & 0x3F;
					unsigned int unpackedSecond = (packedTime & 0x1F) * 2;

					unsigned int unpackedYear = ((packedDate >> 9) & 0x7F) + 1980;
					unsigned int unpackedMonth = (packedDate >> 5) & 0x0F;
					unsigned int unpackedDay = packedDate & 0x1F;

					int startCluster = convertLittleEndianToInt(buffer, 58, 2);
					int fileSize = convertLittleEndianToInt(buffer, 60, 4);

					std::cout << i + 1 << ". Entry No." << fileNumberList[i] << "/";
					std::cout << filename + "." + extension << "/";
					if (strlen(hashedPassword) == 0) {
						std::cout << "No password/";
					}
					else {
						std::cout << "Has password/";
					}
					std::cout << unpackedSecond << "-" << unpackedMinute << "-" << unpackedHour << "/";
					std::cout << unpackedDay << "-" << unpackedMonth << "-" << unpackedYear << "/";
					std::cout << startCluster << "/" << fileSize << "Bytes\n";
				}

				while (true) {
					std::cout << "Which file do you want to access to (0: to back): ";
					int n = 0;
					cin >> n;
					cin.ignore();
					if (n > fileNumberList.size() || n < 0) {
						std::cout << "Wrong input.\n";
					}
					else if (n == 0) {
						std::cout << welcome;
						break;
					}
					else {
						char buffer[ENTRYSIZE];
						readEntry(volume, buffer, fileNumberList[n - 1], firstEntryPos);

						bool isVerified = filePasswordVerify(buffer);
						if (isVerified == 0) {
							std::cout << "Open file cancelled.\n";
							continue;
						}
						else {
							string fwelcome = 
								"Open file successful.\n"
								"What is your action?\n"
								"A. Change password.\n"
								"B. Export this file.\n"
								"E. Exit this file.\n"
								"Enter your action: ";
							std::cout << fwelcome;
							while (true) {
								char c = _getch();
								if (c == 'e' || c == 'E') {
									std::cout << "E\n";
									std::cout << "File exited\n";
									break;
								}
								else if (c == 'a' || c == 'A') {
									std::cout << "A\n";
									std::cout << "Setting a new password for this file.\n";

									string password;
									while (true) {
										std::cout << "Enter your password from 8 to 16 characters: ";
										getline(cin, password);
										if (password.size() < 8 || password.size() > 16) {
											std::cout << "Wrong password lenght.\n";
										}
										else {
											break;
										}
									}

									pair<string, string> hashedPassAndSalt;
									hashedPassAndSalt = argon2PasswordHashing16(password, 8);

									volume.seekp(firstEntryPos + fileNumberList[n - 1] * ENTRYSIZE + 2);
									volume.write(hashedPassAndSalt.first.c_str(), 16);
									volume.write(hashedPassAndSalt.second.c_str(), 8);

									std::cout << "Change password successfully\n";
									std::cout << fwelcome;
								}
							}	
						}
					}
				}
			}
		}
	}

	volume.close();
}

void entryCreate(char entry[ENTRYSIZE], char fileName[24], int filenameSize, char extentsion[3], int startingCluster, int fileSize) {
	//Data prepare
	pair<string, string> hashedPasswordAndSalt = passwordCreate(16, 8);

	for (int i = filenameSize; i < 24; i++) {
		fileName[i] = char(0xFF);
	}

	uint8_t fnamesize = uint8_t(filenameSize);
	uint16_t scluster = uint16_t(startingCluster);
	unsigned int fsize = unsigned int(fileSize);
	
	// Get the current time
	auto currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::tm tmTime;

	// Use localtime_s to safely convert time to a struct tm
	if (localtime_s(&tmTime, &currentTime) != 0) {
		std::cerr << "Error getting local time." << std::endl;
		return;
	}

	// Extract hour, minute, and second values
	unsigned int hour = tmTime.tm_hour;
	unsigned int minute = tmTime.tm_min;
	unsigned int second = tmTime.tm_sec;

	// Pack values into the 2-byte variable
	unsigned short packedTime = 0;
	packedTime |= (hour & 0x1F) << 11;
	packedTime |= (minute & 0x3F) << 5;
	packedTime |= (second / 2) & 0x1F;

	// Extract year, month, and day values
	unsigned int year = tmTime.tm_year + 1900;
	unsigned int month = tmTime.tm_mon + 1;
	unsigned int day = tmTime.tm_mday;

	// Pack values into the 2-byte variable
	unsigned short packedDate = 0;
	packedDate |= ((year - 1980) & 0x7F) << 9;
	packedDate |= (month & 0x0F) << 5;
	packedDate |= (day & 0x1F);

	// Unpack values from the 2-byte variable
	/*
	unsigned int unpackedHour = (packedTime >> 11) & 0x1F;
	unsigned int unpackedMinute = (packedTime >> 5) & 0x3F;
	unsigned int unpackedSecond = (packedTime & 0x1F) * 2;

	// Unpack values from the 2-byte variable
	unsigned int unpackedYear = ((packedDate >> 9) & 0x7F) + 1980;
	unsigned int unpackedMonth = (packedDate >> 5) & 0x0F;
	unsigned int unpackedDay = packedDate & 0x1F;
	*/

	//Writing entry
	entry[0] = 0x0F;
	entry[1] = char(0xFF);

	size_t offset = 2;

	memcpy_s(entry + offset, ENTRYSIZE - offset, hashedPasswordAndSalt.first.c_str(), hashedPasswordAndSalt.first.size());
	offset += hashedPasswordAndSalt.first.size();

	memcpy_s(entry + offset, ENTRYSIZE - offset, hashedPasswordAndSalt.second.c_str(), hashedPasswordAndSalt.second.size());
	offset += hashedPasswordAndSalt.second.size();
	
	memcpy_s(entry + offset, ENTRYSIZE - offset, &fnamesize, sizeof(fnamesize));
	offset += sizeof(fnamesize);

	memcpy_s(entry + offset, ENTRYSIZE - offset, fileName, 24);
	offset += 24;

	memcpy_s(entry + offset, ENTRYSIZE - offset, extentsion, 3);
	offset += 3;

	memcpy_s(entry + offset, ENTRYSIZE - offset, &packedTime, sizeof(packedTime));
	offset += sizeof(packedTime);

	memcpy_s(entry + offset, ENTRYSIZE - offset, &packedDate, sizeof(packedDate));
	offset += sizeof(packedDate);

	memcpy_s(entry + offset, ENTRYSIZE - offset, &scluster, sizeof(scluster));
	offset += sizeof(scluster);

	memcpy_s(entry + offset, ENTRYSIZE - offset, &fsize, sizeof(fsize));
	offset += sizeof(fsize);
}

void writeEntry(std::fstream& volume, char entry[ENTRYSIZE], int entryNumber, int firstEntryPos) {
	volume.seekp(firstEntryPos + entryNumber * ENTRYSIZE);
	volume.write(entry, ENTRYSIZE);
}

void readEntry(std::fstream& volume, char buffer[ENTRYSIZE], int entryNumber, int firstEntryPos) {
	volume.seekg(firstEntryPos + entryNumber * ENTRYSIZE);
	volume.read(buffer, ENTRYSIZE);
}

bool filePasswordVerify(char entry[ENTRYSIZE]) {
	char hashedPassword[16];
	for (int i = 0; i < 16; i++) {
		hashedPassword[i] = entry[i + 2];
	}
	
	if (strlen(hashedPassword) == 0) {
		return 1;
	}
	else {
		string salt;
		for (int i = 0; i < 8; i++) {
			salt += entry[18 + i];
		}
		std::cout << "This file has password.\n";
		while (true) {
			std::cout << "Enter the password: ";
			string password;
			getline(cin, password);

			const int ops_limit = crypto_pwhash_argon2i_OPSLIMIT_INTERACTIVE;
			const int mem_limit = crypto_pwhash_argon2i_MEMLIMIT_INTERACTIVE;
			unsigned char hash[16];

			crypto_pwhash_argon2i(
				hash, sizeof(hash), password.c_str(), password.length(),
				reinterpret_cast<const unsigned char*>(salt.c_str()),
				ops_limit, mem_limit, crypto_pwhash_ALG_ARGON2I13);

			if (memcmp(hashedPassword, reinterpret_cast<char*>(hash), 16) == 0) {
				std::cout << "Correct password.\n";
				return 1;
			}
			else {
				std::cout << "Wrong password\n";
				std::cout << "Try again? (1/yes, else/no): ";
				char c = _getch();
				if (c == '1') {
					std::cout << "1\n";
				}
				else {
					std::cout << c << endl;
					return 0;
				}
			}
		}
	}
}