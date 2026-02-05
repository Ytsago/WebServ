#include "StatusCode.hpp"

std::map<int, std::string> g_status_map;

void init_status_map() 
{
    if (!g_status_map.empty()) 
		return;
	g_status_map[OK] = "OK";
    g_status_map[NOT_MODIFIED] = "Not Modified";
    g_status_map[BAD_REQUEST] = "Bad Request";
    g_status_map[FORBIDDEN] = "Forbidden";
    g_status_map[NOT_FOUND] = "Not Found";
    g_status_map[METHOD_NOT_ALLOWED] = "Method Not Allowed";
    g_status_map[LENGHT_REQUIRED] = "Length Required";
    g_status_map[CONTENT_TOO_LARGE] = "Content Too Large";
    g_status_map[URI_TOO_LONG] = "URI Too Long";
    g_status_map[UNSUPORTED_MEDIA_TYPE] = "Unsupported Media Type";
    g_status_map[TOO_MANY_REQUEST] = "Too Many Requests";
    g_status_map[REQUEST_HEADER_FIELD_TOO_LARGE] = "Request Header Fields Too Large";
    g_status_map[INTERNAL_SERVER_ERROR] = "Internal Server Error";
    g_status_map[NOT_IMPLEMENTED] = "Not Implemented";
    g_status_map[HTTP_VERSION_NOT_SUPPORTED] = "HTTP Version Not Supported";
}