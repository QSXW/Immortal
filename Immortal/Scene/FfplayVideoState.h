/**
 * Copyright (C) 2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * fftools/ffplay.c VideoState — packet queues, clocks, and sync fields used by VideoPlayerContext.
 */
#pragma once

#include "Vision/CodedFrame.h"
#include "Vision/Picture.h"
#include "Vision/Types.h"

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cmath>
#include <cstdint>
#include <deque>
#include <mutex>
#include <utility>

namespace Immortal
{

enum class FfplayAvSyncType
{
	VideoMaster,
	AudioMaster,
	ExternalClock,
};

struct FfplayClock
{
	double             pts         = NAN;
	double             ptsDrift    = 0.0;
	double             lastUpdated = 0.0;
	double             speed       = 1.0;
	int                serial      = -1;
	std::atomic<int>  *queueSerial = nullptr;
	bool               paused      = false;
};

struct FfplayPacketEntry
{
	Vision::CodedFrame pkt;
	int                tagSerial = 0;
	size_t             byteSize  = 0;

	FfplayPacketEntry() = default;
	FfplayPacketEntry(Vision::CodedFrame &&p, int t, size_t b) :
	    pkt(std::move(p)), tagSerial(t), byteSize(b) {}
};

struct FfplayPacketQueue
{
	mutable std::mutex             mutex;
	std::condition_variable        cond;
	std::deque<FfplayPacketEntry>  packets;
	std::atomic<bool>              abortRequest{ false };
	std::atomic<int>               serial{ 0 };
	int                            nbPackets = 0;
	size_t                         sizeBytes  = 0;

	void Start()
	{
		abortRequest.store(false);
	}

	void Abort()
	{
		{
			std::lock_guard<std::mutex> lk(mutex);
			abortRequest.store(true);
		}
		cond.notify_all();
	}

	void Flush()
	{
		std::lock_guard<std::mutex> lk(mutex);
		packets.clear();
		nbPackets = 0;
		sizeBytes = 0;
		serial.fetch_add(1);
		cond.notify_all();
	}

	int Put(Vision::CodedFrame &&pkt, int maxPackets, size_t maxBytes)
	{
		const size_t bc = std::max(size_t(1), pkt.GetSize());
		std::unique_lock<std::mutex> lk(mutex);
		const int tag = serial.load(std::memory_order_relaxed);
		while (!abortRequest.load()
		    && tag == serial.load(std::memory_order_relaxed)
		    && !(nbPackets < maxPackets && sizeBytes + bc <= maxBytes))
		{
			cond.wait_for(lk, std::chrono::milliseconds(10));
		}
		if (abortRequest.load() || tag != serial.load(std::memory_order_relaxed))
			return -1;
		packets.emplace_back(std::move(pkt), tag, bc);
		nbPackets++;
		sizeBytes += bc;
		lk.unlock();
		cond.notify_all();
		return 0;
	}

	int Get(Vision::CodedFrame &out, int *tagOut, bool block)
	{
		std::unique_lock<std::mutex> lk(mutex);
		for (;;)
		{
			if (abortRequest.load())
				return -1;
			if (!packets.empty())
			{
				auto &e = packets.front();
				out = std::move(e.pkt);
				if (tagOut)
					*tagOut = e.tagSerial;
				sizeBytes -= e.byteSize;
				packets.pop_front();
				nbPackets--;
				lk.unlock();
				cond.notify_all();
				return 1;
			}
			if (!block)
				return 0;
			cond.wait(lk);
		}
	}

	size_t PacketCount() const
	{
		std::lock_guard<std::mutex> lk(mutex);
		return packets.size();
	}

	int SerialSnapshot() const
	{
		return serial.load(std::memory_order_relaxed);
	}
};

struct AudioFrameSlot
{
	Picture picture;
	int     packetSerial = 0;
};

struct FfplayVideoState
{
	FfplayPacketQueue videoq;
	FfplayPacketQueue audioq;

	FfplayClock audclk{};
	FfplayClock vidclk{};
	FfplayClock extclk{};

	FfplayAvSyncType avSyncType = FfplayAvSyncType::AudioMaster;

	int framedrop = -1;

	double maxFrameDuration = 10.0;

	double audioClock       = NAN;
	int    audioClockSerial = -1;
	int    audioHwBufSize   = 0;

	double audioDiffCum       = 0.0;
	double audioDiffAvgCoef   = 0.0;
	int    audioDiffAvgCount  = 0;
	double audioDiffThreshold = 0.0;

	int    frameDropsEarly = 0;
	int    frameDropsLate  = 0;

	std::atomic_bool eof{ false };
	std::atomic_bool pause{ false };

	std::atomic_bool seekReq{ false };
	MediaType        seekStreamType = MediaType::Video;
	int64_t          seekPos        = 0;
	int64_t          seekMin        = 0;
	int64_t          seekMax        = 0;
};

} // namespace Immortal
