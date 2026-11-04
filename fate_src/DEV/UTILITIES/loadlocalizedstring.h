/*
	
	Function definitions for working with localized strings

	loadlocalizedstring.h

	Translations.com
	October 2007

*/
#ifndef _LOADLOCALIZEDSTRING_H
#define _LOADLOCALIZEDSTRING_H

#include "../UTILITIES/constants.h"
#include <string>
#include <windows.h>
#include <stdio.h>

std::string LoadLocalizedString(int rcIndex);

// functions to replace $1, $2, etc from the RC file entries
std::string LoadLocalizedString1Variable(int rcIndex, std::string firstString);

std::string LoadLocalizedString2Variables(int rcIndex, std::string firstString,
										  std::string secondString);

std::string LoadLocalizedString3Variables(int rcIndex, std::string firstString,
										  std::string secondString, std::string thirdString);

std::string LoadLocalizedString4Variables(int rcIndex, std::string firstString,
										  std::string secondString, std::string thirdString,
										  std::string fourthString);

std::string LoadLocalizedString5Variables(int rcIndex, std::string firstString,
										  std::string secondString, std::string thirdString,
										  std::string fourthString, std::string fifthString);

#endif