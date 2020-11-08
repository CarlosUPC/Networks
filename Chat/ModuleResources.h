#pragma once

#define USE_TASK_MANAGER

struct Texture;

class ModuleResources : public Module
{
public:

	Texture *background = nullptr;
	Texture *banner = nullptr;
	Texture *client = nullptr;
	Texture *server = nullptr;

	Texture* sadpeepo = nullptr;
	Texture* hypepeepo = nullptr;
	Texture* crosspeepo = nullptr;
	Texture* laughpeepo = nullptr;
	Texture* crunchpeepo = nullptr;
	Texture* ezpeepo = nullptr;
	Texture* monkaspeepo = nullptr;
	Texture* tsmpeepo = nullptr;
	Texture* clownhpeepo = nullptr;

	bool finishedLoading = false;
private:

	bool init() override;

#if defined(USE_TASK_MANAGER)
	void onTaskFinished(Task *task) override;

	void loadTextureAsync(const char *filename, Texture **texturePtrAddress);
#endif

	struct LoadTextureResult {
		Texture **texturePtr = nullptr;
		Task *task = nullptr;
	};

	LoadTextureResult taskResults[1024] = {};
	int taskCount = 0;
	int finishedTaskCount = 0;
};

