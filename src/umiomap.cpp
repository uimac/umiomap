#include <node.h>
#include <v8.h>
#include <memory>
#include "node.h"
#include "node_buffer.h"

#include <vector>
#include <string>
#include <algorithm>
#include <windows.h>
#include "UMIO.h"
#include "UMObject.h"

using namespace v8;

using node::AtExit;

static void dispose(void*)
{
}

static void load(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = Isolate::GetCurrent();
	if (args.Length() < 1) {
		isolate->ThrowException(Exception::TypeError(
			String::NewFromUtf8(isolate, "Wrong number of arguments")));
		return;
	}
	if (!args[0]->IsString()) {
		isolate->ThrowException(Exception::TypeError(
			String::NewFromUtf8(isolate, "Wrong arguments")));
		return;
	}
	v8::String::Utf8Value keyStr(args[0]->ToString());
	const std::string key = *keyStr;

	HANDLE hmap = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, key.c_str());
	DWORD error = GetLastError();
	if (error != 0) {
		args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, key.c_str()));
		return;
	}
	char* mapview = reinterpret_cast<char*>(MapViewOfFile(hmap, FILE_MAP_ALL_ACCESS, 0, 0, 0));
	if (mapview == NULL) {
		args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, "mapview1"));
		return;
	}
	int memorysize = 0;
	memcpy(&memorysize, mapview, 4);

	if (memorysize > 0) {
		std::string buffer;
		buffer.resize(memorysize);
		const int count = (memorysize) / 1024;
		const int last = (memorysize)-count * 1024;
		for (int i = 0; i < count; ++i) {
			memcpy(&buffer[1024 * i], &mapview[1024 * i + 4], 1024);
		}
		memcpy(&buffer[1024 * count], &mapview[1024 * count + 4], last);
		const int hoge = 1024 * count + last;

		umio::UMIO io;
		umio::UMIOSetting setting;
		umio::UMObjectPtr obj = io.load_from_memory(buffer, setting);
		if (obj) {
			const umio::UMSkeleton::IDToSkeletonMap& skeletonMap = obj->skeleton_map();
			std::string skeletons;
			for (auto it = skeletonMap.begin(); it != skeletonMap.end(); ++it) {
				printf("hogehoge %s\n", it->second.name().c_str());
				skeletons += it->second.name();
			}
			args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, skeletons.c_str()));
		}
	}

	UnmapViewOfFile(mapview);
	CloseHandle(hmap);
}

void Init(Handle<Object> exports) {
	AtExit(dispose);
	NODE_SET_METHOD(exports, "load", load);

}
NODE_MODULE(umnode, Init)
