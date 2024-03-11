#include "Create_Format.h"

void createVolume(string filepath, int volumeSize) {
	int si = 1;
	int nb = 2;
	int sr = 32;
	int sv = volumeSize / BLOCKSIZE;
	int sb = 0;
	int sc = 0;
	int sd = 0;
	
	//Calculate Sb, Sd, Sc available for volume size and let user decide Sc
	determineSbScSd(sb, sd, sc, volumeSize);
	//cout << sb << " " << sc << " " << sd / sc << endl;

	//Create file
	fstream volume(filepath, std::ios::out | std::ios::binary | std::ios::trunc);

	if (!volume.is_open()) {
		std::cout << "Fail to create the file." << std::endl;
		exit(1);
	}

	char buffer[BLOCKSIZE];
	std::memset(buffer, 0, BLOCKSIZE);

	//Fill file with sv blocks
	for (int i = 0; i < sv; i++) {
		volume.write(buffer, BLOCKSIZE);
	}

	//Write Volume info
	writeVolumeInfo(buffer, sb, sc, sv);
	writeBlock(volume, 0, buffer);

	//Write data entry
	std::memset(buffer, 0, BLOCKSIZE);
	for (int i = 0; i < sr; i++) {
		writeBlock(volume, si + nb * sb, buffer);
	}

	//Check cluster and write to table
	char* tableBuffer = new char[BLOCKSIZE * sb];
	std::memset(tableBuffer, 0, BLOCKSIZE * sb);
	size_t tableOffset = 0;
	uint16_t free = 0x0000;
	uint16_t bad = 0xFFF7;

	char* data = new char[BLOCKSIZE * sc];
	generateRandomData(data, BLOCKSIZE * sc);
	for (int i = 2; i <= (sd + sc - 1) / sc; i++) {	//Loop from cluster 2 to last cluster
		writeCluster(volume, i, data, si, nb, sb, sr, sc);
		char* buffer = new char[BLOCKSIZE * sc];
		readCluster(volume, i, buffer, si, nb, sb, sr, sc);
		int k = strcmp(buffer, data);

		if (k == 0) {
			memcpy_s(tableBuffer + tableOffset, BLOCKSIZE * sb - tableOffset, &free, sizeof(free));
			tableOffset += sizeof(free);
		}
		else {
			memcpy_s(tableBuffer + tableOffset, BLOCKSIZE * sb - tableOffset, &bad, sizeof(bad));
			tableOffset += sizeof(bad);
		}
		delete[] buffer;
	}

	volume.seekp(si * BLOCKSIZE);
	volume.write(tableBuffer, BLOCKSIZE * sb);

	volume.seekp((si + sb) * BLOCKSIZE);
	volume.write(tableBuffer, BLOCKSIZE * sb);

	//Fill clusters with 0 again
	std::memset(data, 0, BLOCKSIZE * sc);
	for (int i = 2; i <= (sd + sc - 1) / sc; i++) {
		writeCluster(volume, i, data, si, nb, sb, sr, sc);
	}

	delete[] data;
	delete[] tableBuffer;

	volume.close();
}

int min(int a, int b) {
	return (a < b) ? a : b;
}

void determineSbScSd(int& sb, int& sd, int& sc, int volumeSize) {
	string chooseSc = "Selete a number of block per cluster sc:\n";
	int si = 1;
	int nb = 2;
	int sr = 32;
	int sv = volumeSize / BLOCKSIZE;

	vector<VolumeData> volumeOption;

	for (int sc = 4; sc <= 64; sc *= 2) {
		for (int sb = 1; sb <= 256; sb++) {	//sb = 256 mean total 65518 elements is used 
			int numOfElement = min(sb * BLOCKSIZE / 2, 65518);

			int sd = sv - si - nb * sb - sr;

			int numOfCluster = sd / sc;

			if (numOfCluster <= numOfElement) {
				VolumeData A;
				A.blocksPerTable_sb = sb;
				A.blocksPerCluster_sc = sc;
				A.blocksInData_sd = sd;
				volumeOption.push_back(A);
				break;
			}
		}
	}

	for (int i = 0; i < volumeOption.size(); i++) {
		chooseSc += to_string(i + 1) + ". " + to_string(volumeOption[i].blocksPerCluster_sc) + " blocks per cluster\n";
	}
	chooseSc += "Your option: ";
	std::cout << chooseSc;
	while (true) {
		int n = 0;
		std::cin >> n;

		if (n < 1 || n > volumeOption.size()) {
			std::cout << "Wrong input.\n";
			std::cout << "Your option: ";
			continue;
		}
		else {
			sb = volumeOption[n - 1].blocksPerTable_sb;
			sc = volumeOption[n - 1].blocksPerCluster_sc;
			sd = volumeOption[n - 1].blocksInData_sd;
			break;
		}
	}
}

void readBlock(std::fstream& file, int blocknumber, char* buffer) {
	file.seekg(blocknumber * BLOCKSIZE);
	file.read(buffer, BLOCKSIZE);
}

void writeBlock(std::fstream& file, int blockNumber, char* data) {
	file.seekp(blockNumber * BLOCKSIZE);
	file.write(data, BLOCKSIZE);
}

void writeVolumeInfo(char buffer[BLOCKSIZE], int sb_, int sc_, int sv_) {
	char signature[4] = "KKK";
	uint16_t bytesPerBlock = 512;
	uint8_t sc = sc_;
	uint16_t si = 1;
	uint8_t nb = 2;
	uint16_t sb = sb_;
	uint16_t sr = 32;
	uint16_t sv = sv_;
	uint16_t endsign = 0xAA55;

	size_t offset = 0;

	memcpy_s(buffer, BLOCKSIZE, signature, sizeof(signature) - 1);
	offset += sizeof(signature) - 1;

	memcpy_s(buffer + offset, BLOCKSIZE - offset, &bytesPerBlock, sizeof(bytesPerBlock));
	offset += sizeof(bytesPerBlock);

	memcpy_s(buffer + offset, BLOCKSIZE - offset, &sc, sizeof(sc));
	offset += sizeof(sc);

	memcpy_s(buffer + offset, BLOCKSIZE - offset, &si, sizeof(si));
	offset += sizeof(si);

	memcpy_s(buffer + offset, BLOCKSIZE - offset, &nb, sizeof(nb));
	offset += sizeof(nb);

	memcpy_s(buffer + offset, BLOCKSIZE - offset, &sb, sizeof(sb));
	offset += sizeof(sb);

	memcpy_s(buffer + offset, BLOCKSIZE - offset, &sr, sizeof(sr));
	offset += sizeof(sr);

	memcpy_s(buffer + offset, BLOCKSIZE - offset, &sv, sizeof(sv));
	offset += sizeof(sv);

	memcpy_s(buffer + BLOCKSIZE - sizeof(endsign), sizeof(endsign), &endsign, sizeof(endsign));
}

void readCluster(std::fstream& file, int clusternumber, char* buffer, int si, int nb, int sb, int sr, int sc) {
	file.seekg((si + nb * sb + sr) * BLOCKSIZE + (clusternumber - 2) * sc * BLOCKSIZE);
	file.read(buffer, sc * BLOCKSIZE);
}

void writeCluster(std::fstream& file, int clusternumber, char* data, int si, int nb, int sb, int sr, int sc) {
	file.seekp((si + nb * sb + sr) * BLOCKSIZE + (clusternumber - 2) * sc * BLOCKSIZE);
	file.write(data, sc * BLOCKSIZE);
}

void generateRandomData(char* arr, int length) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, 255);

	for (int i = 0; i < length; ++i) {
		arr[i] = static_cast<char>(dis(gen));
	}
}

void quickFormat(string filepath) {
	fstream volume(filepath, std::ios::out | std::ios::binary | std::ios::in);

	if (!volume.is_open()) {
		std::cout << "Cannot open volume file" << endl;
		return;
	}

	char buffer[BLOCKSIZE];
	readBlock(volume, 0, buffer);

	if (buffer[0] != 'K' && buffer[1] != 'K' && buffer[2] != 'K') {
		std::cout << "Not \"kkk\" volume file.";
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

	//cout << endl << sb << " " << sc << " " << sv << endl;

	//Set table data to 0000h without changing FFF7h
	volume.seekg(si * BLOCKSIZE);
	char* tableBuffer = new char[nb * sb * BLOCKSIZE];
	volume.read(tableBuffer, nb * sb * BLOCKSIZE);
	for (int i = 0; i < nb * sb * BLOCKSIZE; i += 2) {
		unsigned char byte1 = static_cast<unsigned char>(tableBuffer[i]);
		unsigned char byte2 = static_cast<unsigned char>(tableBuffer[i + 1]);

		if ((byte1 == 0xF7) && (byte2 == 0xFF)) {
			continue;
		}
		else {
			tableBuffer[i] = 0;
			tableBuffer[i + 1] = 0;
		}
	}
	volume.seekp(si * BLOCKSIZE);
	volume.write(tableBuffer, nb * sb * BLOCKSIZE);

	//Set data entry to 0
	std::memset(buffer, 0, BLOCKSIZE);
	for (int i = 0; i < sr; i++) {
		writeBlock(volume, si + nb * sb, buffer);
	}

	//Set all data to 0
	char* data = new char[BLOCKSIZE * sc];
	std::memset(data, 0, BLOCKSIZE * sc);
	for (int i = 2; i <= (sd + sc - 1) / sc; i++) {
		writeCluster(volume, i, data, si, nb, sb, sr, sc);
	}

	delete[] data;
	delete[] tableBuffer;

	volume.close();

	std::cout << "Quick format completed.\n";
}

void readCharArrayInHex(const char* arr, int length, int startIndex) {
	for (int i = startIndex; i < length; ++i) {
		std::cout << std::setw(2) << std::setfill('0') << std::hex
			<< static_cast<int>(static_cast<unsigned char>(arr[i]))
			<< " ";
	}
	std::cout << std::dec << std::endl;
}

int convertLittleEndianToInt(const char* arr, int position, int numBytes) {
	int value = 0;
	for (int i = 0; i < numBytes; ++i) {
		value |= (static_cast<unsigned char>(arr[position + i]) << (8 * i));
	}
	return value;
}

void readVolumeInfo(char buffer[BLOCKSIZE], int& sb, int& sc, int& sv) {
	sb = convertLittleEndianToInt(buffer, 9, 2);
	sc = convertLittleEndianToInt(buffer, 5, 1);
	sv = convertLittleEndianToInt(buffer, 13, 2);
}