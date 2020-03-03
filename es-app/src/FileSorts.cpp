#include "FileSorts.h"

#include "utils/StringUtil.h"

namespace FileSorts
{
	const FileData::SortType typesArr[] = {
		FileData::SortType(&compareName, true, "nombre del juego, ascendente"),
		FileData::SortType(&compareName, false, "nombre del juego, descendente"),

		FileData::SortType(&compareRating, true, "puntuacion, ascendente"),
		FileData::SortType(&compareRating, false, "puntuacion, descendente"),

		FileData::SortType(&compareTimesPlayed, true, "veces jugado, ascendente"),
		FileData::SortType(&compareTimesPlayed, false, "veces jugado, descendente"),

		FileData::SortType(&compareLastPlayed, true, "jugados recientemente, ascendente"),
		FileData::SortType(&compareLastPlayed, false, "jugados recientemente, descendente"),

		FileData::SortType(&compareNumPlayers, true, "numero de jugadores, ascendente"),
		FileData::SortType(&compareNumPlayers, false, "numero de jugadores, descendente"),

		FileData::SortType(&compareReleaseDate, true, "fecha de lanzamiento, ascendente"),
		FileData::SortType(&compareReleaseDate, false, "fecha de lanzamiento, descendente"),

		FileData::SortType(&compareGenre, true, "genero, ascendente"),
		FileData::SortType(&compareGenre, false, "genero, descendente"),

		FileData::SortType(&compareDeveloper, true, "desarrollador, ascendente"),
		FileData::SortType(&compareDeveloper, false, "desarrollador, descendente"),

		FileData::SortType(&comparePublisher, true, "editor, ascendente"),
		FileData::SortType(&comparePublisher, false, "editor, descendente"),

		FileData::SortType(&compareSystem, true, "sistema, ascendente"),
		FileData::SortType(&compareSystem, false, "sistema, descendente")
	};

	const std::vector<FileData::SortType> SortTypes(typesArr, typesArr + sizeof(typesArr)/sizeof(typesArr[0]));

	//returns if file1 should come before file2
	bool compareName(const FileData* file1, const FileData* file2)
	{
		// we compare the actual metadata name, as collection files have the system appended which messes up the order
		std::string name1 = Utils::String::toUpper(file1->metadata.get("sortname"));
		std::string name2 = Utils::String::toUpper(file2->metadata.get("sortname"));
		if(name1.empty()){
			name1 = Utils::String::toUpper(file1->metadata.get("name"));
		}
		if(name2.empty()){
			name2 = Utils::String::toUpper(file2->metadata.get("name"));
		}
		return name1.compare(name2) < 0;
	}

	bool compareRating(const FileData* file1, const FileData* file2)
	{
		return file1->metadata.getFloat("rating") < file2->metadata.getFloat("rating");
	}

	bool compareTimesPlayed(const FileData* file1, const FileData* file2)
	{
		//only games have playcount metadata
		if(file1->metadata.getType() == GAME_METADATA && file2->metadata.getType() == GAME_METADATA)
		{
			return (file1)->metadata.getInt("playcount") < (file2)->metadata.getInt("playcount");
		}

		return false;
	}

	bool compareLastPlayed(const FileData* file1, const FileData* file2)
	{
		// since it's stored as an ISO string (YYYYMMDDTHHMMSS), we can compare as a string
		// as it's a lot faster than the time casts and then time comparisons
		return (file1)->metadata.get("lastplayed") < (file2)->metadata.get("lastplayed");
	}

	bool compareNumPlayers(const FileData* file1, const FileData* file2)
	{
		return (file1)->metadata.getInt("players") < (file2)->metadata.getInt("players");
	}

	bool compareReleaseDate(const FileData* file1, const FileData* file2)
	{
		// since it's stored as an ISO string (YYYYMMDDTHHMMSS), we can compare as a string
		// as it's a lot faster than the time casts and then time comparisons
		return (file1)->metadata.get("releasedate") < (file2)->metadata.get("releasedate");
	}

	bool compareGenre(const FileData* file1, const FileData* file2)
	{
		std::string genre1 = Utils::String::toUpper(file1->metadata.get("genre"));
		std::string genre2 = Utils::String::toUpper(file2->metadata.get("genre"));
		return genre1.compare(genre2) < 0;
	}

	bool compareDeveloper(const FileData* file1, const FileData* file2)
	{
		std::string developer1 = Utils::String::toUpper(file1->metadata.get("developer"));
		std::string developer2 = Utils::String::toUpper(file2->metadata.get("developer"));
		return developer1.compare(developer2) < 0;
	}

	bool comparePublisher(const FileData* file1, const FileData* file2)
	{
		std::string publisher1 = Utils::String::toUpper(file1->metadata.get("publisher"));
		std::string publisher2 = Utils::String::toUpper(file2->metadata.get("publisher"));
		return publisher1.compare(publisher2) < 0;
	}

	bool compareSystem(const FileData* file1, const FileData* file2)
	{
		std::string system1 = Utils::String::toUpper(file1->getSystemName());
		std::string system2 = Utils::String::toUpper(file2->getSystemName());
		return system1.compare(system2) < 0;
	}
};
