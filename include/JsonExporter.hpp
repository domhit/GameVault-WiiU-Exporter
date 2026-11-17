// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <string>
#include <vector>
#include "InstalledTitle.hpp"
bool ExportJson(const std::vector<InstalledTitle>& titles, std::string& error);
