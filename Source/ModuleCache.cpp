
#include "ModuleCache.h"

std::unordered_map<std::string, ModuleData> ModuleCache::moduleData;

void ModuleCache::Init() {
	moduleData.clear();
	CacheBaseModulesData();
}

void ModuleCache::CacheBaseModulesData() {

	CacheModuleData("Output",			MOD_OUTPUT);
	CacheModuleData("Noise",			MOD_NOISE);
	CacheModuleData("Oscillator",		MOD_OSCILLATOR);
	CacheModuleData("LFO",				MOD_LFO);
	CacheModuleData("Scope",			MOD_SCOPE);
	CacheModuleData("Filter",			MOD_FILTER);
	CacheModuleData("ADSR",				MOD_ADSR);
	CacheModuleData("Sequencer",		MOD_SEQUENCER);
	CacheModuleData("Chord Sequencer",	MOD_CHORD_SEQUENCER);
	CacheModuleData("VCA",				MOD_VCA);
	CacheModuleData("Reverb",			MOD_REVERB);
	CacheModuleData("Clock Divide",		MOD_CLOCK_DIVIDE);
	CacheModuleData("Quantizer",		MOD_VOLT_QUANTIZER);
	CacheModuleData("Voltages", 		MOD_VOLT_KNOBS);
	CacheModuleData("Mixer",			MOD_MIXER);
}

void ModuleCache::CacheModuleData(const std::string& moduleKey, ModuleType moduleType) {

	moduleData[moduleKey] = ModuleData();
	moduleData[moduleKey].name = moduleKey;
	moduleData[moduleKey].moduleType = moduleType;
}

const ModuleData& ModuleCache::GetModuleData(const std::string& moduleKey) {

	return moduleData[moduleKey];
}

const std::unordered_map<std::string, ModuleData>& ModuleCache::GetAllModuleData() {
	return moduleData;
}