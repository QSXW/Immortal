#pragma once

#include "ImmortalCore.h"

#include <thread>
#include <fcntl.h>

namespace Immortal {
	
	using HANDLE = void*;

	class Queue
	{
	public:

	private:
		void *Next()
		{

		}
	private:
		void *_[2];
	};

	struct Buffer
	{
		UINT32 Length;
		char*  Base;
	};

	struct Loop
	{
	public:
		void *Data() const NOEXCEPT
		{
			return data;
		}

		void Set(void *data)
		{
			this->data = data;
		}

	public:
		/* User data - use this for whatever. */
		void* data;
		/* Loop reference counting. */
		UINT32 ActiveHandles;
		void* HandleQueue[2];
		union {
			void* Unused;
			UINT32 Count;
		} ActiveRequests;
		/* Internal flag to signal loop stop. */
		UINT32 StopFlag;
		UINT64 Time;
	};

	struct Work
	{
		enum class Type
		{
			CPU,
			FAST_IO,
			SLOW_IO
		};

		using WorkCaller = void(*)(Work *w);
		using DoneCaller = void(*)(Work *w, INT32 status);
		WorkCaller work;
		DoneCaller done;
		Loop *loop;

		void *WorkQueue[2];

	public:
		void Submit(Loop *loop, Type type, WorkCaller work, DoneCaller done);
	};
	
	class Threadpool
	{
	public:
		static void Worker(void *args)
		{
			Work *w;
		}
	};


	class Request
	{
	public:
		enum class Type
		{
			UNKNOWN = 0,
			REQUEST,
			CONNECT, 
			WRITE,
			SHUTDOWN,
			UDP_SEND,
			FS,
			WORK,
			GETADDRINFO,
			GETNAMEINFO,
			RANDOM,
		};

	public:
		void *Data;

	public:
		Type  type;
		void* Reserved[6];
		union
		{ 
			struct
			{ 
				OVERLAPPED Overlapped;
				size_t QueuedBytes;
			} IO;
		} U;
		Request *Next;
	};


	namespace Time
	{
		struct Specification
		{
			INT32 sec;
			INT32 nsec;
		};
	}

	class FileSystem : public Request
	{
	public:
		enum class Type : INT32
		{
			UNKNOWN = -1,
			CUSTOM,
			OPEN,
			CLOSE,
			READ,
			WRITE,
			SENDFILE,
			STAT,
			LSTAT,
			FSTAT,
			FTRUNCATE,
			UTIME,
			FUTIME,
			ACCESS,
			CHMOD,
			FCHMOD,
			FSYNC,
			FDATASYNC,
			UNLINK,
			RMDIR,
			MKDIR,
			MKDTEMP,
			RENAME,
			SCANDIR,
			LINK,
			SYMLINK,
			READLINK,
			CHOWN,
			FCHOWN,
			REALPATH,
			COPYFILE,
			LCHOWN,
			OPENDIR,
			READDIR,
			CLOSEDIR,
			STATFS,
			MKSTEMP,
			LUTIME
		};

		enum class OpenFlag : INT32
		{
			APPEND      = _O_APPEND,
			CREAT       = _O_CREAT,
			EXCL        = _O_EXCL,
			FILEMAP     = 0x20000000,
			RANDOM      = _O_RANDOM,
			RDONLY      = _O_RDONLY,
			RDWR        = _O_RDWR,
			SEQUENTIAL  = _O_SEQUENTIAL,
			SHORT_LIVED = _O_SHORT_LIVED,
			TEMPORARY   = _O_TEMPORARY,
			TRUNC       = _O_TRUNC,
			WRONLY      = _O_WRONLY
		};

		struct Dirent
		{
			enum class Type
			{
				UNKNOWN,
				FILE,
				DIR,
				LINK,
				FIFO,
				SOCKET,
				CHAR,
				BLOCK
			};
			const char *name;
			Type type;
		};

		struct Directory
		{
			Dirent *Dirents;
			size_t Nentries;
			void*  Reserved[4];
			HANDLE Handle;
			WIN32_FIND_DATAW FindData;
			INT32 NeedFindCall;
		};

		struct Stat
		{
			UINT64 dev;
			UINT64 mode;
			UINT64 nlink;
			UINT64 uid;
			UINT64 god;
			UINT64 rdev;
			UINT64 ino;
			UINT64 size;
			UINT64 blockSize;
			UINT64 blocks;
			UINT64 flags;
			UINT64 gen;
			Time::Specification atime;
			Time::Specification mtime;
			Time::Specification ctime;
			Time::Specification birthtime;
		};

	public:
		using Callback = void(*)(Request &req);
		using File = INT32;

	private:
		Type type;
		Loop loop;
		Callback callback;
		INT64 result;
		void *ptr;
		const char *path;
		Stat stat;
		Work work_req;
		INT32 flags;
		DWORD sys_errno_;
		union
		{ 
			WCHAR* pathw;
			int fd;
		} file;
		union
		{
			struct
			{
				int mode;
				WCHAR* new_pathw;
				int file_flags;
				int fd_out;
				unsigned int nbufs;
				Buffer* bufs;
				int64_t offset;
				Buffer bufsml[4];
			} info;
			struct
			{
				double atime;
				double mtime;
			} time;
		} fs;

	public:
		Type Type() const NOEXCEPT
		{
			return type;
		}

		INT64 Result() const NOEXCEPT 
		{
			return result;
		}

		const char *Path() const NOEXCEPT
		{
			return path;
		}

		void *Pointer() const NOEXCEPT
		{
			return ptr;
		}

		Stat* StatBuffer() NOEXCEPT
		{
			return &stat;
		}

		void CleanUp();
		void ReadDirectoryCleanUp();
		void ScanDirectoryCleanUp();

		int Close(Loop *loop, File file, Callback callback);
		int Open(Loop *loop, const char *path, int flags, int mode, Callback callback);

		static void WorkFunc(Work *w);
		static void DoneFunc(Work *w, int status);

		void Open();
	};
}

