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
		"C. Import a file into this volume.\n"
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
								else if (c == 'b' || c == 'B') {
									std::cout << "B\n";
									std::cout << "Exporting this file.\n";

									int nameSize = convertLittleEndianToInt(buffer, 26, 1);
									string filename_;
									for (int j = 27; j < 27 + nameSize; j++) {
										filename_ += buffer[j];
									}
									string extension;
									for (int j = 51; j < 54; j++) {
										extension += buffer[j];
									}

									int startCluster = convertLittleEndianToInt(buffer, 58, 2);
									int fileSize = convertLittleEndianToInt(buffer, 60, 4);

									std::cout << "(Press Enter if you want to use defaut filepath (ExportedFile/) )\n";
									std::cout << "Enter the file path: ";
									string filepath;
									getline(cin, filepath);
									std::cout << "(Press Enter if you want to use this file name)\n";
									std::cout << "Enter the exported file name without extension: ";
									string filename;
									getline(cin, filename);

									if (filepath.empty()) {
										filepath = "ExportedFile/";
									}
									if (filename.empty()) {
										filename = filename_;
									}

									filename = filename + '.' + extension;

									exportFile(volume, filepath + filename, startCluster, fileSize, sb, sc, sv);

									std::cout << "Export file completed.\n";
									std::cout << fwelcome;
								}
							}	
						}
					}
				}
			}
		}
		else if (c == 'c' || c == 'C') {
			std::cout << "C\n";
			std::cout << "Importing a file.\n";
			std::cout << "(Press Enter if you want to use defaut filepath (FileToInput/) )\n";
			std::cout << "Enter the file path: ";
			string filepath;
			getline(cin, filepath);
			std::cout << "Enter the file name with extension: ";
			string filename;
			getline(cin, filename);
			if (filepath.empty()) {
				filepath = "FileToInput/";
			}

			importFile(volume, filepath + filename, sb, sc, sv);
			std::cout << "Import file completed.\n";
			std::cout << welcome;
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

void importFile(std::fstream& volume, string filepath, int sb_, int sc_, int sv_) {
	fstream inputfile(filepath, std::ios::in | std::ios::binary);

	if (!inputfile.is_open()) {
		std::cout << "Fail to open inputing file.\n";
		return;
	}

	std::streampos begin = inputfile.tellg();

	inputfile.seekg(0, std::ios::end);

	std::streampos end = inputfile.tellg();

	std::size_t fileSize = static_cast<std::size_t>(end - begin);

	std::cout << "File size: " << fileSize << endl;

	inputfile.seekg(0, std::ios::beg);

	int si = 1;
	int nb = 2;
	int sr = 32;
	int sv = sv_;
	int sb = sb_;
	int sc = sc_;
	int sd = sv - si - nb * sb - sr;

	int numOfCluster = (int(fileSize) + (sc * BLOCKSIZE) - 1) / (sc * BLOCKSIZE);

	//Find numOfCluster empty cluster in table
	//Element k manage cluster k

	vector<uint16_t> listOfEmptyCluster;

	volume.seekg(si * BLOCKSIZE);
	char* tableBuffer = new char[sb * BLOCKSIZE];
	volume.read(tableBuffer, sb * BLOCKSIZE);

	for (int i = 0; i < sb * BLOCKSIZE; i += 2) {
		unsigned char byte1 = static_cast<unsigned char>(tableBuffer[i]);
		unsigned char byte2 = static_cast<unsigned char>(tableBuffer[i + 1]);

		if ((byte1 != 0x00) || (byte2 != 0x00)) {
			continue;
		}
		else {
			listOfEmptyCluster.push_back(uint16_t(i / 2 + 2));
		}

		if (listOfEmptyCluster.size() == numOfCluster) {
			break;
		}
	}

	cout << "List of cluster: ";
	for (uint16_t byte : listOfEmptyCluster) {
		cout << byte << " ";
	}
	cout << endl;

	delete[] tableBuffer;

	char eleBuffer[ELEMENTSIZE];
	eleBuffer[0] = char(0xFF);
	eleBuffer[1] = char(0xFF);

	//Write down to 2 table
	writeElementIn2Table(volume, eleBuffer, listOfEmptyCluster[listOfEmptyCluster.size() - 1], si, sb);
	for (int i = int(listOfEmptyCluster.size()) - 1; i > 0; i--) {
		memcpy_s(eleBuffer, ELEMENTSIZE, &listOfEmptyCluster[i], sizeof(listOfEmptyCluster[i]));
		writeElementIn2Table(volume, eleBuffer, listOfEmptyCluster[i - 1], si, sb);
	}
	
	//Create entry for this file
	char entry[64];
	char filename[24];
	char extentsion[3];
	int filenameSize = 0;

	for (int i = 0; i < 3; i++) {
		extentsion[i] = filepath[filepath.size() - 3 + i];
	}

	size_t pos = filepath.find_last_of('/');

	if (pos != std::string::npos) {
		int j = 0;
		while (filenameSize <= 24 && filepath[pos + 1 + j] != '.') {
			filename[j] = filepath[pos + 1 + j];
			j++;
			filenameSize++;
		}
	}
	else {
		int j = 0;
		while (filenameSize <= 24 && filepath[j] != '.') {
			filename[j] = filepath[j];
			j++;
			filenameSize++;
		}
	}

	int startCluster = int(listOfEmptyCluster[0]);

	entryCreate(entry, filename, filenameSize, extentsion, startCluster, int(fileSize));

	//Find empty spot to write entry
	int firstEntryPos = (si + nb * sb) * BLOCKSIZE;
	for (int i = 0; i < sr * BLOCKSIZE / ENTRYSIZE; i++) {	//There are 32 * 512 / 64 = 256 entries in total
		volume.seekg(firstEntryPos + i * ENTRYSIZE);
		char byte[1];
		volume.read(byte, 1);
		if (static_cast<unsigned char>(byte[0]) == 0x00) {
			writeEntry(volume, entry, i, firstEntryPos);
			break;
		}
	}
	
	//Write data down to correct cluster
	inputfile.seekg(0, std::ios::beg);

	char* buffer = new char[sc * BLOCKSIZE];
	for (int i = 0; i < numOfCluster - 1; i++) {
		inputfile.read(buffer, sc * BLOCKSIZE);
		volume.seekp((si + nb * sb + sr) * BLOCKSIZE + (listOfEmptyCluster[i] - 2) * sc * BLOCKSIZE);
		volume.write(buffer, sc * BLOCKSIZE);
		memset(buffer, '\0', sc * BLOCKSIZE);
	}
	memset(buffer, '\0', sc * BLOCKSIZE);
	inputfile.read(buffer, fileSize % (sc * BLOCKSIZE));
	volume.seekp((si + nb * sb + sr) * BLOCKSIZE + (listOfEmptyCluster[numOfCluster - 1] - 2) * sc * BLOCKSIZE);
	volume.write(buffer, fileSize % (sc* BLOCKSIZE));

	delete[] buffer;

	inputfile.close();
}

void writeElementIn2Table(std::fstream& volume, char data[ELEMENTSIZE], int elementNumber, int si, int sb) {
	volume.seekp(si * BLOCKSIZE + (elementNumber - 2) * ELEMENTSIZE);
	volume.write(data, ELEMENTSIZE);
	volume.seekp((si + sb) * BLOCKSIZE + (elementNumber - 2) * ELEMENTSIZE);
	volume.write(data, ELEMENTSIZE);
}

void exportFile(std::fstream& volume, string filepath, int startCluster, int fileSize, int sb_, int sc_, int sv_) {
	fstream outputFile(filepath, std::ios::out | std::ios::binary | std::ios::trunc);

	int si = 1;
	int nb = 2;
	int sr = 32;
	int sv = sv_;
	int sb = sb_;
	int sc = sc_;
	int sd = sv - si - nb * sb - sr;

	if (!outputFile.is_open()) {
		std::cout << "Cannot open exported file.\n";
		return;
	}

	vector<uint16_t> listOfFileCluster;
	
	//seek element manage file clusters
	uint16_t currentCluster = uint16_t(startCluster);
	uint16_t nextCluster = 0;
	do {
		listOfFileCluster.push_back(currentCluster);
		volume.seekg(si * BLOCKSIZE + (currentCluster - 2) * ELEMENTSIZE);

		char nextClusterBuffer[ELEMENTSIZE];
		volume.read(nextClusterBuffer, ELEMENTSIZE);

		//Convert little edian to uint16_t
		nextCluster = static_cast<uint16_t>(static_cast<uint8_t>(nextClusterBuffer[0])) |
			(static_cast<uint16_t>(static_cast<uint8_t>(nextClusterBuffer[1])) << 8);

		currentCluster = nextCluster;
	} while (nextCluster != 0xFFFF);

	cout << "List of cluster: ";
	for (uint16_t byte : listOfFileCluster) {
		cout << byte << " ";
	}
	cout << endl;

	//Write output file.
	char* buffer = new char[sc * BLOCKSIZE];
	for (int i = 0; i < listOfFileCluster.size() - 1; i++) {
		volume.seekg((si + nb * sb + sr) * BLOCKSIZE + (listOfFileCluster[i] - 2) * sc * BLOCKSIZE);
		volume.read(buffer, sc * BLOCKSIZE);
		outputFile.write(buffer, sc * BLOCKSIZE);
		memset(buffer, '\0', sc * BLOCKSIZE);
	}
	memset(buffer, '\0', sc * BLOCKSIZE);
	volume.seekg((si + nb * sb + sr) * BLOCKSIZE + (listOfFileCluster[listOfFileCluster.size() - 1] - 2) * sc * BLOCKSIZE);
	volume.read(buffer, fileSize % (sc * BLOCKSIZE));
	outputFile.write(buffer, fileSize % (sc * BLOCKSIZE));

	delete[] buffer;

	outputFile.close();
}