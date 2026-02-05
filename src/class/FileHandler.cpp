#include "FileHandler.hpp"
#include "ANetContainer.hpp"

FileHandler::FileHandler() {}

FileHandler::FileHandler(const FileHandler &other)
{
	(void)other;
}

FileHandler	&FileHandler::operator=(const FileHandler &other) 
{
	(void)other;
	return (*this);
}

FileHandler::~FileHandler() {
}