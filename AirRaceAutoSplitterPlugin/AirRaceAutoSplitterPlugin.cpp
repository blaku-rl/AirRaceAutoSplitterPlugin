#include "pch.h"
#include "AirRaceAutoSplitterPlugin.h"


BAKKESMOD_PLUGIN(AirRaceAutoSplitterPlugin, "Air Race Auto-Splitter Plugin", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void AirRaceAutoSplitterPlugin::onLoad()
{
	_globalCvarManager = cvarManager;

	cvarManager->registerCvar("airraceautosplit_enabled", "0", "True/False value for if the plugin is enabled", true, true, 0.0f, true, 1.0f, true)
		.addOnValueChanged(bind(&AirRaceAutoSplitterPlugin::ToggleEnabled, this, std::placeholders::_1, std::placeholders::_2));
	cvarManager->registerNotifier("airraceautosplit_toggle_enabled", [this](std::vector<std::string> params) {
		this->ToggleEnabledEasy();
		}, "Switch the plugin on and off", PERMISSION_FREEPLAY);

	gameWrapper->HookEvent("Function TAGame.Mutator_Freeplay_TA.Init", bind(&AirRaceAutoSplitterPlugin::OnMapLoad, this, std::placeholders::_1));

	standardCheckpoints = {
		{1250.0f, Vector(12115, -24835, 0)},
		{1300.0f, Vector(7524.62, -15383.5, 0)},
		{715.0f, Vector(1547.62, 224.58, 0)},
		{985.0f, Vector(-4321.41, -11342.6, 0)},
		{985.0f, Vector(-6567.86, 3832.21, 0)},
		{1135.0f, Vector(11130.2, 4290.75, 0)},
		{930.0f, Vector(16478.7, 9589.6, 0)},
		{955.0f, Vector(13106.6, 15250.4, 0)},
		{1250.0f, Vector(9792.97, 21889, 0)},
		{1310.0f, Vector(-1922.99, 13825.6, 0)},
		{1205.0f, Vector(-13733.4, 7022.4, 58.599)}
	};

	isEnabled = cvarManager->getCvar("airraceautosplit_enabled").getBoolValue();
}

void AirRaceAutoSplitterPlugin::onUnload()
{
}

void AirRaceAutoSplitterPlugin::ToggleEnabled(std::string oldValue, CVarWrapper newValue)
{
	isEnabled = newValue.getBoolValue();
	InitializeSplits();
}

void AirRaceAutoSplitterPlugin::ToggleEnabledEasy()
{
	if (IsPanicsAirRace() && !gameWrapper->IsPaused())
	{
		std::string enabled = !cvarManager->getCvar("airraceautosplit_enabled").getBoolValue() ? "1" : "0";
		cvarManager->executeCommand("airraceautosplit_enabled " + enabled);
	}
}

bool AirRaceAutoSplitterPlugin::IsPanicsAirRace()
{
	std::string mapName = gameWrapper->GetCurrentMap();
	for (int i = 0; i < mapName.length(); i++) {
		mapName[i] = std::tolower(mapName[i]);
	}
	return mapName == "airracetest" || mapName == "panicsairrace";
}

void AirRaceAutoSplitterPlugin::InitializeSplits()
{
	if (!IsPanicsAirRace()) { return; }

	if (isEnabled)
	{
		counter = 0;
		cvarManager->executeCommand("speedrun_livesplit_connect");
		this->gameWrapper->HookEvent("Function TAGame.Car_TA.SetVehicleInput", std::bind(&AirRaceAutoSplitterPlugin::CheckRunStart, this, std::placeholders::_1));
		this->gameWrapper->HookEvent("Function GameEvent_Soccar_TA.Active.StartRound", bind(&AirRaceAutoSplitterPlugin::CheckGameReset, this, std::placeholders::_1));
		this->gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.Destroyed", std::bind(&AirRaceAutoSplitterPlugin::OnMapUnload, this, std::placeholders::_1));
		this->SetSpawn();
		Log("Auto Split Enabled", true);
	}
	else
	{
		Log("Auto Split Disabled", true);
	}
}

void AirRaceAutoSplitterPlugin::OnMapLoad(std::string eventName)
{
	gameWrapper->SetTimeout([this](GameWrapper* gw) {
		this->InitializeSplits();
		}, 4.5f);
}

void AirRaceAutoSplitterPlugin::OnMapUnload(std::string eventName)
{
	cvarManager->executeCommand("speedrun_livesplit_reset");
	gameWrapper->UnhookEvent("Function TAGame.Car_TA.SetVehicleInput");
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.Destroyed");
	gameWrapper->UnhookEvent("Function GameEvent_Soccar_TA.Active.StartRound");
	gameWrapper->UnhookEvent("Function GameEvent_Soccar_TA.Active.StartRound");
}

void AirRaceAutoSplitterPlugin::CheckGameReset(std::string eventName)
{
	counter = 0;
	cvarManager->executeCommand("speedrun_livesplit_reset");
	gameWrapper->UnhookEvent("Function TAGame.Car_TA.SetVehicleInput");
	gameWrapper->UnhookEvent("Function Engine.PlayerController.ClientDrawKismetText");
	gameWrapper->HookEvent("Function TAGame.Car_TA.SetVehicleInput", std::bind(&AirRaceAutoSplitterPlugin::CheckRunStart, this, std::placeholders::_1));
	gameWrapper->SetTimeout([this](GameWrapper* gw) {
		this->SetSpawn();
		}, 0.1f);
}

void AirRaceAutoSplitterPlugin::CheckRunStart(std::string eventName)
{
	if (!isEnabled)
	{
		OnMapUnload("");
		return;
	}

	Vector car = gameWrapper->GetLocalCar().GetLocation();

	if (car.X < -13090 && car.X > -13120 && car.Y < -26835 && car.Y > -27207 && car.Z > 55 && car.Z < 380)
	{
		cvarManager->executeCommand("speedrun_livesplit_start");
		gameWrapper->UnhookEvent("Function TAGame.Car_TA.SetVehicleInput");
		gameWrapper->HookEvent("Function TAGame.Car_TA.SetVehicleInput", std::bind(&AirRaceAutoSplitterPlugin::CheckFirstSplit, this, std::placeholders::_1));
	}
}

void AirRaceAutoSplitterPlugin::CheckFirstSplit(std::string eventName)
{
	if (!isEnabled)
	{
		OnMapUnload("");
		return;
	}

	Vector car = gameWrapper->GetLocalCar().GetLocation();

	if (car.Z < 1170.0f) { return; }

	Vector centerFirstCheck = { -8993.48, -26986.1, 0 };
	Vector repositionedCar = { car.X, car.Y, 0 };
	auto diff = centerFirstCheck - repositionedCar;

	if (diff.magnitude() <= 1420.45)
	{
		cvarManager->executeCommand("speedrun_livesplit_split");
		gameWrapper->UnhookEvent("Function TAGame.Car_TA.SetVehicleInput");
		gameWrapper->HookEvent("Function TAGame.Car_TA.SetVehicleInput", std::bind(&AirRaceAutoSplitterPlugin::CheckRegularSplit, this, std::placeholders::_1));
	}
}

void AirRaceAutoSplitterPlugin::CheckRegularSplit(std::string eventName)
{
	if (!isEnabled)
	{
		OnMapUnload("");
		return;
	}

	Vector car = gameWrapper->GetLocalCar().GetLocation();

	Vector diff = Vector(car.X, car.Y, 0) - standardCheckpoints[counter].midpoint;

	if (diff.magnitude() < standardCheckpoints[counter].distance)
	{
		cvarManager->executeCommand("speedrun_livesplit_split");
		counter++;
		if (counter >= standardCheckpoints.size()) {
			gameWrapper->UnhookEvent("Function TAGame.Car_TA.SetVehicleInput");
			gameWrapper->HookEvent("Function TAGame.Car_TA.SetVehicleInput", std::bind(&AirRaceAutoSplitterPlugin::CheckLastSplit, this, std::placeholders::_1));
		}
	}
}

void AirRaceAutoSplitterPlugin::CheckLastSplit(std::string eventName)
{
	if (!isEnabled)
	{
		OnMapUnload("");
		return;
	}

	Vector car = gameWrapper->GetLocalCar().GetLocation();

	if (car.X < -4307.57 && car.X > -4525 && car.Y < -39833 && car.Y > -40838.5 && car.Z > 848.66 && car.Z < 1849.65)
	{
		cvarManager->executeCommand("speedrun_livesplit_split");
		gameWrapper->UnhookEvent("Function TAGame.Car_TA.SetVehicleInput");
	}
}

void AirRaceAutoSplitterPlugin::SetSpawn()
{
	if (gameWrapper->GetLocalCar().IsNull()) { return; }

	gameWrapper->GetLocalCar().SetLocation(spawnVector);
	gameWrapper->GetLocalCar().SetRotation(spawnRotator);
}

void AirRaceAutoSplitterPlugin::Log(std::string message, bool sendToChat)
{
	cvarManager->log(message);
	if (sendToChat && IsPanicsAirRace())
	{
		gameWrapper->LogToChatbox(message, "Auto Split");
	}
}