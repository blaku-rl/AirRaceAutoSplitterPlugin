#pragma once
#pragma comment( lib, "bakkesmod.lib" )
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/wrappers/kismet/SequenceWrapper.h"
#include <bakkesmod/wrappers/kismet/SequenceVariableWrapper.h>
#include "Checkpoink.h"

class AirRaceAutoSplitterPlugin : public BakkesMod::Plugin::BakkesModPlugin {
public:
	void onLoad();
	void onUnload();
private:
	std::vector<Checkpoint> standardCheckpoints;
	int counter = 0;
	bool isEnabled = false;
	Vector spawnVector = Vector(-14357.8, -27018.7, 55.46);
	Rotator spawnRotator = Rotator(100, 128, 0);
	void ToggleEnabled(std::string oldValue, CVarWrapper newValue);
	void ToggleEnabledEasy();
	bool IsPanicsAirRace();
	void InitializeSplits();
	void OnMapLoad(std::string eventName);
	void OnMapUnload(std::string eventName);
	void CheckGameReset(std::string eventName);
	void CheckRunStart(std::string eventName);
	void CheckFirstSplit(std::string eventName);
	void CheckRegularSplit(std::string eventName);
	void CheckLastSplit(std::string eventName);
	void SetSpawn();
	void Log(std::string message, bool sendToChat);
};