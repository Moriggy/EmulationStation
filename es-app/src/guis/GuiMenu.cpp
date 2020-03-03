#include "guis/GuiMenu.h"

#include "components/OptionListComponent.h"
#include "components/SliderComponent.h"
#include "components/SwitchComponent.h"
#include "guis/GuiCollectionSystemsOptions.h"
#include "guis/GuiDetectDevice.h"
#include "guis/GuiGeneralScreensaverOptions.h"
#include "guis/GuiMsgBox.h"
#include "guis/GuiScraperStart.h"
#include "guis/GuiSettings.h"
#include "views/UIModeController.h"
#include "views/ViewController.h"
#include "CollectionSystemManager.h"
#include "EmulationStation.h"
#include "Scripting.h"
#include "SystemData.h"
#include "VolumeControl.h"
#include <SDL_events.h>
#include <algorithm>
#include "platform.h"

GuiMenu::GuiMenu(Window* window) : GuiComponent(window), mMenu(window, "MENU EmulOS"), mVersion(window)
{
	bool isFullUI = UIModeController::getInstance()->isUIModeFull();

	if (isFullUI)
		addEntry("SCRAPER", 0x777777FF, true, [this] { openScraperSettings(); });

	addEntry("AJUSTES DE SONIDO", 0x777777FF, true, [this] { openSoundSettings(); });


	if (isFullUI)
		addEntry("AJUSTES DE INTERFAZ", 0x777777FF, true, [this] { openUISettings(); });

	if (isFullUI)
		addEntry("AJUSTES DE COLECCION DE JUEGO", 0x777777FF, true, [this] { openCollectionSystemSettings(); });

	if (isFullUI)
		addEntry("OTROS AJUSTES", 0x777777FF, true, [this] { openOtherSettings(); });

	if (isFullUI)
		addEntry("CONFIGURAR ENTRADA (MANDO)", 0x777777FF, true, [this] { openConfigInput(); });

	addEntry("SALIR", 0x777777FF, true, [this] {openQuitMenu(); });

	addChild(&mMenu);
	addVersionInfo();
	setSize(mMenu.getSize());
	setPosition((Renderer::getScreenWidth() - mSize.x()) / 2, Renderer::getScreenHeight() * 0.15f);
}

void GuiMenu::openScraperSettings()
{
	auto s = new GuiSettings(mWindow, "SCRAPER");

	std::string scraper = Settings::getInstance()->getString("Scraper");

	// scrape from
	auto scraper_list = std::make_shared< OptionListComponent< std::string > >(mWindow, "SCRAPEAR DESDE", false);
	std::vector<std::string> scrapers = getScraperList();

	// Select either the first entry of the one read from the settings, just in case the scraper from settings has vanished.
	for(auto it = scrapers.cbegin(); it != scrapers.cend(); it++)
		scraper_list->add(*it, *it, *it == scraper);

	s->addWithLabel("SCRAPEAR DESDE", scraper_list);
	s->addSaveFunc([scraper_list] { Settings::getInstance()->setString("Scraper", scraper_list->getSelected()); });

	// scrape ratings
	auto scrape_ratings = std::make_shared<SwitchComponent>(mWindow);
	scrape_ratings->setState(Settings::getInstance()->getBool("ScrapeRatings"));
	s->addWithLabel("SCRAPEAR PUNTUACION", scrape_ratings);
	s->addSaveFunc([scrape_ratings] { Settings::getInstance()->setBool("ScrapeRatings", scrape_ratings->getState()); });

	if (scraper == "ScreenScraper")
	{
		// image source
		std::string imageSourceName = Settings::getInstance()->getString("ScrapperImageSrc");
		auto imageSource = std::make_shared< OptionListComponent<std::string> >(mWindow, "IMAGEN PREFERIDA", false);
		imageSource->add("NINGUNA", "", imageSourceName.empty());
		imageSource->add("CAPTURA DE PANTALLA", "ss", imageSourceName == "ss");
		imageSource->add("CAJA 2D", "box-2D", imageSourceName == "box-2D");
		imageSource->add("CAJA 3D", "box-3D", imageSourceName == "box-3D");
		imageSource->add("MIX", "mixrbv1", imageSourceName == "mixrbv1");
		imageSource->add("LOGO", "wheel", imageSourceName == "wheel");
		s->addWithLabel("IMAGEN PREFERIDA", imageSource);

		s->addSaveFunc([imageSource] {
			if (Settings::getInstance()->getString("ScrapperImageSrc") != imageSource->getSelected())
				Settings::getInstance()->setString("ScrapperImageSrc", imageSource->getSelected());
		});

		std::string thumbSourceName = Settings::getInstance()->getString("ScrapperThumbSrc");
		auto thumbSource = std::make_shared< OptionListComponent<std::string> >(mWindow, "FOTO PREFERIDA", false);
		thumbSource->add("NINGUNA", "", thumbSourceName.empty());
		thumbSource->add("CAPTURA DE PANTALLA", "ss", thumbSourceName == "ss");
		thumbSource->add("CAJA 2D", "box-2D", thumbSourceName == "box-2D");
		thumbSource->add("CAJA 3D", "box-3D", thumbSourceName == "box-3D");
		thumbSource->add("MIX", "mixrbv1", thumbSourceName == "mixrbv1");
		thumbSource->add("LOGO", "wheel", thumbSourceName == "wheel");
		s->addWithLabel("FOTO PREFERIDA", thumbSource);

		s->addSaveFunc([thumbSource] {
			if (Settings::getInstance()->getString("ScrapperThumbSrc") != thumbSource->getSelected())
				Settings::getInstance()->setString("ScrapperThumbSrc", thumbSource->getSelected());
		});

		// scrape marquee
		auto scrape_marquee = std::make_shared<SwitchComponent>(mWindow);
		scrape_marquee->setState(Settings::getInstance()->getBool("ScrapeMarquee"));
		s->addWithLabel("SCRAPEAR MARQUESINA", scrape_marquee);
		s->addSaveFunc([scrape_marquee] { Settings::getInstance()->setBool("ScrapeMarquee", scrape_marquee->getState()); });

		// scrape video
		auto scrape_video = std::make_shared<SwitchComponent>(mWindow);
		scrape_video->setState(Settings::getInstance()->getBool("ScrapeVideos"));
		s->addWithLabel("SCRAPEAR VIDEOS", scrape_video);
		s->addSaveFunc([scrape_video] { Settings::getInstance()->setBool("ScrapeVideos", scrape_video->getState()); });
	}

	// scrape now
	ComponentListRow row;
	auto openScrapeNow = [this] { mWindow->pushGui(new GuiScraperStart(mWindow)); };
	std::function<void()> openAndSave = openScrapeNow;
	openAndSave = [s, openAndSave] { s->save(); openAndSave(); };
	s->addEntry("SCRAPEAR AHORA"), false, openAndSave);

	scraper_list->setSelectedChangedCallback([this, s, scraper, scraper_list](std::string value)
	{
		if (value != scraper && (scraper == "ScreenScraper" || value == "ScreenScraper"))
		{
			Settings::getInstance()->setString("Scraper", value);
			delete s;
			openScraperSettings();
		}
	});

	mWindow->pushGui(s);
}

void GuiMenu::openSoundSettings()
{
	auto s = new GuiSettings(mWindow, "AJUSTES DE SONIDO");

	// volume
	auto volume = std::make_shared<SliderComponent>(mWindow, 0.f, 100.f, 1.f, "%");
	volume->setValue((float)VolumeControl::getInstance()->getVolume());
	s->addWithLabel("VOLUMEN DEL SISTEMA", volume);
	s->addSaveFunc([volume] { VolumeControl::getInstance()->setVolume((int)Math::round(volume->getValue())); });

	if (UIModeController::getInstance()->isUIModeFull())
	{
#if defined(__linux__)
		// audio card
		auto audio_card = std::make_shared< OptionListComponent<std::string> >(mWindow, "TARJETA DE AUDIO", false);
		std::vector<std::string> audio_cards;
	#ifdef _RPI_
		// RPi Specific  Audio Cards
		audio_cards.push_back("local");
		audio_cards.push_back("hdmi");
		audio_cards.push_back("both");
	#endif
		audio_cards.push_back("default");
		audio_cards.push_back("sysdefault");
		audio_cards.push_back("dmix");
		audio_cards.push_back("hw");
		audio_cards.push_back("plughw");
		audio_cards.push_back("null");
		if (Settings::getInstance()->getString("AudioCard") != "") {
			if(std::find(audio_cards.begin(), audio_cards.end(), Settings::getInstance()->getString("AudioCard")) == audio_cards.end()) {
				audio_cards.push_back(Settings::getInstance()->getString("AudioCard"));
			}
		}
		for(auto ac = audio_cards.cbegin(); ac != audio_cards.cend(); ac++)
			audio_card->add(*ac, *ac, Settings::getInstance()->getString("AudioCard") == *ac);
		s->addWithLabel("TARJETA DE AUDIO", audio_card);
		s->addSaveFunc([audio_card] {
			Settings::getInstance()->setString("AudioCard", audio_card->getSelected());
			VolumeControl::getInstance()->deinit();
			VolumeControl::getInstance()->init();
		});

		// volume control device
		auto vol_dev = std::make_shared< OptionListComponent<std::string> >(mWindow, "DISPOSITIVO DE AUDIO", false);
		std::vector<std::string> transitions;
		transitions.push_back("PCM");
		transitions.push_back("Speaker");
		transitions.push_back("Master");
		transitions.push_back("Digital");
		transitions.push_back("Analogico");
		if (Settings::getInstance()->getString("AudioDevice") != "") {
			if(std::find(transitions.begin(), transitions.end(), Settings::getInstance()->getString("AudioDevice")) == transitions.end()) {
				transitions.push_back(Settings::getInstance()->getString("AudioDevice"));
			}
		}
		for(auto it = transitions.cbegin(); it != transitions.cend(); it++)
			vol_dev->add(*it, *it, Settings::getInstance()->getString("AudioDevice") == *it);
		s->addWithLabel("DISPOSITIVO DE AUDIO", vol_dev);
		s->addSaveFunc([vol_dev] {
			Settings::getInstance()->setString("AudioDevice", vol_dev->getSelected());
			VolumeControl::getInstance()->deinit();
			VolumeControl::getInstance()->init();
		});
#endif

		// disable sounds
		auto sounds_enabled = std::make_shared<SwitchComponent>(mWindow);
		sounds_enabled->setState(Settings::getInstance()->getBool("EnableSounds"));
		s->addWithLabel("ACTIVAR SONIDOS DE NAVEGACION", sounds_enabled);
		s->addSaveFunc([sounds_enabled] {
			if (sounds_enabled->getState()
				&& !Settings::getInstance()->getBool("EnableSounds")
				&& PowerSaver::getMode() == PowerSaver::INSTANT)
			{
				Settings::getInstance()->setString("PowerSaverMode", "default");
				PowerSaver::init();
			}
			Settings::getInstance()->setBool("EnableSounds", sounds_enabled->getState());
		});

		auto video_audio = std::make_shared<SwitchComponent>(mWindow);
		video_audio->setState(Settings::getInstance()->getBool("VideoAudio"));
		s->addWithLabel("ACTIVAR SONIDO DE VIDEOS", video_audio);
		s->addSaveFunc([video_audio] { Settings::getInstance()->setBool("VideoAudio", video_audio->getState()); });

#ifdef _RPI_
		// OMX player Audio Device
		auto omx_audio_dev = std::make_shared< OptionListComponent<std::string> >(mWindow, "DISPOSITIVO DE AUDIO OMX PLAYER", false);
		std::vector<std::string> omx_cards;
		// RPi Specific  Audio Cards
		omx_cards.push_back("local");
		omx_cards.push_back("hdmi");
		omx_cards.push_back("both");
		omx_cards.push_back("alsa:hw:0,0");
		omx_cards.push_back("alsa:hw:1,0");
		if (Settings::getInstance()->getString("OMXAudioDev") != "") {
			if (std::find(omx_cards.begin(), omx_cards.end(), Settings::getInstance()->getString("OMXAudioDev")) == omx_cards.end()) {
				omx_cards.push_back(Settings::getInstance()->getString("OMXAudioDev"));
			}
		}
		for (auto it = omx_cards.cbegin(); it != omx_cards.cend(); it++)
			omx_audio_dev->add(*it, *it, Settings::getInstance()->getString("OMXAudioDev") == *it);
		s->addWithLabel("DISPOSITIVO DE AUDIO OMX PLAYER", omx_audio_dev);
		s->addSaveFunc([omx_audio_dev] {
			if (Settings::getInstance()->getString("OMXAudioDev") != omx_audio_dev->getSelected())
				Settings::getInstance()->setString("OMXAudioDev", omx_audio_dev->getSelected());
		});
#endif
	}

	mWindow->pushGui(s);

}

void GuiMenu::openUISettings()
{
	auto s = new GuiSettings(mWindow, "AJUSTES DE INTERFAZ");

	//UI mode
	auto UImodeSelection = std::make_shared< OptionListComponent<std::string> >(mWindow, "MODO DE INTERFAZ", false);
	std::vector<std::string> UImodes = UIModeController::getInstance()->getUIModes();
	for (auto it = UImodes.cbegin(); it != UImodes.cend(); it++)
		UImodeSelection->add(*it, *it, Settings::getInstance()->getString("UIMode") == *it);
	s->addWithLabel("MODO DE INTERFAZ", UImodeSelection);
	Window* window = mWindow;
	s->addSaveFunc([ UImodeSelection, window]
	{
		std::string selectedMode = UImodeSelection->getSelected();
		if (selectedMode != "Full")
		{
			std::string msg = "You are changing the UI to a restricted mode:\n" + selectedMode + "\n";
			msg += "This will hide most menu-options to prevent changes to the system.\n";
			msg += "To unlock and return to the full UI, enter this code: \n";
			msg += "\"" + UIModeController::getInstance()->getFormattedPassKeyStr() + "\"\n\n";
			msg += "Do you want to proceed?";
			window->pushGui(new GuiMsgBox(window, msg,
				"SI", [selectedMode] {
					LOG(LogDebug) << "Setting UI mode to " << selectedMode;
					Settings::getInstance()->setString("UIMode", selectedMode);
					Settings::getInstance()->saveFile();
			}, "NO",nullptr));
		}
	});

	// screensaver
	ComponentListRow screensaver_row;
	screensaver_row.elements.clear();
	screensaver_row.addElement(std::make_shared<TextComponent>(mWindow, "AJUSTES DE SCREENSAVER", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
	screensaver_row.addElement(makeArrow(mWindow), false);
	screensaver_row.makeAcceptInputHandler(std::bind(&GuiMenu::openScreensaverOptions, this));
	s->addRow(screensaver_row);

	// quick system select (left/right in game list view)
	auto quick_sys_select = std::make_shared<SwitchComponent>(mWindow);
	quick_sys_select->setState(Settings::getInstance()->getBool("QuickSystemSelect"));
	s->addWithLabel("SELECCIÓN RAPIDA DEL SISTEMA", quick_sys_select);
	s->addSaveFunc([quick_sys_select] { Settings::getInstance()->setBool("QuickSystemSelect", quick_sys_select->getState()); });

	// carousel transition option
	auto move_carousel = std::make_shared<SwitchComponent>(mWindow);
	move_carousel->setState(Settings::getInstance()->getBool("MoveCarousel"));
	s->addWithLabel("TRANSICION DEL CAROUSEL", move_carousel);
	s->addSaveFunc([move_carousel] {
		if (move_carousel->getState()
			&& !Settings::getInstance()->getBool("MoveCarousel")
			&& PowerSaver::getMode() == PowerSaver::INSTANT)
		{
			Settings::getInstance()->setString("PowerSaverMode", "default");
			PowerSaver::init();
		}
		Settings::getInstance()->setBool("MoveCarousel", move_carousel->getState());
	});

	// transition style
	auto transition_style = std::make_shared< OptionListComponent<std::string> >(mWindow, "TIPO DE TRANSICION", false);
	std::vector<std::string> transitions;
	transitions.push_back("fade");
	transitions.push_back("slide");
	transitions.push_back("instantaneo");
	for(auto it = transitions.cbegin(); it != transitions.cend(); it++)
		transition_style->add(*it, *it, Settings::getInstance()->getString("TransitionStyle") == *it);
	s->addWithLabel("TIPO DE TRANSITION", transition_style);
	s->addSaveFunc([transition_style] {
		if (Settings::getInstance()->getString("TransitionStyle") == "instantaneo"
			&& transition_style->getSelected() != "instantaneo"
			&& PowerSaver::getMode() == PowerSaver::INSTANT)
		{
			Settings::getInstance()->setString("PowerSaverMode", "default");
			PowerSaver::init();
		}
		Settings::getInstance()->setString("TransitionStyle", transition_style->getSelected());
	});

	// theme set
	auto themeSets = ThemeData::getThemeSets();

	if(!themeSets.empty())
	{
		std::map<std::string, ThemeSet>::const_iterator selectedSet = themeSets.find(Settings::getInstance()->getString("ThemeSet"));
		if(selectedSet == themeSets.cend())
			selectedSet = themeSets.cbegin();

		auto theme_set = std::make_shared< OptionListComponent<std::string> >(mWindow, "SELECCIONAR THEME", false);
		for(auto it = themeSets.cbegin(); it != themeSets.cend(); it++)
			theme_set->add(it->first, it->first, it == selectedSet);
		s->addWithLabel("SELECCIONAR THEME", theme_set);

		Window* window = mWindow;
		s->addSaveFunc([window, theme_set]
		{
			bool needReload = false;
			std::string oldTheme = Settings::getInstance()->getString("ThemeSet");
			if(oldTheme != theme_set->getSelected())
				needReload = true;

			Settings::getInstance()->setString("ThemeSet", theme_set->getSelected());

			if(needReload)
			{
				Scripting::fireEvent("theme-changed", theme_set->getSelected(), oldTheme);
				CollectionSystemManager::get()->updateSystemsList();
				ViewController::get()->goToStart();
				ViewController::get()->reloadAll(); // TODO - replace this with some sort of signal-based implementation
			}
		});
	}

	// GameList view style
	auto gamelist_style = std::make_shared< OptionListComponent<std::string> >(mWindow, "TIPO DE VISTA DE LISTADO DE JUEGOS", false);
	std::vector<std::string> styles;
	styles.push_back("automatico");
	styles.push_back("basico");
	styles.push_back("detallado");
	styles.push_back("video");
	styles.push_back("grid");

	for (auto it = styles.cbegin(); it != styles.cend(); it++)
		gamelist_style->add(*it, *it, Settings::getInstance()->getString("GamelistViewStyle") == *it);
	s->addWithLabel("TIPO DE VISTA DE LISTADO DE JUEGOS", gamelist_style);
	s->addSaveFunc([gamelist_style] {
		bool needReload = false;
		if (Settings::getInstance()->getString("GamelistViewStyle") != gamelist_style->getSelected())
			needReload = true;
		Settings::getInstance()->setString("GamelistViewStyle", gamelist_style->getSelected());
		if (needReload)
			ViewController::get()->reloadAll();
	});

	// Optionally start in selected system
	auto systemfocus_list = std::make_shared< OptionListComponent<std::string> >(mWindow, "EMPEZAR EN EL SISTEMA", false);
	systemfocus_list->add("NINGUNO", "", Settings::getInstance()->getString("StartupSystem") == "");
	for (auto it = SystemData::sSystemVector.cbegin(); it != SystemData::sSystemVector.cend(); it++)
	{
		if ("emulos" != (*it)->getName())
		{
			systemfocus_list->add((*it)->getName(), (*it)->getName(), Settings::getInstance()->getString("StartupSystem") == (*it)->getName());
		}
	}
	s->addWithLabel("EMPEZAR EN EL SISTEMA", systemfocus_list);
	s->addSaveFunc([systemfocus_list] {
		Settings::getInstance()->setString("StartupSystem", systemfocus_list->getSelected());
	});

	// show help
	auto show_help = std::make_shared<SwitchComponent>(mWindow);
	show_help->setState(Settings::getInstance()->getBool("ShowHelpPrompts"));
	s->addWithLabel("AYUDA EN PANTALLA", show_help);
	s->addSaveFunc([show_help] { Settings::getInstance()->setBool("ShowHelpPrompts", show_help->getState()); });

	// enable filters (ForceDisableFilters)
	auto enable_filter = std::make_shared<SwitchComponent>(mWindow);
	enable_filter->setState(!Settings::getInstance()->getBool("ForceDisableFilters"));
	s->addWithLabel("ACTIVAR FILTROS", enable_filter);
	s->addSaveFunc([enable_filter] {
		bool filter_is_enabled = !Settings::getInstance()->getBool("ForceDisableFilters");
		Settings::getInstance()->setBool("ForceDisableFilters", !enable_filter->getState());
		if (enable_filter->getState() != filter_is_enabled) ViewController::get()->ReloadAndGoToStart();
	});

	mWindow->pushGui(s);

}

void GuiMenu::openOtherSettings()
{
	auto s = new GuiSettings(mWindow, "OTROS AJUSTES");

	// maximum vram
	auto max_vram = std::make_shared<SliderComponent>(mWindow, 0.f, 1000.f, 10.f, "Mb");
	max_vram->setValue((float)(Settings::getInstance()->getInt("MaxVRAM")));
	s->addWithLabel("LIMITE DE VRAM", max_vram);
	s->addSaveFunc([max_vram] { Settings::getInstance()->setInt("MaxVRAM", (int)Math::round(max_vram->getValue())); });

	// power saver
	auto power_saver = std::make_shared< OptionListComponent<std::string> >(mWindow, "MODOS DE AHORRO DE ENERGIA", false);
	std::vector<std::string> modes;
	modes.push_back("deshabilitado");
	modes.push_back("por defecto");
	modes.push_back("mejorado");
	modes.push_back("instantaneo");
	for (auto it = modes.cbegin(); it != modes.cend(); it++)
		power_saver->add(*it, *it, Settings::getInstance()->getString("PowerSaverMode") == *it);
	s->addWithLabel("MODOS DE AHORRO DE ENERGIA", power_saver);
	s->addSaveFunc([this, power_saver] {
		if (Settings::getInstance()->getString("PowerSaverMode") != "instantaneo" && power_saver->getSelected() == "instantaneo") {
			Settings::getInstance()->setString("TransitionStyle", "instant");
			Settings::getInstance()->setBool("MoveCarousel", false);
			Settings::getInstance()->setBool("EnableSounds", false);
		}
		Settings::getInstance()->setString("PowerSaverMode", power_saver->getSelected());
		PowerSaver::init();
	});

	// gamelists
	auto gamelistsSaveMode = std::make_shared< OptionListComponent<std::string> >(mWindow, "SAVE METADATA", false);
	std::vector<std::string> saveModes;
	saveModes.push_back("al salir");
	saveModes.push_back("siempre");
	saveModes.push_back("nunca");

	for(auto it = saveModes.cbegin(); it != saveModes.cend(); it++)
		gamelistsSaveMode->add(*it, *it, Settings::getInstance()->getString("SaveGamelistsMode") == *it);
	s->addWithLabel("SAVE METADATA", gamelistsSaveMode);
	s->addSaveFunc([gamelistsSaveMode] {
		Settings::getInstance()->setString("SaveGamelistsMode", gamelistsSaveMode->getSelected());
	});

	auto parse_gamelists = std::make_shared<SwitchComponent>(mWindow);
	parse_gamelists->setState(Settings::getInstance()->getBool("ParseGamelistOnly"));
	s->addWithLabel("SINCRONIZAR SOLO GAMELIST", parse_gamelists);
	s->addSaveFunc([parse_gamelists] { Settings::getInstance()->setBool("ParseGamelistOnly", parse_gamelists->getState()); });

	auto local_art = std::make_shared<SwitchComponent>(mWindow);
	local_art->setState(Settings::getInstance()->getBool("LocalArt"));
	s->addWithLabel("BUSCAR LOCAL ART", local_art);
	s->addSaveFunc([local_art] { Settings::getInstance()->setBool("LocalArt", local_art->getState()); });

	// hidden files
	auto hidden_files = std::make_shared<SwitchComponent>(mWindow);
	hidden_files->setState(Settings::getInstance()->getBool("ShowHiddenFiles"));
	s->addWithLabel("MOSTRAR ARCHIVOS OCULTOS", hidden_files);
	s->addSaveFunc([hidden_files] { Settings::getInstance()->setBool("ShowHiddenFiles", hidden_files->getState()); });

#ifdef _RPI_
	// Video Player - VideoOmxPlayer
	auto omx_player = std::make_shared<SwitchComponent>(mWindow);
	omx_player->setState(Settings::getInstance()->getBool("VideoOmxPlayer"));
	s->addWithLabel("USAR OMX PLAYER (HW ACELERADO)", omx_player);
	s->addSaveFunc([omx_player]
	{
		// need to reload all views to re-create the right video components
		bool needReload = false;
		if(Settings::getInstance()->getBool("VideoOmxPlayer") != omx_player->getState())
			needReload = true;

		Settings::getInstance()->setBool("VideoOmxPlayer", omx_player->getState());

		if(needReload)
			ViewController::get()->reloadAll();
	});

#endif

	// framerate
	auto framerate = std::make_shared<SwitchComponent>(mWindow);
	framerate->setState(Settings::getInstance()->getBool("DrawFramerate"));
	s->addWithLabel("VER FRAMERATE", framerate);
	s->addSaveFunc([framerate] { Settings::getInstance()->setBool("DrawFramerate", framerate->getState()); });


	mWindow->pushGui(s);

}

void GuiMenu::openConfigInput()
{
	Window* window = mWindow;
	window->pushGui(new GuiMsgBox(window, "ESTAS SEGURO DE QUE QUIERES CONFIGURAR UN INPUT (MANDO)", "SI",
		[window] {
		window->pushGui(new GuiDetectDevice(window, false, nullptr));
	}, "NO", nullptr)
	);

}

void GuiMenu::openQuitMenu()
{
	auto s = new GuiSettings(mWindow, "SALIR");

	Window* window = mWindow;

	ComponentListRow row;
	if (UIModeController::getInstance()->isUIModeFull())
	{
		row.makeAcceptInputHandler([window] {
			window->pushGui(new GuiMsgBox(window, "REINICIAR?", "SI",
				[] {
				Scripting::fireEvent("quit");
				if(quitES(QuitMode::RESTART) != 0)
					LOG(LogWarning) << "Restart terminated with non-zero result!";
			}, "NO", nullptr));
		});
		row.addElement(std::make_shared<TextComponent>(window, "REINICIAR EMULATIONSTATION", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
		s->addRow(row);



		if(Settings::getInstance()->getBool("ShowExit"))
		{
			row.elements.clear();
			row.makeAcceptInputHandler([window] {
				window->pushGui(new GuiMsgBox(window, "SALIR?", "SI",
					[] {
					Scripting::fireEvent("quit");
					quitES();
				}, "NO", nullptr));
			});
			row.addElement(std::make_shared<TextComponent>(window, "SALIR DE EMULATIONSTATION", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
			s->addRow(row);
		}
	}
	row.elements.clear();
	row.makeAcceptInputHandler([window] {
		window->pushGui(new GuiMsgBox(window, "REINICIAR?", "SI",
			[] {
			Scripting::fireEvent("quit", "reboot");
			Scripting::fireEvent("reboot");
			if (quitES(QuitMode::REBOOT) != 0)
				LOG(LogWarning) << "Restart terminated with non-zero result!";
		}, "NO", nullptr));
	});
	row.addElement(std::make_shared<TextComponent>(window, "REINICIAR EMULOS", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
	s->addRow(row);

	row.elements.clear();
	row.makeAcceptInputHandler([window] {
		window->pushGui(new GuiMsgBox(window, "APAGAR?", "SI",
			[] {
			Scripting::fireEvent("quit", "shutdown");
			Scripting::fireEvent("shutdown");
			if (quitES(QuitMode::SHUTDOWN) != 0)
				LOG(LogWarning) << "Shutdown terminated with non-zero result!";
		}, "NO", nullptr));
	});
	row.addElement(std::make_shared<TextComponent>(window, "APAGAR EMULOS", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
	s->addRow(row);

	mWindow->pushGui(s);
}

void GuiMenu::addVersionInfo()
{
	std::string  buildDate = (Settings::getInstance()->getBool("Debug") ? std::string( "   (" + Utils::String::toUpper(PROGRAM_BUILT_STRING) + ")") : (""));

	mVersion.setFont(Font::get(FONT_SIZE_SMALL));
	mVersion.setColor(0x5E5E5EFF);
	mVersion.setText("EMULATIONSTATION V" + Utils::String::toUpper(PROGRAM_VERSION_STRING) + buildDate);
	mVersion.setHorizontalAlignment(ALIGN_CENTER);
	addChild(&mVersion);
}

void GuiMenu::openScreensaverOptions() {
	mWindow->pushGui(new GuiGeneralScreensaverOptions(mWindow, "AJUSTES DE SCREENSAVER"));
}

void GuiMenu::openCollectionSystemSettings() {
	mWindow->pushGui(new GuiCollectionSystemsOptions(mWindow));
}

void GuiMenu::onSizeChanged()
{
	mVersion.setSize(mSize.x(), 0);
	mVersion.setPosition(0, mSize.y() - mVersion.getSize().y());
}

void GuiMenu::addEntry(const char* name, unsigned int color, bool add_arrow, const std::function<void()>& func)
{
	std::shared_ptr<Font> font = Font::get(FONT_SIZE_MEDIUM);

	// populate the list
	ComponentListRow row;
	row.addElement(std::make_shared<TextComponent>(mWindow, name, font, color), true);

	if(add_arrow)
	{
		std::shared_ptr<ImageComponent> bracket = makeArrow(mWindow);
		row.addElement(bracket, false);
	}

	row.makeAcceptInputHandler(func);

	mMenu.addRow(row);
}

bool GuiMenu::input(InputConfig* config, Input input)
{
	if(GuiComponent::input(config, input))
		return true;

	if((config->isMappedTo("b", input) || config->isMappedTo("start", input)) && input.value != 0)
	{
		delete this;
		return true;
	}

	return false;
}

HelpStyle GuiMenu::getHelpStyle()
{
	HelpStyle style = HelpStyle();
	style.applyTheme(ViewController::get()->getState().getSystem()->getTheme(), "system");
	return style;
}

std::vector<HelpPrompt> GuiMenu::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;
	prompts.push_back(HelpPrompt("up/down", "escoger"));
	prompts.push_back(HelpPrompt("a", "seleccionar"));
	prompts.push_back(HelpPrompt("start", "cerrar"));
	return prompts;
}
