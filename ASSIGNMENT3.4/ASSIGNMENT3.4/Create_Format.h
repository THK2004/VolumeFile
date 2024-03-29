#pragma once
#include "Header.h"
#include <cmath>
#include <random>
#include <iomanip>
#include <sodium.h>
#include <conio.h>

#define BLOCKSIZE 512

struct VolumeData {
	int blocksPerTable_sb = 0;
	int blocksPerCluster_sc = 0;
	int blocksInData_sd = 0;
};
void createVolume(string, int);
int min(int a, int b);

void determineSbScSd(int&, int&, int&, int);

void readBlock(std::fstream& file, int blocknumber, char* buffer);
void writeBlock(std::fstream& file, int blockNumber, char* data);

void writeVolumeInfo(char[BLOCKSIZE], int, int, int, pair<string, string>);

void generateRandomData(char* arr, int length);

void readCluster(std::fstream& file, int clusternumber, char* buffer, int si, int nb, int sb, int sr, int sc);
void writeCluster(std::fstream& file, int clusternumber, char* data, int si, int nb, int sb, int sr, int sc);

void quickFormat(string);
void readCharArrayInHex(const char* arr, int length, int startIndex);
int convertLittleEndianToInt(const char* arr, int position, int numBytes);
void readVolumeInfo(char[BLOCKSIZE], int&, int&, int&);

pair<string, string> passwordCreate(int outputSize, int saltSize);	//OutputSize is 32 or 16
string generateRandomSalt(int length);
pair<string, string> argon2PasswordHashing32(string password, int salt_length);
pair<string, string> argon2PasswordHashing16(string password, int salt_length);

bool volumePasswordVerify(std::fstream&);