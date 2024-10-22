// This is the source code of Ayu for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radscorp, 2024
#pragma once

namespace AyuMapper {

std::pair<std::string, std::vector<char>> serializeTextWithEntities(not_null<HistoryItem*> item);
int mapItemFlagsToMTPFlags(not_null<HistoryItem*> item);

} // namespace AyuMapper
