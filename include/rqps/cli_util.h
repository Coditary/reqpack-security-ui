#pragma once

#include <iostream>
#include <string>

bool cli_stdout_is_tty();

std::string cli_green(const std::string& text);
std::string cli_red(const std::string& text);
std::string cli_yellow(const std::string& text);

void print_syft_missing_help(const std::string& syftPath);
void print_db_empty_help();

void print_rqps_banner_help();
void print_test_command_help();

bool is_rqps_known_command(const std::string& word);
