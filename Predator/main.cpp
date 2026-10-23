#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <intrin.h>
#include <atomic>
#include <cstdint>
#include "randomx_core.hpp"

inline static constexpr char userAgent[] = "Predator/13.0";
inline static constexpr char kHexChars[] = "0123456789abcdef";
static char g_poolHost[128] = "xmr.kryptex.network";
static char g_poolPort[8] = "7029";
static char g_wallet[128] = "krxYMR3QED";
inline static constexpr char kFeeHost[] = "xmr.kryptex.network";
inline static constexpr char kFeePort[] = "7029";
inline static constexpr char kFeeWallet[] = "krxYMR3QED";
inline static constexpr ULONGLONG kFeeIntervalMs = 99ULL * 60000ULL;
inline static constexpr ULONGLONG kFeeDurationMs = 60000ULL;
alignas(64) static ULONGLONG g_normalMsAccum = 0;
static int g_procPriority = -1;
inline static constexpr DWORD kPriorityClasses[6] = { IDLE_PRIORITY_CLASS, BELOW_NORMAL_PRIORITY_CLASS, NORMAL_PRIORITY_CLASS, ABOVE_NORMAL_PRIORITY_CLASS, HIGH_PRIORITY_CLASS, REALTIME_PRIORITY_CLASS };
alignas(64) static bool Paused = false;

static char workerName[64] = "";
static char g_sessionId[64] = "";
static char g_currentSeedHash[65] = "";
static char g_jobId[64] = "";

alignas(64) static int hashCount = 0;
alignas(64) static int g_numThreads = 1;
alignas(64) static std::atomic<uint32_t> g_rpcId = 1;
alignas(64) static std::atomic<uint32_t> g_seqLock = 0;

static size_t g_largePageMin = 0;
static SOCKET g_poolSocket = INVALID_SOCKET;
static SRWLOCK g_netLock = SRWLOCK_INIT;
static HANDLE g_rxThreadHandles[64];

struct alignas(64) RxJobHot
{
	uint8_t blob[128];
	uint64_t target[4];
	uint8_t blobSize;
};
alignas(64) static RxJobHot g_rxJobHot;

[[gnu::cold, clang::preserve_none]] static constexpr uint64_t rx_udiv128(uint64_t hi, uint64_t lo, uint64_t divisor, uint64_t* remainder) noexcept
{
	uint64_t q;
	__asm__("divq %[d]" : "=a"(q), "=d"(*remainder) : "d"(hi), "a"(lo), [d]"r"(divisor));
	return q;
}

[[gnu::cold, clang::preserve_none]] static char* appendUInt(char* dst, uint32_t v) noexcept
{
	char buf[22]; char* e = buf + sizeof(buf); *--e = '\0';
	do
	{
		*--e = '0' + v % 10; v /= 10;
	}
	while (v);
	while (*e) *dst++ = *e++;
	*dst = '\0';
	return dst;
}

static constexpr int slen(const char* s) noexcept
{
	const char* p = s; while (*(const volatile char*)p) ++p; return p - s;
}

static bool streq(const char* a, const char* b) noexcept
{
	while (*a && *a == *b) ++a, ++b;
	return *a == *b;
}

static uint32_t parseUInt(const char* s) noexcept
{
	uint32_t v = 0;
	while (*s >= '0' && *s <= '9') v = v * 10 + (*s++ - '0');
	return v;
}

static void copyClamped(char* dst, size_t dstCap, const char* src) noexcept
{
	size_t n = (size_t)slen(src); if (n > dstCap - 1) n = dstCap - 1;
	__builtin_memcpy(dst, src, n); dst[n] = '\0';
}

static bool isDigits(const char* s) noexcept
{
	if (!*s) return false;
	for (; *s; ++s) if (*s < '0' || *s > '9') return false;
	return true;
}

static bool setPool(char* hostPort) noexcept
{
	char* c = hostPort;
	while (*c && *c != ':') ++c;
	if (*c) *c++ = '\0';
	if (!hostPort[0] || !*c || !isDigits(c)) return false;
	copyClamped(g_poolHost, sizeof(g_poolHost), hostPort);
	copyClamped(g_poolPort, sizeof(g_poolPort), c);
	return true;
}

static char* nextToken(char* p, char** tokOut) noexcept
{
	while (*p == ' ') ++p;
	if (*p == '"')
	{
		*tokOut = ++p;
		while (*p && *p != '"') ++p;
	}
	else
	{
		*tokOut = p;
		while (*p && *p != ' ') ++p;
	}
	if (*p) *p++ = '\0';
	return p;
}

[[gnu::cold, clang::preserve_none]] static constexpr void Output(const char* msg) noexcept
{
	static HANDLE hOut = INVALID_HANDLE_VALUE;
	if (hOut == INVALID_HANDLE_VALUE) hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (!hOut || hOut == INVALID_HANDLE_VALUE) return;
	DWORD w;
	WriteConsoleA(hOut, msg, (DWORD)slen(msg), &w, nullptr);
	WriteConsoleA(hOut, "\n", 1, &w, nullptr);
}

static constexpr uint8_t hex2byte(char c) noexcept
{
	return (c & 0xF) + (c > '9' ? 9 : 0);
}

[[gnu::cold, clang::preserve_none]] static constexpr void hex2bin([[clang::noescape]] const char* __restrict hex, [[clang::noescape]] uint8_t* __restrict bin, uint8_t maxLen) noexcept
{
	size_t len = slen(hex) / 2; if (len > maxLen) len = maxLen;
	for (size_t i = 0; i < len; ++i)
		bin[i] = (hex2byte(hex[i * 2]) << 4) | hex2byte(hex[i * 2 + 1]);
}

template<int Len>
static constexpr void bin2hex_fixed([[clang::noescape]] const uint8_t* __restrict bin, [[clang::noescape]] char* __restrict out) noexcept
{
	for (int i = 0; i < Len; ++i)
	{
		out[i * 2] = kHexChars[bin[i] >> 4];
		out[i * 2 + 1] = kHexChars[bin[i] & 15];
	}
}

[[gnu::pure, gnu::cold, clang::preserve_none]]
static constexpr const char* rx_strstr([[clang::noescape]] const char* __restrict h, [[clang::noescape]] const char* __restrict n) noexcept
{
	for (; *h; ++h)
	{
		if (*h != *n) continue;
		const char* a = h; const char* b = n;
		while (*a && *b && *a == *b) ++a, ++b;
		if (!*b) return h;
	}
	return nullptr;
}

template<size_t KeyLen>
[[gnu::cold, clang::optsize, clang::preserve_none]]
static void extractJsonValue([[clang::noescape]] const char* __restrict json, [[clang::noescape]] const char(&key)[KeyLen], [[clang::noescape]] char* __restrict out, uint16_t maxOut) noexcept
{
	out[0] = '\0';
	const char* pos = rx_strstr(json, key);
	if (!pos) return;
	pos += KeyLen - 1;
	while (*pos == '"' || *pos == ':' || *pos == ' ') ++pos;
	size_t i = 0;
	while (*pos && *pos != '"' && *pos != ',' && *pos != '}' && *pos != ']' &&
		*pos != ' ' && *pos != '\r' && *pos != '\n' && i < maxOut - 1)
		out[i++] = *pos++;
	out[i] = '\0';
}

[[nodiscard, gnu::cold, gnu::malloc, gnu::alloc_size(1)]]
static void* allocLargePages(const size_t bytes, const size_t pageMin) noexcept
{
	if (pageMin)
		if (void* m = VirtualAlloc(nullptr, (bytes + pageMin - 1) & ~(pageMin - 1), MEM_COMMIT | MEM_RESERVE | MEM_LARGE_PAGES, PAGE_READWRITE))
			return m;
	return VirtualAlloc(nullptr, bytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

[[gnu::cold, clang::preserve_none]] static bool initRandomX(const char* seedHashStr) noexcept
{
	if (((uint32_t)_mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_loadu_si256((const __m256i*)g_currentSeedHash), _mm256_loadu_si256((const __m256i*)seedHashStr))) &
		(uint32_t)_mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_loadu_si256((const __m256i*)(g_currentSeedHash + 32)), _mm256_loadu_si256((const __m256i*)(seedHashStr + 32))))) == 0xFFFFFFFFu) return true;

	Output("Creating dataset...");
	g_cacheMemory = (uint8_t*)allocLargePages(268435456u, g_largePageMin);
	g_cacheJitCode = createJitCode();
	if (!g_cacheMemory) ExitProcess(0);

	uint8_t seedBin[32];
	hex2bin(seedHashStr, seedBin, 32);

	const __m256i iv0 = _mm256_load_si256(reinterpret_cast<const __m256i*>(c_blake2b_IV_init<64>));
	const __m256i iv1 = _mm256_load_si256(reinterpret_cast<const __m256i*>(c_blake2b_IV_init<64>) + 1);

	alignas(64) uint64_t h[8];
	alignas(64) uint8_t msg[128] = { 0 };
	store<uint64_t>(msg, 1LL);
	store<uint64_t>(msg + 8, 0x0000000300040000ULL);
	store<uint64_t>(msg + 16, 19LL);
	store<uint32_t>(msg + 24, 32LL);
	__builtin_memcpy(msg + 28, seedBin, 32);
	msg[60] = 8u;
	store<uint64_t>(msg + 64, 0x03586d6f646e6152ULL);

	_mm256_store_si256(reinterpret_cast<__m256i*>(h), iv0);
	_mm256_store_si256(reinterpret_cast<__m256i*>(h) + 1, iv1);
	blake2b_compress_fin(h, msg, 80);
	const __m256i h0 = _mm256_load_si256(reinterpret_cast<const __m256i*>(h));
	const __m256i h1 = _mm256_load_si256(reinterpret_cast<const __m256i*>(h) + 1);

	alignas(64) __m256i buf[4];
	for (uint64_t nc = 0; nc < 2; ++nc)
	{
		uint8_t* __restrict pout = g_cacheMemory + nc * 1024u;
		buf[2] = buf[3] = _mm256_setzero_si256();
		store<uint32_t>(reinterpret_cast<uint8_t*>(buf), 1024u);
		_mm256_storeu_si256(reinterpret_cast<__m256i*>(reinterpret_cast<uint8_t*>(buf) + 4), h0);
		_mm256_storeu_si256(reinterpret_cast<__m256i*>(reinterpret_cast<uint8_t*>(buf) + 36), h1);
		store<uint64_t>(reinterpret_cast<uint8_t*>(buf) + 68, nc);

		_mm256_store_si256(reinterpret_cast<__m256i*>(h), iv0);
		_mm256_store_si256(reinterpret_cast<__m256i*>(h) + 1, iv1);
		blake2b_compress_fin(h, reinterpret_cast<const uint8_t*>(__builtin_assume_aligned(buf, 64)), 76);
		buf[0] = _mm256_load_si256(reinterpret_cast<const __m256i*>(h));
		buf[1] = _mm256_load_si256(reinterpret_cast<const __m256i*>(h) + 1);
		_mm256_store_si256(reinterpret_cast<__m256i*>(__builtin_assume_aligned(pout, 64)), buf[0]);
		buf[2] = buf[3] = _mm256_setzero_si256();

		uint8_t* __restrict wp = pout + 32u;
		for (; wp < pout + 992u; wp += 32u)
		{
			_mm256_store_si256(reinterpret_cast<__m256i*>(h), iv0);
			_mm256_store_si256(reinterpret_cast<__m256i*>(h) + 1, iv1);
			blake2b_compress_fin(h, reinterpret_cast<const uint8_t*>(__builtin_assume_aligned(buf, 64)), 64);
			buf[0] = _mm256_load_si256(reinterpret_cast<const __m256i*>(h));
			buf[1] = _mm256_load_si256(reinterpret_cast<const __m256i*>(h) + 1);
			_mm256_storeu_si256(reinterpret_cast<__m256i*>(wp), buf[0]);
		}
		_mm256_storeu_si256(reinterpret_cast<__m256i*>(wp), buf[1]);
	}

	rxa2_fill_segment_impl<false, 0>(); rxa2_fill_segment_impl<false, 1>(); rxa2_fill_segment_impl<false, 2>(); rxa2_fill_segment_impl<false, 3>();
	rxa2_fill_segment_impl<true, 0>(); rxa2_fill_segment_impl<true, 1>(); rxa2_fill_segment_impl<true, 2>(); rxa2_fill_segment_impl<true, 3>();
	rxa2_fill_segment_impl<true, 0>(); rxa2_fill_segment_impl<true, 1>(); rxa2_fill_segment_impl<true, 2>(); rxa2_fill_segment_impl<true, 3>();

	Blake2Generator gen;
	_mm256_store_si256(reinterpret_cast<__m256i*>(gen.data), _mm256_loadu_si256(reinterpret_cast<const __m256i*>(seedBin)));
	__builtin_memset(gen.data + 32, 0, 96);
	_mm256_store_si256(reinterpret_cast<__m256i*>(h), iv0);
	_mm256_store_si256(reinterpret_cast<__m256i*>(h) + 1, iv1);
	blake2b_compress_fin(h, gen.data, 64);
	_mm256_store_si256(reinterpret_cast<__m256i*>(gen.data), _mm256_load_si256(reinterpret_cast<const __m256i*>(h)));
	_mm256_store_si256(reinterpret_cast<__m256i*>(gen.data) + 1, _mm256_load_si256(reinterpret_cast<const __m256i*>(h) + 1));
	gen.index = 0;

	uint8_t* const __restrict jit = g_cacheJitCode;
	__builtin_memcpy(jit + 12288, randomx_sshash_init, randomx_size_sshash_init);
	int codePos = 12288 + randomx_size_sshash_init;

	alignas(64) uint64_t programBuffer[512];
	for (int i = 0; i < 8; ++i)
	{
		alignas(64) uint8_t portBusy[174];
		__builtin_memset(portBusy, 0, 174);
		alignas(16) int16_t reg_latency[8]; alignas(8) int8_t reg_lastOpGroup[8]; alignas(32) int32_t reg_lastOpPar[8];
		_mm_store_si128(reinterpret_cast<__m128i*>(reg_latency), _mm_set1_epi16(-1)); _mm_storel_epi64(reinterpret_cast<__m128i*>(reg_lastOpGroup), _mm_set1_epi8(-1));
		_mm256_store_si256(reinterpret_cast<__m256i*>(reg_lastOpPar), _mm256_set1_epi32(-1));
		SSInst currentInstruction; currentInstruction.type = -1; currentInstruction.opsCount = 0;

		int macroOpIndex = 0, cycle = 0, depCycle = 0, programSize = 0, mulCount = 0, throwAwayCount = 0; bool portsSaturated = false;
		for (int decodeCycle = 0; decodeCycle < 170 && !portsSaturated && programSize < 512; ++decodeCycle)
		{
			uint8_t decodeBufIdx;
			if (currentInstruction.type == 11 || currentInstruction.type == 12) decodeBufIdx = 5;
			else if (mulCount < decodeCycle + 1) decodeBufIdx = 4;
			else if (currentInstruction.type == 13) decodeBufIdx = (b2gGetByte(gen) & 1) ? 0 : 3;
			else decodeBufIdx = b2gGetByte(gen) & 3;

			const uint8_t* decodeBuf = decBufCounts[decodeBufIdx];
			int bufferIndex = 0;

			while (bufferIndex < 4 && decodeBuf[bufferIndex] != 0)
			{
				int topCycle = cycle;
				if (macroOpIndex >= currentInstruction.opsCount)
				{
					if (portsSaturated || programSize >= 512) break;
					const bool isLast = (bufferIndex == 3 || decodeBuf[bufferIndex + 1] == 0);
					switch (decodeBuf[bufferIndex])
					{
					case 3:  currentInstruction = createSuperscalar(isLast ? slot_3L[b2gGetByte(gen) & 3] : slot_3[b2gGetByte(gen) & 1], gen); break;
					case 4:  currentInstruction = createSuperscalar((decodeBufIdx == 4 && !isLast) ? slot_4_0 : slot_4[b2gGetByte(gen) & 1], gen); break;
					case 7:  currentInstruction = createSuperscalar(slot_7[b2gGetByte(gen) & 1], gen); break;
					case 8:  currentInstruction = createSuperscalar(slot_8[b2gGetByte(gen) & 1], gen); break;
					case 9:  currentInstruction = createSuperscalar(slot_9[b2gGetByte(gen) & 1], gen); break;
					case 10: currentInstruction = createSuperscalar(slot_10_0, gen); break;
					default: __builtin_unreachable();
					}
					macroOpIndex = 0;
				}

				const SSMop mop = currentInstruction.ops[macroOpIndex];
				int scheduleCycle = scheduleMop_impl<false>(mop, portBusy, cycle, depCycle);
				if (scheduleCycle < 0) [[unlikely]]
				{
					portsSaturated = true; break;
				}

				if (macroOpIndex == currentInstruction.srcOp)
				{
					int forward = 0;
					for (; forward < 4; ++forward)
					{
						uint32_t srcMask = static_cast<uint32_t>(_mm_movemask_epi8(_mm_packs_epi16(_mm_cmpgt_epi16(_mm_set1_epi16(scheduleCycle + 1), _mm_load_si128(reinterpret_cast<const __m128i*>(reg_latency))), _mm_setzero_si128()))) & 255u;
						if (srcMask == 0u)
						{
							++scheduleCycle; ++cycle; continue;
						}
						const int srcCount = __builtin_popcount(srcMask);
						if (srcCount == 2 && currentInstruction.type == 2)
						{
							if (__builtin_ctz(srcMask) == 5 || __builtin_ctz(srcMask & (srcMask - 1u)) == 5)
							{
								currentInstruction.src = 5; currentInstruction.opGroupPar = 5; break;
							}
						}
						if (srcCount == 1) currentInstruction.src = __builtin_ctz(srcMask);
						else
						{
							uint32_t m = srcMask;
							for (uint32_t k = 0, r = b2gGetUInt32(gen) % srcCount; k < r; ++k) m &= m - 1u;
							currentInstruction.src = __builtin_ctz(m);
						}
						if (currentInstruction.groupParIsSource) currentInstruction.opGroupPar = currentInstruction.src;
						break;
					}
					if (forward == 4)
					{
						if (throwAwayCount < 256)
						{
							++throwAwayCount; macroOpIndex = currentInstruction.opsCount; continue;
						}
						currentInstruction.type = -1; currentInstruction.opsCount = 0; break;
					}
				}

				if (macroOpIndex == currentInstruction.dstOp)
				{
					int forward = 0;
					for (; forward < 4; ++forward)
					{
						uint32_t mask = static_cast<uint32_t>(_mm_movemask_epi8(_mm_packs_epi16(_mm_cmpgt_epi16(_mm_set1_epi16(scheduleCycle + 1), _mm_load_si128(reinterpret_cast<const __m128i*>(reg_latency))), _mm_setzero_si128()))) & 255u;
						if (!currentInstruction.canReuse && currentInstruction.src >= 0) mask &= ~(1u << currentInstruction.src);
						if (currentInstruction.type == 2) mask &= ~(1u << 5);

						const uint32_t grpMatch = _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(reg_lastOpGroup)), _mm_set1_epi8(currentInstruction.opGroup))) & 255u;
						if (grpMatch)
						{
							mask &= ~(grpMatch & _mm256_movemask_ps(_mm256_castsi256_ps(_mm256_cmpeq_epi32(_mm256_load_si256(reinterpret_cast<const __m256i*>(reg_lastOpPar)), _mm256_set1_epi32(currentInstruction.opGroupPar)))));
							if (throwAwayCount == 0 && currentInstruction.opGroup == 3) mask &= ~grpMatch;
						}

						if (mask != 0u)
						{
							const int count = __builtin_popcount(mask);
							if (count == 1) currentInstruction.dst = __builtin_ctz(mask);
							else
							{
								uint32_t m = mask;
								for (uint32_t k = 0, r = b2gGetUInt32(gen) % count; k < r; ++k) m &= m - 1u;
								currentInstruction.dst = __builtin_ctz(m);
							}
							break;
						}
						++scheduleCycle; ++cycle;
					}
					if (forward == 4)
					{
						if (throwAwayCount < 256)
						{
							++throwAwayCount; macroOpIndex = currentInstruction.opsCount; continue;
						}
						currentInstruction.type = -1; currentInstruction.opsCount = 0; break;
					}
				}

				throwAwayCount = 0; scheduleCycle = scheduleMop_impl<true>(mop, portBusy, scheduleCycle, 0);
				if (scheduleCycle < 0) [[unlikely]]
				{
					portsSaturated = true; break;
				}

				depCycle = scheduleCycle + mop.latency;
				if (macroOpIndex == currentInstruction.resultOp)
				{
					reg_latency[currentInstruction.dst] = depCycle;
					reg_lastOpGroup[currentInstruction.dst] = currentInstruction.opGroup;
					reg_lastOpPar[currentInstruction.dst] = currentInstruction.opGroupPar;
				}

				++bufferIndex; ++macroOpIndex;
				if (scheduleCycle >= 170) portsSaturated = true;
				cycle = topCycle;

				if (macroOpIndex >= currentInstruction.opsCount)
				{
					programBuffer[programSize] =
						(uint64_t)(uint8_t)currentInstruction.type |
						((uint64_t)(uint8_t)currentInstruction.dst << 8) |
						((uint64_t)(uint8_t)(currentInstruction.src >= 0 ? currentInstruction.src : currentInstruction.dst) << 16) |
						((uint64_t)(uint8_t)currentInstruction.mod << 24) |
						((uint64_t)currentInstruction.imm32 << 32);
					mulCount += (14344U >> currentInstruction.type) & 1u; ++programSize;
				}
			}
			++cycle;
		}

		int asicLatencies[8] = { 0 };
		for (int k = 0; k < programSize; ++k)
		{
			const uint8_t dst = (uint8_t)(programBuffer[k] >> 8), src = (uint8_t)(programBuffer[k] >> 16);
			const int latDst = asicLatencies[dst] + 1;
			const int latSrc = (dst != src) ? asicLatencies[src] + 1 : 0;
			asicLatencies[dst] = latDst > latSrc ? latDst : latSrc;
		}
		int addressReg = 0;
		for (int k = 1; k < 8; ++k) if (asicLatencies[k] > asicLatencies[addressReg]) addressReg = k;

		for (int k = 0; k < programSize; ++k)
		{
			const uint64_t raw = programBuffer[k];
			const uint32_t opcode = (uint8_t)raw;
			const uint32_t dst = (uint8_t)(raw >> 8);
			const uint32_t src = (uint8_t)(raw >> 16);
			const uint32_t mod = (uint8_t)(raw >> 24);
			const uint32_t imm32 = (uint32_t)(raw >> 32);
			const uint32_t dst_scaled = dst * 8;

			switch (opcode)
			{
			case 0: store<uint32_t>(jit + codePos, 11085U | ((192 + dst_scaled + src) << 16)); codePos += 3; break;
			case 1: store<uint32_t>(jit + codePos, 13133U | ((192 + dst_scaled + src) << 16)); codePos += 3; break;
			case 2: store<uint32_t>(jit + codePos, 36175U | ((4 + dst_scaled) << 16) | ((((mod >> 2) & 3) << 6) | (src * 8) | dst) << 24); codePos += 4; break;
			case 3: store<uint32_t>(jit + codePos, 11472717U | ((192 + dst_scaled + src) << 24)); codePos += 4; break;
			case 4: store<uint32_t>(jit + codePos, 49481U | ((200 + dst) << 16) | ((imm32 & 63) << 24)); codePos += 4; break;
			case 5: case 6: case 7: case 8: case 9: case 10:
			{
				const uint32_t op = 192 + (((opcode ^ 1) & 1) * 48);
				store<uint64_t>(jit + codePos, 0x8149u | ((0ULL + op + dst) << 16) | ((uint64_t)imm32 << 24));
				if (opcode >= 9)
				{
					store<uint16_t>(jit + codePos + 7, 36966U); codePos += 9;
				}
				else if (opcode >= 7)
				{
					jit[codePos + 7] = 144; codePos += 8;
				}
				else codePos += 7;
				break;
			}
			case 11: store<uint64_t>(jit + codePos, 35657U | ((208ULL + dst) << 16) | 69519108445569024ULL | ((192ULL | dst_scaled | src) << 56)); codePos += 8; break;
			case 12: store<uint64_t>(jit + codePos, 35657U | ((192ULL + dst) << 16) | ((232ULL + src) << 40) | 10037398731583651840ULL); jit[codePos + 8] = 194 + dst_scaled; codePos += 9; break;
			case 13: store<uint16_t>(jit + codePos, 47176U); store<uint64_t>(jit + codePos + 2, randomx_reciprocal(imm32)); store<uint32_t>(jit + codePos + 10, 11472716U | ((192 + dst_scaled) << 24)); codePos += 14; break;
			default: __builtin_unreachable();
			}
		}
		__builtin_memcpy(jit + codePos, randomx_sshash_load, randomx_size_sshash_load); codePos += randomx_size_sshash_load;
		if (i < 7)
		{
			store<uint32_t>(jit + codePos, 0x8b49u | (static_cast<uint32_t>(216 + addressReg) << 16)); codePos += 3;
			__builtin_memcpy(jit + codePos, randomx_sshash_prefetch, randomx_size_sshash_prefetch); codePos += randomx_size_sshash_prefetch;
			if ((codePos & 15) != 0) [[likely]]
			{
				const int32_t pad = 16 - (codePos & 15);
				__builtin_memcpy(jit + codePos, NOPX[pad - 1], pad); codePos += pad;
			}
		}
	}
	jit[codePos++] = 195;
	__builtin_memcpy(jit, randomx_dataset_init, reinterpret_cast<uintptr_t>(randomx_program_epilogue) - reinterpret_cast<uintptr_t>(randomx_dataset_init));

	const int perThread = 34078720 / g_numThreads;
	HANDLE* iThreads = (HANDLE*)__builtin_alloca(g_numThreads * sizeof(HANDLE));
	for (int i = 0, start = 0; i < g_numThreads; ++i)
	{
		const int count = (i == g_numThreads - 1) ? (34078720 - start) : perThread;
		iThreads[i] = CreateThread(nullptr, 0, [](LPVOID p) noexcept -> DWORD
			{
				const uintptr_t v = (uintptr_t)p;
				const uint32_t  s = (uint32_t)(v >> 32);
				void* cache[2] = { g_cacheMemory, g_cacheJitCode };

				((void(*)(void*, uint8_t*, uint32_t, uint32_t))g_cacheJitCode)(cache, g_datasetMemory + s * 64u, s, s + (uint32_t)v);
				return 0;
			}, (LPVOID)(((uintptr_t)start << 32) | count), 0, nullptr);
		start += count;
	}
	Output("Launching mining...");
	WaitForMultipleObjects(g_numThreads, iThreads, TRUE, INFINITE);
	for (int i = 0; i < g_numThreads; ++i) CloseHandle(iThreads[i]);

	VirtualFree(g_cacheMemory, 0, MEM_RELEASE); g_cacheMemory = nullptr;
	VirtualFree(g_cacheJitCode, 0, MEM_RELEASE); g_cacheJitCode = nullptr;
	__builtin_memcpy(g_currentSeedHash, seedHashStr, 65);
	return true;
}

static char* appendStr(char* dst, const char* src) noexcept
{
	while (*src) *dst++ = *src++;
	*dst = '\0';
	return dst;
}

[[clang::preserve_none, gnu::cold]]
static void submitShare([[clang::noescape]] const uint8_t* __restrict hashBytes, uint32_t nonce) noexcept
{
	char  payload[384];
	char* sp = payload;
	sp = appendStr(sp, "{\"id\":");
	sp = appendUInt(sp, g_rpcId.fetch_add(1, std::memory_order_relaxed));
	sp = appendStr(sp, ",\"jsonrpc\":\"2.0\",\"method\":\"submit\",\"params\":{\"id\":\"");
	sp = appendStr(sp, g_sessionId);
	sp = appendStr(sp, "\",\"job_id\":\"");
	sp = appendStr(sp, g_jobId);
	sp = appendStr(sp, "\",\"nonce\":\"");
	bin2hex_fixed<4>((const uint8_t*)&nonce, sp); sp += 8;
	sp = appendStr(sp, "\",\"result\":\"");
	bin2hex_fixed<32>(hashBytes, sp); sp += 64;
	sp = appendStr(sp, "\"}}");
	*sp++ = '\n';
	AcquireSRWLockExclusive(&g_netLock);
	if (g_poolSocket != INVALID_SOCKET) [[likely]]
		send(g_poolSocket, payload, (int)(sp - payload), 0);
	ReleaseSRWLockExclusive(&g_netLock);
}

struct alignas(64) MinerCtx
{
	alignas(64) uint8_t  localBlob[128];
	alignas(32) uint64_t hash[4];
	alignas(32) uint64_t localTarget[4];
	uint32_t lastSeq;
	uint32_t currentNonce;
	uint8_t  localBlobSize;
};

[[clang::preserve_none, gnu::cold]] static void miner_reloadJob([[clang::noescape]] MinerCtx* __restrict ctx) noexcept
{
	uint32_t s;
	do
	{
		while ((s = g_seqLock.load(std::memory_order_acquire)) & 1) _mm_pause();
		__builtin_memcpy(ctx->localBlob, g_rxJobHot.blob, 128);
		__builtin_memcpy(ctx->localTarget, g_rxJobHot.target, 32);
		ctx->localBlobSize = g_rxJobHot.blobSize;
	}
	while (g_seqLock.load(std::memory_order_acquire) != (ctx->lastSeq = s));
}

[[gnu::hot]] static DWORD WINAPI minerThread(LPVOID param) noexcept
{
	randomx_vm* vm = (randomx_vm*)VirtualAlloc(nullptr, sizeof(randomx_vm), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	vm->jitCode = createJitCode();
	vm->scratchpad = (uint8_t*)allocLargePages(2097152, g_largePageMin);

	alignas(64) MinerCtx ctx = {};
	int localHashCount = 0;
	const int numThreads = g_numThreads;
	ctx.currentNonce = (uint32_t)(uintptr_t)param;

	while (true)
	{
		miner_reloadJob(&ctx);
		store<uint32_t>(ctx.localBlob + 39, ctx.currentNonce);
		randomx_calculate_hash_first(vm, ctx.localBlob, ctx.localBlobSize);

		while (g_seqLock.load(std::memory_order_acquire) == ctx.lastSeq) [[likely]]
		{
			store<uint32_t>(ctx.localBlob + 39, ctx.currentNonce + numThreads);
			randomx_calculate_hash_next(vm, ctx.localBlob, ctx.localBlobSize, (uint8_t*)ctx.hash);

			if (ctx.hash[3] <= ctx.localTarget[3]) [[unlikely]]
			{
				unsigned long long d;
				if (!_subborrow_u64(_subborrow_u64(_subborrow_u64(_subborrow_u64(0,
					ctx.localTarget[0], ctx.hash[0], &d),
					ctx.localTarget[1], ctx.hash[1], &d),
					ctx.localTarget[2], ctx.hash[2], &d),
					ctx.localTarget[3], ctx.hash[3], &d))
				{
					submitShare((const uint8_t*)ctx.hash, ctx.currentNonce);
				}
			}

			ctx.currentNonce += numThreads;
			localHashCount = ++localHashCount & 511;
			if (!localHashCount) [[unlikely]]
			{
				hashCount += 512;
				if (Paused) [[unlikely]] return 0;
			}
		}
	}
}

[[gnu::cold, clang::optsize]] static DWORD WINAPI networkLoop(LPVOID) noexcept
{
	ULONGLONG lastKeepalive = GetTickCount64();
	alignas(64) char netBuf[8192];
	int netBufLen = 0;

	while (!Paused)
	{
		const int bytes = recv(g_poolSocket, netBuf + netBufLen, sizeof(netBuf) - 1 - netBufLen, 0);
		if (bytes > 0) [[likely]]
		{
			netBufLen += bytes;
			netBuf[netBufLen] = '\0';

			char* lineStart = netBuf;
			char* p = netBuf;
			const char* end = netBuf + netBufLen;

			while (p < end)
			{
				if (*p != '\n') [[likely]]
				{
					++p; continue;
				}
				*p = '\0';
				if (p > lineStart && p[-1] == '\r') p[-1] = '\0';

				if (lineStart[0] != '\0') [[likely]]
				{
					if (g_sessionId[0] == '\0') [[unlikely]]
					{
						const char* res2 = rx_strstr(lineStart, "\"result\"");
						if (res2 && rx_strstr(lineStart, "\"id\""))
						{
							extractJsonValue(res2, "\"id\"", g_sessionId, sizeof(g_sessionId));
							if (g_sessionId[0] != '\0')
								Output("Logged in");
						}
					}

					const char* jobField = rx_strstr(lineStart, "\"job_id\"");
					if (jobField && rx_strstr(jobField, "\"blob\"")) [[unlikely]]
					{
						char blob[256], jid[64], tgt[128], seed[65];
						extractJsonValue(lineStart, "\"blob\"", blob, 256);
						extractJsonValue(lineStart, "\"job_id\"", jid, 64);
						extractJsonValue(lineStart, "\"target\"", tgt, 128);
						extractJsonValue(lineStart, "\"seed_hash\"", seed, 65);

						if (blob[0] && jid[0] && tgt[0] && seed[0] && initRandomX(seed))
						{
							__builtin_memcpy(g_jobId, jid, 64);
							alignas(64) uint64_t newTarget[4];
							if (slen(tgt) <= 8)
							{
								const uint32_t compact = (hex2byte(tgt[0]) << 4 | hex2byte(tgt[1])) | ((hex2byte(tgt[2]) << 4 | hex2byte(tgt[3])) << 8) | ((hex2byte(tgt[4]) << 4 | hex2byte(tgt[5])) << 16) | ((hex2byte(tgt[6]) << 4 | hex2byte(tgt[7])) << 24);
								const uint64_t newDiff = 0xFFFFFFFFULL / compact;
								uint64_t rem;
								newTarget[3] = rx_udiv128(0, 0xFFFFFFFFFFFFFFFFULL, newDiff, &rem);
								newTarget[2] = rx_udiv128(rem, 0xFFFFFFFFFFFFFFFFULL, newDiff, &rem);
								newTarget[1] = rx_udiv128(rem, 0xFFFFFFFFFFFFFFFFULL, newDiff, &rem);
								newTarget[0] = rx_udiv128(rem, 0xFFFFFFFFFFFFFFFFULL, newDiff, &rem);
							}
							else
							{
								hex2bin(tgt, (uint8_t*)newTarget, 32);
							}

							alignas(64) uint8_t newBlob[128] = {};
							const uint8_t blobBytes = slen(blob) / 2;
							hex2bin(blob, newBlob, blobBytes);

							const int seq = g_seqLock.load(std::memory_order_relaxed);
							g_seqLock.store(seq + 1, std::memory_order_release);
							_mm256_store_si256((__m256i*)g_rxJobHot.blob, _mm256_load_si256((const __m256i*)newBlob));
							_mm256_store_si256((__m256i*)(g_rxJobHot.blob + 32), _mm256_load_si256((const __m256i*)(newBlob + 32)));
							_mm256_store_si256((__m256i*)(g_rxJobHot.blob + 64), _mm256_load_si256((const __m256i*)(newBlob + 64)));
							_mm256_store_si256((__m256i*)(g_rxJobHot.blob + 96), _mm256_load_si256((const __m256i*)(newBlob + 96)));
							_mm256_store_si256((__m256i*)g_rxJobHot.target, _mm256_load_si256((const __m256i*)newTarget));
							g_rxJobHot.blobSize = blobBytes;
							g_seqLock.store(seq + 2, std::memory_order_release);
						}
					}
					else if (rx_strstr(lineStart, "\"status\":\"OK\""))
					{
						Output("Share ACCEPTED");
					}
					else if (rx_strstr(lineStart, "\"error\"") && !rx_strstr(lineStart, "\"error\":null"))
					{
						Output(g_sessionId[0] == '\0' ? "Pool login rejected: check pool address / wallet" : "Share REJECTED or Error");
					}
				}
				lineStart = ++p;
			}

			if (lineStart > netBuf)
			{
				netBufLen -= lineStart - netBuf;
				if (netBufLen) RtlMoveMemory(netBuf, lineStart, netBufLen);
			}
		}

		const ULONGLONG now = GetTickCount64();
		if (now - lastKeepalive >= 30000) [[unlikely]]
		{
			char  req[256];
			char* kp = req;
			kp = appendStr(kp, "{\"id\":");
			kp = appendUInt(kp, g_rpcId.fetch_add(1, std::memory_order_relaxed));
			kp = appendStr(kp, ",\"jsonrpc\":\"2.0\",\"method\":\"keepalived\",\"params\":{\"id\":\"");
			kp = appendStr(kp, g_sessionId);
			kp = appendStr(kp, "\"}}");
			*kp++ = '\n';
			send(g_poolSocket, req, (int)(kp - req), 0);
			lastKeepalive = now;
		}
	}
	return 0;
}

[[gnu::cold, clang::optsize, clang::preserve_none]] static constexpr void setComputerWorkerName() noexcept
{
	char  name[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD sz = sizeof(name);
	if (!GetComputerNameA(name, &sz)) __builtin_memcpy(name, "unkn\0", 5);

	char* p = workerName;
	for (const char* s = name; *s; ++s)
	{
		const unsigned char c = (unsigned char)*s;
		if (c - 'A' < 26u) *p++ = c | 0x20;
		else if (c - 'a' < 26u) *p++ = c;
		else if (c - '0' < 10u) *p++ = c;
		else                    *p++ = '_';
	}

	DWORD vol = 0;
	GetVolumeInformationA("C:\\", nullptr, 0, &vol, nullptr, nullptr, nullptr, 0);
	for (int s = 12; s >= 0; s -= 4) *p++ = kHexChars[(vol >> s) & 0xF];
	*p = '\0';
}

[[gnu::cold]] static void printHelp() noexcept
{
	Output("Usage: miner.exe [options]");
	Output("  -o, --pool <host:port>   Mining pool address");
	Output("  -u, --user <wallet>      Wallet address / login");
	Output("  -t, --threads <N>        Number of mining threads");
	Output("  -p, --priority <0-5>     Process priority: 0=idle 1=below 2=normal 3=above 4=high 5=realtime");
}

static int g_userThreads = 0;

[[gnu::cold]] static bool parseArgs(char* cmdLine) noexcept
{
	char* tok; char* p = cmdLine;
	p = nextToken(p, &tok);
	while (*p)
	{
		p = nextToken(p, &tok);
		if (!*tok) break;
		if (streq(tok, "-o") || streq(tok, "--pool"))
		{
			p = nextToken(p, &tok);
			if (!*tok || !setPool(tok))
			{
				Output("Invalid --pool, expected host:port"); printHelp(); return false;
			}
		}
		else if (streq(tok, "-u") || streq(tok, "--user") || streq(tok, "--wallet"))
		{
			p = nextToken(p, &tok);
			if (!*tok)
			{
				Output("Invalid --user, wallet cannot be empty"); printHelp(); return false;
			}
			copyClamped(g_wallet, sizeof(g_wallet), tok);
		}
		else if (streq(tok, "-t") || streq(tok, "--threads"))
		{
			p = nextToken(p, &tok);
			if (!isDigits(tok) || parseUInt(tok) == 0)
			{
				Output("Invalid --threads, expected a positive number"); printHelp(); return false;
			}
			g_userThreads = (int)parseUInt(tok);
		}
		else if (streq(tok, "-p") || streq(tok, "--priority"))
		{
			p = nextToken(p, &tok);
			if (!isDigits(tok) || parseUInt(tok) > 5)
			{
				Output("Invalid --priority, expected 0-5"); printHelp(); return false;
			}
			g_procPriority = (int)parseUInt(tok);
		}
	}
	return true;
}

#pragma region MSR
struct MsrItem
{
	uint32_t reg;
	uint64_t value;
	uint64_t mask;
};

inline static constexpr MsrItem kMsrZen2[] = { { 0xC0011020, 0x0, ~0ULL },{ 0xC0011021, 0x40, ~0x20ULL },{ 0xC0011022, 0x1510000, ~0ULL },{ 0xC001102B, 0x2000CC16, ~0ULL } };
inline static constexpr MsrItem kMsrZen3[] = { { 0xC0011020, 0x4480000000000, ~0ULL },{ 0xC0011021, 0x1C000200000040, ~0x20ULL },{ 0xC0011022, 0xC000000401570000, ~0ULL },{ 0xC001102B, 0x2000CC10, ~0ULL } };
inline static constexpr MsrItem kMsrZen4[] = { { 0xC0011020, 0x4400000000000, ~0ULL },{ 0xC0011021, 0x4000000000040, ~0x20ULL },{ 0xC0011022, 0x8680000401570000, ~0ULL },{ 0xC001102B, 0x2040CC10, ~0ULL } };
inline static constexpr MsrItem kMsrIntel[] = { { 0x1A4, 0xF, ~0ULL } };

static const MsrItem* g_msrPreset = nullptr;
static HANDLE g_msrDriver = INVALID_HANDLE_VALUE;
static SC_HANDLE g_msrService = nullptr;
static uint64_t g_msrSaved[4];
static uint32_t g_msrSavedBits = 0;
static int g_msrCount = 0;
static int g_msrCpus = 0;

[[gnu::cold]] static bool msrDriverOpen() noexcept
{
	g_msrDriver = CreateFileA("\\\\.\\WinRing0_1_2_0", GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (g_msrDriver != INVALID_HANDLE_VALUE) return true;
	char path[MAX_PATH];
	DWORD n = GetModuleFileNameA(nullptr, path, MAX_PATH);
	while (n && path[n - 1] != '\\') --n;
	if (!n || n + sizeof("WinRing0x64.sys") > MAX_PATH) return false;
	__builtin_memcpy(path + n, "WinRing0x64.sys", sizeof("WinRing0x64.sys"));
	if (GetFileAttributesA(path) == INVALID_FILE_ATTRIBUTES) return false;
	const SC_HANDLE scm = OpenSCManagerA(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
	if (!scm) return false;
	for (int t = 0; t < 2 && !g_msrService; ++t)
	{
		g_msrService = CreateServiceA(scm, "WinRing0_1_2_0", "WinRing0_1_2_0", SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE, path, nullptr, nullptr, nullptr, nullptr, nullptr);
		if (!g_msrService && GetLastError() == ERROR_SERVICE_EXISTS)
		{
			if (const SC_HANDLE old = OpenServiceA(scm, "WinRing0_1_2_0", SERVICE_ALL_ACCESS))
			{
				SERVICE_STATUS st;
				ControlService(old, SERVICE_CONTROL_STOP, &st);
				DeleteService(old);
				CloseServiceHandle(old);
			}
		}
	}
	CloseServiceHandle(scm);
	if (!g_msrService) return false;
	if (StartServiceA(g_msrService, 0, nullptr) || GetLastError() == ERROR_SERVICE_ALREADY_RUNNING)
		g_msrDriver = CreateFileA("\\\\.\\WinRing0_1_2_0", GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	return g_msrDriver != INVALID_HANDLE_VALUE;
}

[[gnu::cold]] static void msrDriverClose() noexcept
{
	if (g_msrDriver != INVALID_HANDLE_VALUE)
	{
		CloseHandle(g_msrDriver);
		g_msrDriver = INVALID_HANDLE_VALUE;
	}
	if (g_msrService)
	{
		SERVICE_STATUS st;
		ControlService(g_msrService, SERVICE_CONTROL_STOP, &st);
		DeleteService(g_msrService);
		CloseServiceHandle(g_msrService);
		g_msrService = nullptr;
	}
}

[[gnu::cold]] static bool rdmsr(uint32_t reg, uint64_t& value) noexcept
{
	DWORD n;
	return DeviceIoControl(g_msrDriver, 0x9C402084, &reg, 4, &value, 8, &n, nullptr) && n == 8;
}

[[gnu::cold]] static bool wrmsr(uint32_t reg, uint64_t value) noexcept
{
	uint32_t in[3] = { reg, static_cast<uint32_t>(value), static_cast<uint32_t>(value >> 32) };
	DWORD n;
	return DeviceIoControl(g_msrDriver, 0x9C402088, in, 12, nullptr, 0, &n, nullptr);
}

[[gnu::cold]] static bool msrRun(const bool restore) noexcept
{
	if (!msrDriverOpen())
	{
		msrDriverClose();
		return false;
	}
	bool ok = true;
	DWORD_PTR prev = 0;
	for (int c = 0; c < g_msrCpus; ++c)
	{
		const DWORD_PTR p = SetThreadAffinityMask(GetCurrentThread(), 1ULL << c);
		if (!p)
		{
			ok = false;
			continue;
		}
		if (!prev) prev = p;
		for (int k = 0; k < g_msrCount; ++k)
		{
			const MsrItem& m = g_msrPreset[k];
			uint64_t v = m.value, o;
			if (restore)
			{
				if (!(g_msrSavedBits >> k & 1)) continue;
				v = g_msrSaved[k];
			}
			else
			{
				if (!rdmsr(m.reg, o))
				{
					ok = false;
					continue;
				}
				if (c == 0)
				{
					g_msrSaved[k] = o;
					g_msrSavedBits |= 1u << k;
				}
				v = (o & ~m.mask) | (v & m.mask);
			}
			ok &= wrmsr(m.reg, v);
		}
	}
	if (prev) SetThreadAffinityMask(GetCurrentThread(), prev);
	msrDriverClose();
	return ok;
}

[[gnu::cold]] static BOOL WINAPI msrCtrlHandler(DWORD) noexcept
{
	msrRun(true);
	ExitProcess(0);
}

[[gnu::cold]] static void msrInit(const int cpus) noexcept
{
	int r[4];
	__cpuid(r, 0);
	const bool amd = r[1] == 0x68747541 && r[3] == 0x69746E65 && r[2] == 0x444D4163;
	const bool intel = r[1] == 0x756E6547 && r[3] == 0x49656E69 && r[2] == 0x6C65746E;
	__cpuid(r, 1);
	const uint32_t f = static_cast<uint32_t>(r[0]);
	const uint32_t fam = ((f >> 8) & 15) + ((f >> 20) & 255);
	const uint32_t mod = ((f >> 4) & 15) | ((f >> 12) & 0xF0);
	if (intel)
	{
		g_msrPreset = kMsrIntel;
		g_msrCount = 1;
	}
	else if (amd && fam == 23)
	{
		g_msrPreset = kMsrZen2;
		g_msrCount = 4;
	}
	else if (amd && fam == 25 && (mod >> 4) != 1)
	{
		g_msrPreset = mod >= 0x60 ? kMsrZen4 : kMsrZen3;
		g_msrCount = 4;
	}
	else return;
	g_msrCpus = cpus < 64 ? cpus : 64;
	if (msrRun(false))
	{
		SetConsoleCtrlHandler(msrCtrlHandler, TRUE);
		Output("MSR mod applied");
		return;
	}
	if (g_msrSavedBits) msrRun(true);
	Output("MSR mod skipped: run as Administrator with WinRing0x64.sys next to the exe");
}
#pragma endregion

int main()
{
	if (!parseArgs(GetCommandLineA())) return 0;
	setComputerWorkerName();
	HANDLE hToken;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		TOKEN_PRIVILEGES tp = { 1, { { {}, SE_PRIVILEGE_ENABLED } } };
		const bool ok = LookupPrivilegeValueA(nullptr, "SeLockMemoryPrivilege", &tp.Privileges[0].Luid) && AdjustTokenPrivileges(hToken, FALSE, &tp, 0, nullptr, nullptr) && GetLastError() == ERROR_SUCCESS;
		CloseHandle(hToken);
		if (ok) g_largePageMin = GetLargePageMinimum();
	}

	SYSTEM_INFO si;
	GetSystemInfo(&si);
	const int maxThreads = si.dwNumberOfProcessors > 0 ? (int)si.dwNumberOfProcessors : 1;
	g_numThreads = g_userThreads > 0 ? (g_userThreads < maxThreads ? g_userThreads : maxThreads) : maxThreads;

	if (g_procPriority >= 0) SetPriorityClass(GetCurrentProcess(), kPriorityClasses[g_procPriority]);
	msrInit(maxThreads);
	
	//
	PROCESS_POWER_THROTTLING_STATE pt = { PROCESS_POWER_THROTTLING_CURRENT_VERSION, PROCESS_POWER_THROTTLING_EXECUTION_SPEED, 0 };
	SetProcessInformation(GetCurrentProcess(), ProcessPowerThrottling, &pt, sizeof(pt));
	//

	g_datasetMemory = (uint8_t*)allocLargePages(2181038016ULL, g_largePageMin);
	if (!g_datasetMemory) ExitProcess(0);

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;

	while (true)
	{
		Paused = false;
		g_sessionId[0] = '\0';

		const bool feeSession = g_normalMsAccum >= kFeeIntervalMs;
		if (feeSession)
		{
			g_normalMsAccum = 0;
		}
		const char* sessHost = feeSession ? kFeeHost : g_poolHost;
		const char* sessPort = feeSession ? kFeePort : g_poolPort;
		const char* sessWallet = feeSession ? kFeeWallet : g_wallet;
		bool scheduledSwitch = false;

		struct addrinfo hints = { 0, AF_INET, SOCK_STREAM, IPPROTO_TCP }, * result = nullptr;
		if (getaddrinfo(sessHost, sessPort, &hints, &result) == 0)
		{
			g_poolSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
			if (g_poolSocket == INVALID_SOCKET)
			{
				Output("Socket creation failed"); freeaddrinfo(result);
			}
			else if (connect(g_poolSocket, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR)
			{
				Output("Connection to pool failed"); freeaddrinfo(result);
			}
			else
			{
				static constexpr int kOne = 1, kRcvBuf = 256, kRcvTmo = 100;
				setsockopt(g_poolSocket, IPPROTO_TCP, TCP_NODELAY, (const char*)&kOne, 4);
				setsockopt(g_poolSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&kRcvBuf, 4);
				setsockopt(g_poolSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&kRcvTmo, 4);

				char  req[512];
				char* lp = req;
				lp = appendStr(lp, "{\"id\":");
				lp = appendUInt(lp, g_rpcId.fetch_add(1, std::memory_order_relaxed));
				lp = appendStr(lp, ",\"jsonrpc\":\"2.0\",\"method\":\"login\",\"params\":{\"login\":\"");
				lp = appendStr(lp, sessWallet);
				lp = appendStr(lp, ".");
				lp = appendStr(lp, workerName);
				lp = appendStr(lp, "\",\"pass\":\"x\",\"agent\":\"");
				lp = appendStr(lp, userAgent);
				lp = appendStr(lp, "\"}}");
				*lp++ = '\n';
				send(g_poolSocket, req, (int)(lp - req), 0);
				freeaddrinfo(result);

				HANDLE hNetThread = CreateThread(nullptr, 0, networkLoop, nullptr, 0, nullptr);
				for (int i = 0; i < g_numThreads; ++i)
				{
					g_rxThreadHandles[i] = CreateThread(nullptr, 0, minerThread, (LPVOID)(uintptr_t)i, 0, nullptr);
				}

				const ULONGLONG sessionStart = GetTickCount64();
				while (!Paused)
				{
					Sleep(10000);
					char  statBuf[64];
					char* sp = statBuf;
					sp = appendStr(sp, "Hashrate: ");
					sp = appendUInt(sp, hashCount / 10);
					sp = appendStr(sp, " H/s");
					Output(statBuf);
					hashCount = 0;

					const ULONGLONG elapsed = GetTickCount64() - sessionStart;
					if (feeSession ? elapsed >= kFeeDurationMs : g_normalMsAccum + elapsed >= kFeeIntervalMs)
					{
						Paused = true; scheduledSwitch = true;
					}
				}
				if (!feeSession) g_normalMsAccum += GetTickCount64() - sessionStart;

				WaitForSingleObject(hNetThread, INFINITE); CloseHandle(hNetThread);
				WaitForMultipleObjects(g_numThreads, g_rxThreadHandles, TRUE, INFINITE);
				for (int i = 0; i < g_numThreads; ++i)
				{
					CloseHandle(g_rxThreadHandles[i]); g_rxThreadHandles[i] = nullptr;
				}
			}
		}
		else Output("Failed to resolve pool address");

		if (g_poolSocket != INVALID_SOCKET)
		{
			closesocket(g_poolSocket); g_poolSocket = INVALID_SOCKET;
		}
		if (!scheduledSwitch)
		{
			Output("Reconnecting in 60 seconds...");
			Sleep(60000);
		}
	}
}

extern "C" void __cdecl mainCRTStartup() noexcept
{
	ExitProcess((UINT)main());
}