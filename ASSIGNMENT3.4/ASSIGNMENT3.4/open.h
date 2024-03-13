#pragma once
#include "Create_Format.h"
#include <chrono>

#define ENTRYSIZE 64

void openVolume(string);
void entryCreate(char entry[ENTRYSIZE], char fileName[24], int filenameSize, char extentsion[3], int startingCluster, int fileSize);
void writeEntry(std::fstream&, char[ENTRYSIZE], int entryNumber, int firstEntryPos);
void readEntry(std::fstream&, char buffer[ENTRYSIZE], int entryNumber, int firstEntryPos);
bool filePasswordVerify(char entry[ENTRYSIZE]);