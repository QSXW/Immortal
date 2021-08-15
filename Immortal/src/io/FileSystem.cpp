#include "impch.h"
#include "FileSystem.h"

namespace Immortal
{
	void Work::Submit(Loop * loop, Type type, WorkCaller work, DoneCaller done)
	{
		this->loop = loop;
		this->work = work;
		this->done = done;
	}

	void FileSystem::CleanUp()
	{
		if (path != nullptr &&
			(callback != nullptr ||
				type == Type::MKDTEMP || type == Type::MKSTEMP))
		{
			free((void*)path); /* Memory is shared with req->new_path. */
		}

		path = nullptr;
		
		if (type == Type::READDIR && ptr != nullptr)
		{
			ReadDirectoryCleanUp();
		}
		if (type == Type::SCANDIR && ptr != nullptr)
		{
			ScanDirectoryCleanUp();
		}

		if (fs.info.bufs != fs.info.bufsml)
		{
			free(fs.info.bufs);
		}
		fs.info.bufs = nullptr;

		if (type != Type::OPENDIR && ptr != (void*)&stat)
		{
			free(ptr);
		}
		ptr = nullptr;
	}

	void FileSystem::ReadDirectoryCleanUp()
	{
		Directory *dir;
		Dirent *dirents;
		
		if (ptr == nullptr)
		{
			return;
		}

		dir = reinterpret_cast<Directory*>(ptr);
		dirents = dir->Dirents;
		ptr = nullptr;

		if (dirents == nullptr)
		{
			return;
		}

		for (INT32 i = 0; i < result; i++)
		{
			free((char*)dirents[i].name);
			dirents[i].name = nullptr;
		}
	}

	void FileSystem::ScanDirectoryCleanUp()
	{
		Dirent **dents;
		UINT32 nbufs = fs.info.nbufs;

		dents = reinterpret_cast<Dirent**>(ptr);
		if (nbufs > 0 && nbufs != static_cast<INT32>(result))
		{
			nbufs--;
		}
		for (; nbufs < static_cast<INT32>(result); nbufs++)
		{
			free(dents[nbufs]);
		}
		free(ptr);
		ptr = nullptr;
	}

	int FileSystem::Close(Loop *loop, File file, Callback callback)
	{
		return 0;
	}

	void FileSystem::Open()
	{
		int flags = this->fs.info.file_flags;

		if (flags & static_cast<int>(OpenFlag::FILEMAP))
		{

		}
	}

	int FileSystem::Open(Loop *loop, const char *path, int flags, int mode, Callback callback)
	{
		int error;

		this->callback = callback;

		this->flags = flags;
		this->fs.info.mode = mode;

		if (callback != ((void *)0))
		{ 
			(loop)->ActiveRequests.Count++;
			this->work_req.Submit(loop, Work::Type::FAST_IO, WorkFunc, DoneFunc);
			return 0;
		}
		else
		{ 
			WorkFunc(&this->work_req);
			return result;
		}
	}

	void FileSystem::WorkFunc(Work * w)
	{
		FileSystem *req = reinterpret_cast<FileSystem*>((((INT8 *)(w)-((size_t)&(((FileSystem*)0)->work_req)))));
		SLASSERT((dynamic_cast<Request *>(req))->type == Request::Type::FS, "");

#define XX(uc, lc)  case FileSystem::Type::##uc: ##req->lc(); break;
		switch (req->type) {
				XX(OPEN, Open)
		default:
			assert(!"bad File System Type");
		}
	}

	void FileSystem::DoneFunc(Work * w, int status)
	{

	}
}


