#include <node.h>
#include <v8.h>
#include <memory>
#include "node.h"
#include "node_buffer.h"

#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <windows.h>

using namespace v8;

using node::AtExit;

namespace {
	std::string buffer;

	typedef std::map<std::string, HANDLE> HandleMap;
	HandleMap handle_map;

	typedef std::map<std::string, LPSTR> ViewMap;
	ViewMap view_map;
}

static void dispose(void*)
{
	for (ViewMap::iterator it = view_map.begin(); it != view_map.end(); ++it)
	{
		UnmapViewOfFile(it->second);
	}
	for (HandleMap::iterator it = handle_map.begin(); it != handle_map.end(); ++it)
	{
		CloseHandle(it->second);
	}
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
	buffer = "";

	if (memorysize > 0) {
		buffer.resize(memorysize);
		const int count = (memorysize) / 1024;
		const int last = (memorysize)-count * 1024;
		for (int i = 0; i < count; ++i) {
			memcpy(&buffer[1024 * i], &mapview[1024 * i + 4], 1024);
		}
		memcpy(&buffer[1024 * count], &mapview[1024 * count + 4], last);

		Local<ArrayBuffer> array_buffer = v8::ArrayBuffer::New(isolate, const_cast<char*>(buffer.c_str()), memorysize);
		args.GetReturnValue().Set(array_buffer);
	}

	UnmapViewOfFile(mapview);
	CloseHandle(hmap);
}

static void save(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = Isolate::GetCurrent();
	if (args.Length() < 2) {
		isolate->ThrowException(Exception::TypeError(
			String::NewFromUtf8(isolate, "Wrong number of arguments")));
		return;
	}
	if (!args[0]->IsString()) {
		isolate->ThrowException(Exception::TypeError(
			String::NewFromUtf8(isolate, "Wrong arguments")));
		return;
	}
	if (!args[1]->IsArrayBuffer()) {
		isolate->ThrowException(Exception::TypeError(
			String::NewFromUtf8(isolate, "Wrong arguments")));
		return;
	}

	Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
	ArrayBuffer::Contents contents = buffer->GetContents();

	v8::String::Utf8Value keyStr(args[0]->ToString());
	const std::string key = *keyStr;
	const int memorysize = contents.ByteLength();

	if (handle_map.find(key) == handle_map.end()) {
		handle_map[key] = CreateFileMappingA((HANDLE)-1, NULL, PAGE_READWRITE, 0,memorysize + 4, key.c_str());
	}
	if (view_map.find(key) == view_map.end()) {
		view_map[key] =  (LPSTR)MapViewOfFile(handle_map[key], FILE_MAP_ALL_ACCESS, 0, 0, 0);
	}
	memcpy(view_map[key], &memorysize, 4);
	memcpy(view_map[key] + 4, contents.Data(), contents.ByteLength());

	args.GetReturnValue().Set(true);
}

void Init(Handle<Object> exports) {
	AtExit(dispose);
	NODE_SET_METHOD(exports, "load", load);
	NODE_SET_METHOD(exports, "save", save);

}
NODE_MODULE(umnode, Init)
