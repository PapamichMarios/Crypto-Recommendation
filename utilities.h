#ifndef __UTILITIES_H__
#define __UTILITIES_H__

void rerunCheck(int argc, int args);
void getInlineArguments(int argc, char** argv, short int &inputFileIndex, short int &outputFileIndex, bool &validateFlag);

std::map<std::string, float> vaderLexiconMap();
std::map<std::string, int> cryptosMap();

#endif
