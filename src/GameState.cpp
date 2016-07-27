#include "pch.h"

#include "GameGlobal.h"
#include "GameState.h"
#include "GameWindow.h"
#include "Song.h"
#include "SongDatabase.h"
#include "SongWheel.h"

#include "ImageLoader.h"
#include "Song7K.h"

#include "NoteLoader7K.h"


#define DirectoryPrefix std::string("GameData/")
#define SkinsPrefix std::string("Skins/")
#define SongsPrefix std::string("Songs")
#define ScriptsPrefix std::string("Scripts/")

using namespace Game;

bool GameState::FileExistsOnSkin(const char* Filename, const char* Skin)
{
    std::filesystem::path path(Skin);
    path = DirectoryPrefix / (SkinsPrefix / path / Filename);

    return std::filesystem::exists(path);
}

GameState::GameState(): 
	StageImage(nullptr), 
	SongBG(nullptr), 
	CurrentGaugeType(0), 
	CurrentScoreType(0), 
	CurrentSubsystemType(0)
{
    CurrentSkin = "default";
    SelectedSong = nullptr;
    SKeeper7K = nullptr;
    Database = nullptr;
    Params = std::make_shared<GameParameters>();

    // TODO: circular references are possible :(
    std::filesystem::path SkinsDir(DirectoryPrefix + SkinsPrefix);
    std::vector<std::filesystem::path> listing = Utility::GetFileListing(SkinsDir);;
    for (auto s : listing)
    {
		auto st = s.filename().string();
        std::ifstream fallback;
        fallback.open((s / "fallback.txt").string());
        if (fallback.is_open() && s != "default")
        {
            std::string ln;
			while (getline(fallback, ln)) {
				if (Utility::ToLower(ln) != Utility::ToLower(st))
					Fallback[st].push_back(ln);
			}
        }
        if (!Fallback[st].size()) Fallback[st].push_back("default");
    }
}

std::filesystem::path GameState::GetSkinScriptFile(const char* Filename, const std::string& skin)
{
    std::string Fn = Filename;

    if (Fn.find(".lua") == std::string::npos)
        Fn += ".lua";

    return GetSkinFile(Fn, skin).replace_extension("");
}

std::shared_ptr<Game::Song> GameState::GetSelectedSongShared() const
{
    return SelectedSong;
}

std::string GameState::GetFirstFallbackSkin()
{
    return Fallback[GetSkin()][0];
}

GameState& GameState::GetInstance()
{
    static GameState* StateInstance = new GameState;
    return *StateInstance;
}

Song *GameState::GetSelectedSong() const
{
    return SelectedSong.get();
}

void GameState::SetSelectedSong(std::shared_ptr<Game::Song> Song)
{
    SelectedSong = Song;
}

std::filesystem::path GameState::GetSkinFile(const std::string &Name, const std::string &Skin)
{
    std::string Test = GetSkinPrefix(Skin) + Name;

    if (std::filesystem::exists(Test))
        return Test;

    if (Fallback.find(Skin) != Fallback.end())
    {
        for (auto &s : Fallback[Skin])
        {
            if (FileExistsOnSkin(Name.c_str(), s.c_str()))
                return GetSkinFile(Name, s);
        }
    }

    return Test;
}

std::filesystem::path GameState::GetSkinFile(const std::string& Name)
{
    return GetSkinFile(Name, GetSkin());
}

void GameState::Initialize()
{
    if (!Database)
    {
        Database = new SongDatabase("rd.db");

        SongBG = new Image();
        StageImage = new Image();
    }
}

void GameState::SetDifficultyIndex(uint32_t Index)
{
    SongWheel::GetInstance().SetDifficulty(Index);
}

uint32_t GameState::GetDifficultyIndex() const
{
    return SongWheel::GetInstance().GetDifficulty();
}

GameWindow* GameState::GetWindow()
{
    return &WindowFrame;
}

std::string GameState::GetDirectoryPrefix()
{
    return DirectoryPrefix;
}

std::string GameState::GetSkinPrefix()
{
    // I wonder if a directory transversal is possible. Or useful, for that matter.
    return GetSkinPrefix(GetSkin());
}

std::string GameState::GetSkinPrefix(const std::string& skin)
{
    return DirectoryPrefix + SkinsPrefix + skin + "/";
}

void GameState::SetSkin(std::string Skin)
{
    CurrentSkin = Skin;
}

std::string GameState::GetScriptsDirectory()
{
    return DirectoryPrefix + ScriptsPrefix;
}

SongDatabase* GameState::GetSongDatabase()
{
    return Database;
}

std::filesystem::path GameState::GetFallbackSkinFile(const std::string &Name)
{
    std::string Skin = GetSkin();

    if (Fallback.find(Skin) != Fallback.end())
    {
        for (auto s : Fallback[Skin])
            if (FileExistsOnSkin(Name.c_str(), s.c_str()))
                return GetSkinFile(Name, s);
    }

    return GetSkinPrefix() + Name;
}

void GameState::SetCurrentGaugeType(int GaugeType)
{
	this->CurrentGaugeType = GaugeType;
}

int GameState::GetCurrentGaugeType() const
{
	return CurrentGaugeType;
}

Image* GameState::GetSongBG()
{
	if (SelectedSong)
	{
		auto toLoad = SelectedSong->SongDirectory / SelectedSong->BackgroundFilename;

		if (std::filesystem::exists(toLoad))
		{
			SongBG->Assign(toLoad, true);
			return SongBG;
		}

		// file doesn't exist
		return nullptr;
	}

	// no song selected
	return nullptr;
}

Image* GameState::GetSongStage()
{
	if (SelectedSong)
	{
		if (SelectedSong->Mode == MODE_VSRG)
		{
			VSRG::Song *Song = static_cast<VSRG::Song*>(SelectedSong.get());

			if (Song->Difficulties.size() > GetDifficultyIndex())
			{
				std::filesystem::path File = Database->GetStageFile(Song->Difficulties.at(GetDifficultyIndex())->ID);

				// Oh so it's loaded and it's not in the database, fine.
				if (File.string().length() == 0 && Song->Difficulties.at(GetDifficultyIndex())->Data)
					File = Song->Difficulties.at(GetDifficultyIndex())->Data->StageFile;

				auto toLoad = SelectedSong->SongDirectory / File;

				// ojn files use their cover inside the very ojn
				if (File.extension() == ".ojn")
				{
					size_t read;
					const unsigned char* buf = reinterpret_cast<const unsigned char*>(LoadOJNCover(toLoad, read));
					ImageData data = ImageLoader::GetDataForImageFromMemory(buf, read);
					StageImage->SetTextureData(data, true);
					delete[] buf;

					return StageImage;
				}

				if (File.string().length() && std::filesystem::exists(toLoad))
				{
					StageImage->Assign(toLoad, true);
					return StageImage;
				}

				return nullptr;
			}

			return nullptr;
			// Oh okay, no difficulty assigned.
		}
		// Stage file not supported for DC songs yet
		return nullptr;
	}

	// no song selected
	return nullptr;
}

Image* GameState::GetSkinImage(const std::string& Path)
{
    /* Special paths */
    if (Path == "STAGEFILE")
	    return GetSongStage();

    if (Path == "SONGBG")
	    return GetSongBG();

    /* Regular paths */
    if (Path.length())
        return ImageLoader::Load(GetSkinFile(Path, GetSkin()));

	// no path?
    return nullptr;
}

bool GameState::SkinSupportsChannelCount(int Count)
{
    char nstr[256];
    sprintf(nstr, "Channels%d", Count);
    return Configuration::ListExists(nstr);
}

std::string GameState::GetSkin()
{
    return CurrentSkin;
}

ScoreKeeper7K* GameState::GetScorekeeper7K()
{
    return SKeeper7K.get();
}

void GameState::SetScorekeeper7K(std::shared_ptr<ScoreKeeper7K> Other)
{
    SKeeper7K = Other;
}

void GameState::SetCurrentScoreType(int ScoreType)
{
	CurrentScoreType = ScoreType;
}

int GameState::GetCurrentScoreType() const
{
	return CurrentScoreType;
}

void GameState::SetCurrentSystemType(int SystemType)
{
	CurrentSubsystemType = SystemType;
}

int GameState::GetCurrentSystemType() const
{
	return CurrentSubsystemType;
}

void GameState::SubmitScore(std::shared_ptr<ScoreKeeper7K> score)
{
	// TODO
}

void GameState::SortWheelBy(int criteria)
{
	SongWheel::GetInstance().SortBy(ESortCriteria(criteria));
}
