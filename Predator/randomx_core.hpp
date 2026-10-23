#pragma once

#include <avx2intrin.h>
#include <avxintrin.h>
#include <cstdarg>
#include <cstdint>
#include <emmintrin.h>
#include <mmintrin.h>
#include <smmintrin.h>
#include <windows.h>

struct randomx_vm;

extern "C"
{
	extern const uint8_t randomx_prefetch_scratchpad[];
	extern const uint8_t randomx_program_prologue[];
	extern const uint8_t randomx_program_loop_load[];
	extern const uint8_t randomx_program_read_dataset[];
	extern const uint8_t randomx_dataset_init[];
	extern const uint8_t randomx_program_epilogue[];
	extern const uint8_t randomx_sshash_load[];
	extern const uint8_t randomx_sshash_prefetch[];
	extern const uint8_t randomx_sshash_init[];

	[[clang::noalias, gnu::nonnull(1), gnu::access(read_write, 1)]]
	void fillAes1Rx4_hw(randomx_vm* __restrict machine) noexcept;
	[[clang::noalias, gnu::nonnull(1), gnu::access(read_write, 1)]]
	void fillAes4Rx4_hw(randomx_vm* __restrict machine) noexcept;
	[[clang::noalias, gnu::nonnull(1), gnu::access(read_write, 1)]]
	void hashAndFillAes1Rx4_hw(randomx_vm* __restrict machine) noexcept;
	[[clang::noalias, gnu::nonnull(1, 2), gnu::access(read_write, 1), gnu::access(read_only, 2)]]
	void blake2b_compress_mid(uint64_t* __restrict h, const uint8_t* __restrict m) noexcept;
	[[clang::noalias, gnu::nonnull(1, 2), gnu::access(read_write, 1), gnu::access(read_only, 2)]]
	void blake2b_compress_fin(uint64_t* __restrict h, const uint8_t* __restrict m, uint64_t t0) noexcept;
}

inline static constexpr int32_t randomx_size_prologue = 320;
inline static constexpr int32_t randomx_size_loopload = 150;
inline static constexpr int32_t randomx_size_read_dataset = 66;
inline static constexpr int32_t randomx_size_loopstore = 82;
inline static constexpr int32_t randomx_size_epilogue = 213;
inline static constexpr int32_t randomx_size_prefetch_scratchpad = 25;
inline static constexpr int32_t randomx_size_sshash_load = 31;
inline static constexpr int32_t randomx_size_sshash_prefetch = 17;
inline static constexpr int32_t randomx_size_sshash_init = 192;

struct SSMop
{
	uint8_t latency, uop1, uop2, dep;
};
struct SSInfo
{
	int8_t type, resultOp, dstOp, srcOp; uint8_t opsCount; SSMop ops[3];
};

static constexpr SSInfo slot_3[] = { { 0,0,0, 0,1,{ { 1,7,0,0 } } },{ 1,0,0, 0,1,{ { 1,7,0,0 } } } };
static constexpr SSInfo slot_3L[] = { { 0,0,0, 0,1,{ { 1,7,0,0 } } },{ 1,0,0, 0,1,{ { 1,7,0,0 } } },{ 11,1,0,1,3,{ { 0,0,0,0 },{ 4,2,4,0 },{ 0,0,0,0 } } },{ 12,1,0,1,3,{ { 0,0,0,0 },{ 4,2,4,0 },{ 0,0,0,0 } } } };
static constexpr SSInfo slot_4[] = { { 4,0,0,-1,1,{ { 1,5,0,0 } } },{ 2,0,0, 0,1,{ { 1,3,0,0 } } } };
static constexpr SSInfo slot_7[] = { { 6,0,0,-1,1,{ { 1,7,0,0 } } },{ 5,0,0,-1,1,{ { 1,7,0,0 } } } };
static constexpr SSInfo slot_8[] = { { 8,0,0,-1,1,{ { 1,7,0,0 } } },{ 7,0,0,-1,1,{ { 1,7,0,0 } } } };
static constexpr SSInfo slot_9[] = { { 10,0,0,-1,1,{ { 1,7,0,0 } } },{ 9,0,0,-1,1,{ { 1,7,0,0 } } } };
static constexpr SSInfo slot_4_0 = { 3,0,0, 0,1,{ { 3,2,0,0 } } };
static constexpr SSInfo slot_10_0 = { 13,1,1,-1,2,{ { 1,7,0,0 },{ 3,2,0,1 } } };
static constexpr uint8_t decBufCounts[6][4] = { { 4, 8, 4, 0 },{ 7, 3, 3, 3 },{ 3, 7, 3, 3 },{ 4, 9, 3, 0 },{ 4, 4, 4, 4 },{ 3, 3, 10, 0 } };
static constexpr uint8_t NOP_DATA[] =
{
	0x90, 0x66,0x90, 0x66,0x66,0x90, 0x0F,0x1F,0x40,0x00, 0x0F,0x1F,0x44,0x00,0x00, 0x66,0x0F,0x1F,0x44,0x00,0x00, 0x0F,0x1F,0x80,0x00,0x00,0x00,0x00,
	0x0F,0x1F,0x84,0x00,0x00,0x00,0x00,0x00, 0x66,0x0F,0x1F,0x84,0x00,0x00,0x00,0x00,0x00, 0x66,0x66,0x0F,0x1F,0x84,0x00,0x00,0x00,0x00,0x00,
	0x66,0x66,0x66,0x0F,0x1F,0x84,0x00,0x00,0x00,0x00,0x00, 0x66,0x66,0x66,0x66,0x0F,0x1F,0x84,0x00,0x00,0x00,0x00,0x00, 0x66,0x66,0x66,0x66,0x66,0x0F,0x1F,0x84,0x00,0x00,0x00,0x00,0x00,
	0x66,0x66,0x66,0x66,0x66,0x66,0x0F,0x1F,0x84,0x00,0x00,0x00,0x00,0x00, 0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x0F,0x1F,0x84,0x00,0x00,0x00,0x00,0x00
};
static constexpr const uint8_t* const NOPX[] = { NOP_DATA, NOP_DATA + 1, NOP_DATA + 3, NOP_DATA + 6, NOP_DATA + 10, NOP_DATA + 15, NOP_DATA + 21, NOP_DATA + 28, NOP_DATA + 36, NOP_DATA + 45, NOP_DATA + 55, NOP_DATA + 66, NOP_DATA + 78, NOP_DATA + 91, NOP_DATA + 105 };

template<size_t OutLen>
alignas(64) static constexpr uint64_t c_blake2b_IV_init[8] = { 0x6a09e667f3bcc908ULL ^ (0x01010000ULL | OutLen), 0xbb67ae8584caa73bULL, 0x3c6ef372fe94f82bULL, 0xa54ff53a5f1d36f1ULL, 0x510e527fade682d1ULL, 0x9b05688c2b3e6c1fULL, 0x1f83d9abfb41bd6bULL, 0x5be0cd19137e2179ULL };

alignas(64) static constexpr uint8_t opcode_handler_lut[256] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
	5,5,5,5, 6,6,6,6, 7, 8,8,8,8, 9, 10,10,10,10,10,10,10,10, 11,11, 12,12,12,12,12,12,12,12,12,12,12,12,12,12,12, 13,13,13,13,13,
	14,14,14,14,14,14,14,14, 15,15, 16,16,16,16, 17,17,17,17, 18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18, 19,19,19,19,19,
	20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20, 21,21,21,21,21, 22,22,22,22,22,22, 23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
	23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23, 24,24,24,24, 25,25,25,25,25,25, 26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,
	26,26,26,26,26,26,26,26,26, 27, 28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28
};

struct alignas(64) randomx_vm
{
	uint64_t entropy[16];
	uint64_t instructions[256];
	uint64_t r[8];
	__m128d  f[4];
	__m128d  e[4];
	__m128d  a[4];
	__m128i  eMask;
	alignas(64) uint64_t tempHash[8];
	uint32_t mx, ma;
	uint8_t* memory;
	uint8_t* scratchpad;
	uint8_t* jitCode;
};

struct alignas(64) Blake2Generator
{
	uint8_t data[128];
	int index;
};

static uint8_t* g_cacheMemory;
static uint8_t* g_cacheJitCode;
static uint8_t* g_datasetMemory;

[[clang::always_inline]] static uint64_t randomx_reciprocal(const uint32_t divisor) noexcept
{
	[[assume((divisor & (divisor - 1u)) != 0u)]];
	const uint32_t shift = 32u - __builtin_clz(divisor);
	uint64_t quotient, dummy;
	__asm__("divq %4"
		: "=a"(quotient), "=d"(dummy)
		: "d"(1ULL << (shift - 1u)), "a"(0ULL), "r"(static_cast<uint64_t>(divisor)));
	return quotient;
}

template<bool G2>
static void G_AVX2(__m256i& __restrict A0, __m256i& __restrict A1, __m256i& __restrict B0, __m256i& __restrict B1, __m256i& __restrict C0, __m256i& __restrict C1, __m256i& __restrict D0, __m256i& __restrict D1) noexcept
{
	const __m256i pA0 = _mm256_mul_epu32(A0, B0);
	const __m256i pA1 = _mm256_mul_epu32(A1, B1);
	A0 = _mm256_add_epi64(_mm256_add_epi64(A0, B0), _mm256_add_epi64(pA0, pA0));
	A1 = _mm256_add_epi64(_mm256_add_epi64(A1, B1), _mm256_add_epi64(pA1, pA1));

	if constexpr (G2)
	{
		const __m256i rotr16 = _mm256_set_epi8(9, 8, 15, 14, 13, 12, 11, 10, 1, 0, 7, 6, 5, 4, 3, 2, 9, 8, 15, 14, 13, 12, 11, 10, 1, 0, 7, 6, 5, 4, 3, 2);
		D0 = _mm256_shuffle_epi8(_mm256_xor_si256(D0, A0), rotr16);
		D1 = _mm256_shuffle_epi8(_mm256_xor_si256(D1, A1), rotr16);
	}
	else
	{
		D0 = _mm256_shuffle_epi32(_mm256_xor_si256(D0, A0), _MM_SHUFFLE(2, 3, 0, 1));
		D1 = _mm256_shuffle_epi32(_mm256_xor_si256(D1, A1), _MM_SHUFFLE(2, 3, 0, 1));
	}

	const __m256i pC0 = _mm256_mul_epu32(C0, D0);
	const __m256i pC1 = _mm256_mul_epu32(C1, D1);
	C0 = _mm256_add_epi64(_mm256_add_epi64(C0, D0), _mm256_add_epi64(pC0, pC0));
	C1 = _mm256_add_epi64(_mm256_add_epi64(C1, D1), _mm256_add_epi64(pC1, pC1));

	if constexpr (G2)
	{
		const __m256i xB0 = _mm256_xor_si256(B0, C0);
		const __m256i xB1 = _mm256_xor_si256(B1, C1);
		B0 = _mm256_xor_si256(_mm256_srli_epi64(xB0, 63), _mm256_add_epi64(xB0, xB0));
		B1 = _mm256_xor_si256(_mm256_srli_epi64(xB1, 63), _mm256_add_epi64(xB1, xB1));
	}
	else
	{
		const __m256i rotr24 = _mm256_set_epi8(10, 9, 8, 15, 14, 13, 12, 11, 2, 1, 0, 7, 6, 5, 4, 3, 10, 9, 8, 15, 14, 13, 12, 11, 2, 1, 0, 7, 6, 5, 4, 3);
		B0 = _mm256_shuffle_epi8(_mm256_xor_si256(B0, C0), rotr24);
		B1 = _mm256_shuffle_epi8(_mm256_xor_si256(B1, C1), rotr24);
	}
}

template<bool Fwd>
static void DIAG2(__m256i& __restrict B0, __m256i& __restrict B1, __m256i& __restrict C0, __m256i& __restrict C1, __m256i& __restrict D0, __m256i& __restrict D1) noexcept
{
	const __m256i bCC = _mm256_blend_epi32(B0, B1, 0xCC);
	const __m256i b33 = _mm256_blend_epi32(B0, B1, 0x33);
	if constexpr (Fwd)
	{
		B1 = _mm256_permute4x64_epi64(bCC, _MM_SHUFFLE(2, 3, 0, 1)); B0 = _mm256_permute4x64_epi64(b33, _MM_SHUFFLE(2, 3, 0, 1));
	}
	else
	{
		B0 = _mm256_permute4x64_epi64(bCC, _MM_SHUFFLE(2, 3, 0, 1)); B1 = _mm256_permute4x64_epi64(b33, _MM_SHUFFLE(2, 3, 0, 1));
	}
	__m256i ct = C0; C0 = C1; C1 = ct;
	const __m256i d0orig = D0;
	D0 = _mm256_permute4x64_epi64(_mm256_blend_epi32(d0orig, D1, Fwd ? 0xCC : 0x33), _MM_SHUFFLE(2, 3, 0, 1));
	D1 = _mm256_permute4x64_epi64(_mm256_blend_epi32(d0orig, D1, Fwd ? 0x33 : 0xCC), _MM_SHUFFLE(2, 3, 0, 1));
}

template<typename T>
[[clang::always_inline]] static void store([[clang::noescape]] uint8_t* __restrict p, const T v) noexcept
{
	__builtin_memcpy(p, &v, sizeof(T));
}

[[gnu::cold]] static void b2gRefill([[clang::noescape]] Blake2Generator& gen) noexcept
{
	alignas(32) __m256i h[2];
	h[0] = _mm256_load_si256(reinterpret_cast<const __m256i*>(c_blake2b_IV_init<64>));
	h[1] = _mm256_load_si256(reinterpret_cast<const __m256i*>(c_blake2b_IV_init<64>) + 1);
	blake2b_compress_fin(reinterpret_cast<uint64_t*>(h), gen.data, 64);
	_mm256_store_si256(reinterpret_cast<__m256i*>(gen.data), h[0]);
	_mm256_store_si256(reinterpret_cast<__m256i*>(gen.data) + 1, h[1]);
	gen.index = 0;
}

[[nodiscard]] static uint8_t b2gGetByte([[clang::noescape]] Blake2Generator& gen) noexcept
{
	if (gen.index >= 64) [[unlikely]] b2gRefill(gen);
	return gen.data[gen.index++];
}

[[nodiscard]] static uint32_t b2gGetUInt32([[clang::noescape]] Blake2Generator& gen) noexcept
{
	if (gen.index > 60) [[unlikely]] b2gRefill(gen);
	gen.index += 4;
	uint32_t v; __builtin_memcpy(&v, gen.data + gen.index - 4, 4); return v;
}

struct SSInst
{
	int8_t   type, resultOp, dstOp, srcOp, opGroup, src, dst;
	uint8_t  opsCount, mod;
	SSMop    ops[3];
	uint32_t imm32;
	int32_t  opGroupPar;
	bool     canReuse, groupParIsSource;
};

[[nodiscard]] static SSInst createSuperscalar(const SSInfo& inf, [[clang::noescape]] Blake2Generator& gen) noexcept
{
	static constexpr uint8_t ssi_props[14] = { 50, 49, 50, 51, 4, 5, 6, 5, 6, 5, 6, 75, 76, 13 };
	SSInst inst;
	inst.type = inf.type; inst.resultOp = inf.resultOp; inst.dstOp = inf.dstOp; inst.srcOp = inf.srcOp;
	inst.opsCount = inf.opsCount; inst.ops[0] = inf.ops[0]; inst.ops[1] = inf.ops[1]; inst.ops[2] = inf.ops[2];
	inst.src = inst.dst = -1; inst.mod = 0; inst.imm32 = 0;
	const uint8_t p = ssi_props[inf.type];
	inst.opGroup = p & 15u;
	inst.opGroupPar = (p & 16u) ? 0 : -1;
	inst.groupParIsSource = (p & 32u) != 0;
	inst.canReuse = (p & 64u) != 0;

	switch (inf.type)
	{
	case 2: inst.mod = b2gGetByte(gen); break;
	case 4: do
	{
		inst.imm32 = b2gGetByte(gen) & 63u;
	}
	while (inst.imm32 == 0u); break;
	case 5: case 6: case 7: case 8: case 9: case 10: inst.imm32 = b2gGetUInt32(gen); break;
	case 11: case 12: inst.opGroupPar = static_cast<int32_t>(b2gGetUInt32(gen)); break;
	case 13: do
	{
		inst.imm32 = b2gGetUInt32(gen);
	}
	while ((inst.imm32 & (inst.imm32 - 1u)) == 0u); break;
	}
	return inst;
}

template<bool UseXor>
[[gnu::nonnull(1, 2, 3), gnu::access(read_write, 1), gnu::access(read_only, 2), gnu::access(read_write, 3)]]
static void fill_block([[clang::noescape]] __m256i* __restrict state, [[clang::noescape]] const __m256i* __restrict ref_block, [[clang::noescape]] __m256i* __restrict next_block) noexcept
{
	[[assume(reinterpret_cast<uintptr_t>(state) % 64 == 0)]];
	[[assume(reinterpret_cast<uintptr_t>(ref_block) % 64 == 0)]];
	[[assume(reinterpret_cast<uintptr_t>(next_block) % 64 == 0)]];
	alignas(64) __m256i temp_next[32];
	auto pass1 = [&](const int off)
		{
			__m256i s[8];
			for (int i = 0; i < 8; ++i)
			{
				s[i] = _mm256_xor_si256(state[off + i], _mm256_load_si256(ref_block + off + i));
				if constexpr (UseXor) temp_next[off + i] = _mm256_xor_si256(s[i], _mm256_load_si256(next_block + off + i));
				else temp_next[off + i] = s[i];
			}
			G_AVX2<false>(s[0], s[4], s[1], s[5], s[2], s[6], s[3], s[7]);
			G_AVX2<true>(s[0], s[4], s[1], s[5], s[2], s[6], s[3], s[7]);
			s[1] = _mm256_permute4x64_epi64(s[1], _MM_SHUFFLE(0, 3, 2, 1));
			s[2] = _mm256_permute4x64_epi64(s[2], _MM_SHUFFLE(1, 0, 3, 2));
			s[3] = _mm256_permute4x64_epi64(s[3], _MM_SHUFFLE(2, 1, 0, 3));
			s[5] = _mm256_permute4x64_epi64(s[5], _MM_SHUFFLE(0, 3, 2, 1));
			s[6] = _mm256_permute4x64_epi64(s[6], _MM_SHUFFLE(1, 0, 3, 2));
			s[7] = _mm256_permute4x64_epi64(s[7], _MM_SHUFFLE(2, 1, 0, 3));
			G_AVX2<false>(s[0], s[4], s[1], s[5], s[2], s[6], s[3], s[7]);
			G_AVX2<true>(s[0], s[4], s[1], s[5], s[2], s[6], s[3], s[7]);
			s[1] = _mm256_permute4x64_epi64(s[1], _MM_SHUFFLE(2, 1, 0, 3));
			s[2] = _mm256_permute4x64_epi64(s[2], _MM_SHUFFLE(1, 0, 3, 2));
			s[3] = _mm256_permute4x64_epi64(s[3], _MM_SHUFFLE(0, 3, 2, 1));
			s[5] = _mm256_permute4x64_epi64(s[5], _MM_SHUFFLE(2, 1, 0, 3));
			s[6] = _mm256_permute4x64_epi64(s[6], _MM_SHUFFLE(1, 0, 3, 2));
			s[7] = _mm256_permute4x64_epi64(s[7], _MM_SHUFFLE(0, 3, 2, 1));
			for (int i = 0; i < 8; ++i) state[off + i] = s[i];
		};
	pass1(0); pass1(8); pass1(16); pass1(24);
	auto pass2 = [&](const int off)
		{
			__m256i s[8] = { state[off], state[off + 4], state[off + 8], state[off + 12], state[off + 16], state[off + 20], state[off + 24], state[off + 28] };
			G_AVX2<false>(s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7]);
			G_AVX2<true>(s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7]);
			DIAG2<true>(s[2], s[3], s[4], s[5], s[6], s[7]);
			G_AVX2<false>(s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7]);
			G_AVX2<true>(s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7]);
			DIAG2<false>(s[2], s[3], s[4], s[5], s[6], s[7]);
			auto mix = [&](__m256i& reg, int idx)
				{
					reg = _mm256_xor_si256(reg, temp_next[idx]);
					if constexpr (UseXor) _mm256_store_si256(next_block + idx, reg);
					else _mm256_stream_si256(next_block + idx, reg);
					state[idx] = reg;
				};
			mix(s[0], off); mix(s[1], off + 4); mix(s[2], off + 8); mix(s[3], off + 12);
			mix(s[4], off + 16); mix(s[5], off + 20); mix(s[6], off + 24); mix(s[7], off + 28);
		};
	pass2(0); pass2(1); pass2(2); pass2(3);
}

template<bool Pass0, int Slice>
[[gnu::const]] static uint32_t rxa2_index_alpha_t(const uint32_t index, const uint32_t pseudo_rand) noexcept
{
	const uint32_t reference_area_size = Pass0 ? (Slice == 0 ? index - 1u : Slice * 65536u + index - 1u) : 196608u + index - 1u;
	const uint32_t sq_hi = (static_cast<uint64_t>(pseudo_rand) * pseudo_rand) >> 32u;
	const uint32_t ras_hi = (static_cast<uint64_t>(reference_area_size) * sq_hi) >> 32u;
	constexpr uint32_t start = Pass0 ? 0u : (Slice == 3 ? 0u : (Slice + 1) * 65536u);
	const uint32_t r = (start + reference_area_size - 1u - ras_hi) & 262143u;
	[[assume(r < 262144u)]];
	return r;
}

template<bool UseXor, int Slice>
[[clang::preserve_none]] static void rxa2_fill_segment_impl() noexcept
{
	uint8_t* const __restrict memory = static_cast<uint8_t*>(__builtin_assume_aligned(g_cacheMemory, 64));
	constexpr uint32_t start_idx = (!UseXor && (Slice == 0)) ? 2 : 0;
	constexpr uint32_t initial_curr = Slice * 65536 + start_idx;
	constexpr uint32_t initial_prev = (initial_curr == 0) ? 262143 : initial_curr - 1;
	constexpr int pf_hint = UseXor ? _MM_HINT_NTA : _MM_HINT_T1;
	alignas(64) __m256i state[32];
	const __m256i* const prev_block = reinterpret_cast<const __m256i*>(memory + initial_prev * 1024);
	for (int _k = 0; _k < 32; ++_k) state[_k] = _mm256_load_si256(prev_block + _k);
	uint32_t next_pseudo_rand = *reinterpret_cast<const uint32_t*>(memory + initial_prev * 1024);
	{
		const char* const __restrict fp = reinterpret_cast<const char*>(memory + rxa2_index_alpha_t<!UseXor, Slice>(start_idx, next_pseudo_rand) * 1024);
		for (int p = 0; p < 1024; p += 64) _mm_prefetch(fp + p, pf_hint);
	}
	uint32_t curr_offset = initial_curr;
	for (uint32_t i = start_idx; i < 65535; ++i, ++curr_offset)
	{
		[[assume(curr_offset < 262144)]];
		fill_block<UseXor>(state, reinterpret_cast<__m256i*>(memory + rxa2_index_alpha_t<!UseXor, Slice>(i, next_pseudo_rand) * 1024), reinterpret_cast<__m256i*>(memory + curr_offset * 1024));
		next_pseudo_rand = static_cast<uint32_t>(_mm_cvtsi128_si32(_mm256_castsi256_si128(state[0])));
		const char* const __restrict np = reinterpret_cast<const char*>(memory + rxa2_index_alpha_t<!UseXor, Slice>(i + 1, next_pseudo_rand) * 1024);
		for (int p = 0; p < 1024; p += 64) _mm_prefetch(np + p, pf_hint);
	}

	[[assume(curr_offset < 262144)]];
	fill_block<UseXor>(state, reinterpret_cast<__m256i*>(memory + rxa2_index_alpha_t<!UseXor, Slice>(65535, next_pseudo_rand) * 1024), reinterpret_cast<__m256i*>(memory + curr_offset * 1024));
}

template<bool Commit>
[[nodiscard, clang::preserve_none]] static int scheduleMop_impl(const SSMop mop, [[clang::noescape]] uint8_t* __restrict portBusy, int cycle, const int depCycle) noexcept
{
	if (mop.dep && depCycle > cycle) cycle = depCycle;
	if (mop.uop1 == 0) return cycle;
	for (; cycle < 174; ++cycle)
	{
		const unsigned free = ~portBusy[cycle] & 7u;
		const unsigned can1 = free & mop.uop1;
		if (mop.uop2 == 0)
		{
			if (!can1) continue;
			if constexpr (Commit) portBusy[cycle] |= (can1 & 4) ? 4 : (can1 & -can1);
			return cycle;
		}
		const unsigned can2 = free & mop.uop2;
		if (!can1 || !can2) continue;
		if constexpr (Commit) portBusy[cycle] |= (can1 & -can1) | (can2 & -can2);
		return cycle;
	}
	return -1;
}

template<bool rax = true, bool St = false>
[[clang::always_inline, gnu::nonnull(1)]] static void genAddressRegWP([[clang::noescape]] uint8_t* __restrict& __restrict wp, const uint32_t src, const uint32_t mod, const uint32_t imm32) noexcept
{
	store<uint32_t>(wp, 604015937u | (((rax ? 128u : 136u) + src) << 16));
	store<uint32_t>(wp + 3 + (src == 4), imm32); wp += 7 + (src == 4);
	const uint32_t mask = St && (mod >> 4) >= 14 ? 2097144u : (mod & 3) ? 16376u : 262136u;
	if constexpr (rax)
	{
		store<uint64_t>(wp, 37 | (mask << 8)); wp += 5;
	}
	else
	{
		store<uint64_t>(wp, 57729u | (static_cast<uint64_t>(mask) << 16)); wp += 6;
	}
}

template<uint32_t Op>
[[clang::always_inline]] static void emit_alu_reg_mem([[clang::noescape]] uint8_t* __restrict& __restrict wp, const uint32_t src, const uint32_t dst, const uint32_t mod, const uint32_t imm32, const uint32_t dst_scaled) noexcept
{
	if (src != dst) [[likely]]
	{
		genAddressRegWP(wp, src, mod, imm32);
		store<uint32_t>(wp, 100663372u | (Op << 8) | ((4 + dst_scaled) << 16)); wp += 4;
	}
	else
	{
		store<uint64_t>(wp, 76u | (Op << 8) | ((134 + dst_scaled) << 16) | (static_cast<uint64_t>(imm32 & 2097144u) << 24)); wp += 7;
	}
}

template<uint64_t Sel>
[[clang::always_inline]] static void emit_group3_mem([[clang::noescape]] uint8_t* __restrict& __restrict wp, const uint32_t src, const uint32_t dst, const uint32_t mod, const uint32_t imm32, const uint32_t dst_scaled) noexcept
{
	if (src != dst) [[likely]]
	{
		genAddressRegWP<false>(wp, src, mod, imm32);
		store<uint64_t>(wp, 5480318858621389641ULL | ((192 + dst) << 16) | (Sel << 40));
		store<uint16_t>(wp + 8, 139u | ((194 + dst_scaled) << 8)); wp += 10;
	}
	else
	{
		store<uint64_t>(wp, 1062064917321ULL | ((192 + dst) << 16) | ((Sel + 130) << 40));
		store<uint32_t>(wp + 6, imm32 & 2097144u);
		store<uint32_t>(wp + 10, 35660u | ((194 + dst_scaled) << 16)); wp += 13;
	}
}

template<uint64_t Base>
[[clang::always_inline]] static void emit_rotate([[clang::noescape]] uint8_t* __restrict& __restrict wp, const uint32_t src, const uint32_t dst, const uint32_t imm32) noexcept
{
	if (src != dst) [[likely]]
	{
		store<uint64_t>(wp, 907462871873ULL | ((200 + src) << 16) | ((Base + dst) << 40)); wp += 6;
	}
	else
	{
		store<uint32_t>(wp, 49481u | ((Base + dst) << 16) | ((imm32 & 63) << 24)); wp += 4;
	}
}

[[nodiscard, gnu::returns_nonnull]] static uint8_t* createJitCode() noexcept
{
	uint8_t* code = static_cast<uint8_t*>(VirtualAlloc(nullptr, 77824, MEM_COMMIT | MEM_RESERVE, 0x40));
	[[assume(code != nullptr)]];
	__builtin_memcpy(code, randomx_program_prologue, randomx_size_prologue);
	__builtin_memcpy(code + randomx_size_prologue, randomx_program_loop_load, randomx_size_loopload);
	__builtin_memcpy(code + (77824 - randomx_size_epilogue), randomx_program_epilogue, randomx_size_epilogue);
	return code;
}

[[clang::always_inline]] static void inline_copy_jit([[clang::noescape]] uint8_t* __restrict dst, const uint8_t* __restrict src, int32_t n) noexcept
{
	int32_t off = 0;
	while (off < n)
	{
		_mm256_storeu_si256(reinterpret_cast<__m256i*>(dst + off), _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src + off)));
		off += 32;
	}
}

[[gnu::hot, clang::preserve_none, gnu::nonnull(1)]]
static void randomx_vm_run(randomx_vm* __restrict machine) noexcept
{
	[[assume(g_datasetMemory != nullptr)]];
	[[assume(machine->scratchpad != nullptr)]];
	[[assume(reinterpret_cast<uintptr_t>(machine->scratchpad) % 64 == 0)]];
	[[assume(reinterpret_cast<uintptr_t>(g_datasetMemory) % 64 == 0)]];
	fillAes4Rx4_hw(machine);

	const uint64_t* const entropy = machine->entropy;
	const __m256i e0 = _mm256_load_si256(reinterpret_cast<const __m256i*>(entropy));
	const __m256i e1 = _mm256_load_si256(reinterpret_cast<const __m256i*>(entropy) + 1);
	_mm256_store_si256(reinterpret_cast<__m256i*>(machine->a), _mm256_or_si256(_mm256_slli_epi64(_mm256_add_epi64(_mm256_srli_epi64(e0, 59), _mm256_set1_epi64x(1023)), 52), _mm256_and_si256(e0, _mm256_set1_epi64x(4503599627370495))));
	_mm256_store_si256(reinterpret_cast<__m256i*>(machine->a) + 1, _mm256_or_si256(_mm256_slli_epi64(_mm256_add_epi64(_mm256_srli_epi64(e1, 59), _mm256_set1_epi64x(1023)), 52), _mm256_and_si256(e1, _mm256_set1_epi64x(4503599627370495))));
	const __m128i e14_15 = _mm_load_si128(reinterpret_cast<const __m128i*>(entropy + 14));

	machine->eMask = _mm_or_si128(_mm_or_si128(_mm_slli_epi64(_mm_srli_epi64(e14_15, 60), 56), _mm_set1_epi64x(3458764513820540928)), _mm_and_si128(e14_15, _mm_set1_epi64x(4194303)));
	machine->ma = entropy[8] & 2147483584;
	machine->mx = entropy[10];
	machine->memory = g_datasetMemory + (entropy[13] & 524287) * 64;
	const int ar = entropy[12] & 15;
	alignas(64) uint16_t instrOffsets[256];
	alignas(64) int16_t registerUsage[8];
	_mm_store_si128(reinterpret_cast<__m128i*>(registerUsage), _mm_set1_epi16(-1));
	uint8_t* const __restrict jit = machine->jitCode;
	uint8_t* __restrict wp = jit + (randomx_size_prologue + randomx_size_loopload);
	const uint64_t* const instructions = machine->instructions;

	for (int i = 0; i < 256; ++i)
	{
		const uint64_t raw_instr = instructions[i];
		const uint32_t imm32 = raw_instr >> 32;
		const uint32_t lo = static_cast<uint32_t>(raw_instr);
		const int mod = (lo >> 24) & 255;
		const int src = (lo >> 16) & 7;
		const int dst = (lo >> 8) & 7;
		const uint8_t op = opcode_handler_lut[lo & 255];
		[[assume(op <= 28)]];
		const int dst_scaled = dst * 8;
		const int vec_dst_idx = dst & 3;
		const int fpu_dst_scaled = vec_dst_idx * 8;
		instrOffsets[i] = static_cast<uint16_t>(wp - jit);

		switch (op)
		{
		case 0: registerUsage[dst] = i; store<uint64_t>(wp, 36175u | (((4 + dst_scaled) | (dst == 5 ? 128 : 0)) << 16) | ((((mod & 12) << 4) | (src * 8) | dst) << 24) | (static_cast<uint64_t>(imm32) << 32)); wp += 4 + ((dst == 5) << 2); continue;
		case 1: registerUsage[dst] = i; emit_alu_reg_mem<3>(wp, src, dst, mod, imm32, dst_scaled); continue;
		case 2: registerUsage[dst] = i; if (src != dst) [[likely]]
		{
			store<uint32_t>(wp, 11085u | ((192 + dst_scaled + src) << 16)); wp += 3;
		}
			  else
		{
			store<uint64_t>(wp, 33097u | ((232 + dst) << 16) | (static_cast<uint64_t>(imm32) << 24)); wp += 7;
		} continue;
		case 3: registerUsage[dst] = i; emit_alu_reg_mem<43>(wp, src, dst, mod, imm32, dst_scaled); continue;
		case 4: registerUsage[dst] = i; if (src != dst) [[likely]]
		{
			store<uint32_t>(wp, 11472717u | ((192 + dst_scaled + src) << 24)); wp += 4;
		}
			  else
		{
			store<uint64_t>(wp, 26957u | ((192 + dst_scaled + dst) << 16) | (static_cast<uint64_t>(imm32) << 24)); wp += 7;
		} continue;
		case 5: registerUsage[dst] = i; if (src != dst) [[likely]]
		{
			genAddressRegWP(wp, src, mod, imm32); store<uint64_t>(wp, 25781276492ULL | ((4 + dst_scaled) << 24)); wp += 5;
		}
			  else
		{
			store<uint64_t>(wp, 11472716u | ((134ULL + dst_scaled) << 24) | (static_cast<uint64_t>(imm32 & 2097144) << 32)); wp += 8;
		} continue;
		case 6: registerUsage[dst] = i; store<uint64_t>(wp, 69519108445604681ULL | ((208 + dst) << 16) | (static_cast<uint64_t>(192 | dst_scaled | src) << 56)); wp += 8; continue;
		case 7: registerUsage[dst] = i; emit_group3_mem<36>(wp, src, dst, mod, imm32, dst_scaled); continue;
		case 8: registerUsage[dst] = i; store<uint64_t>(wp, 10037398731583687497ULL | ((192 + dst) << 16) | ((232ULL + src) << 40)); wp[8] = 194 + dst_scaled; wp += 9; continue;
		case 9: registerUsage[dst] = i; emit_group3_mem<44>(wp, src, dst, mod, imm32, dst_scaled); continue;
		case 10: if (imm32 & (imm32 - 1)) [[likely]]
		{
			registerUsage[dst] = i; store<uint16_t>(wp, 47176u); store<uint64_t>(wp + 2, randomx_reciprocal(imm32)); store<uint32_t>(wp + 10, 11472716u | ((192 + dst_scaled) << 24)); wp += 14;
		} continue;
		case 11: registerUsage[dst] = i; store<uint32_t>(wp, 63305u | ((216 + dst) << 16)); wp += 3; continue;
		case 12: registerUsage[dst] = i; if (src != dst) [[likely]]
		{
			store<uint32_t>(wp, 13133u | ((192 + dst_scaled + src) << 16)); wp += 3;
		}
			   else
		{
			store<uint64_t>(wp, 33097u | ((240 + dst) << 16) | (static_cast<uint64_t>(imm32) << 24)); wp += 7;
		} continue;
		case 13: registerUsage[dst] = i; emit_alu_reg_mem<51>(wp, src, dst, mod, imm32, dst_scaled); continue;
		case 14: registerUsage[dst] = i; emit_rotate<200>(wp, src, dst, imm32); continue;
		case 15: registerUsage[dst] = i; emit_rotate<192>(wp, src, dst, imm32); continue;
		case 16: if (src != dst) [[likely]]
		{
			registerUsage[dst] = i; registerUsage[src] = i; store<uint32_t>(wp, 34637u | ((192 + src + dst_scaled) << 16)); wp += 3;
		} continue;
		case 17: store<uint64_t>(wp, 4307943621ULL | ((249 - dst_scaled) << 8) | ((192u + dst_scaled + dst) << 24)); wp += 5; continue;
		case 18: store<uint64_t>(wp, 1476444612ULL | ((121 - fpu_dst_scaled) << 16) | ((192ULL + (src & 3) + fpu_dst_scaled) << 32)); wp += 5; continue;
		case 19: genAddressRegWP(wp, src, mod, imm32); store<uint64_t>(wp, 13962291500310749636ULL); store<uint32_t>(wp + 8, (22649 - fpu_dst_scaled) | ((196 + fpu_dst_scaled) << 16)); wp += 11; continue;
		case 20: store<uint64_t>(wp, 1543553476ULL | ((121 - fpu_dst_scaled) << 16) | ((192ULL + (src & 3) + fpu_dst_scaled) << 32)); wp += 5; continue;
		case 21: genAddressRegWP(wp, src, mod, imm32); store<uint64_t>(wp, 13962291500310749636ULL); store<uint32_t>(wp + 8, (23673 - fpu_dst_scaled) | ((196 + fpu_dst_scaled) << 16)); wp += 11; continue;
		case 22: store<uint64_t>(wp, 1459667396ULL | ((121 - fpu_dst_scaled) << 16) | ((199ULL + fpu_dst_scaled) << 32)); wp += 5; continue;
		case 23: [[likely]] store<uint64_t>(wp, 1493221828ULL | ((89 - fpu_dst_scaled) << 16) | ((224ULL + (src & 3) + fpu_dst_scaled) << 32)); wp += 5; continue;
		case 24: genAddressRegWP(wp, src, mod, imm32); store<uint64_t>(wp, 4738919463455973828ULL); store<uint64_t>(wp + 8, 16597481246987473945ULL); store<uint64_t>(wp + 16, 1577107908ULL | ((89 - fpu_dst_scaled) << 16) | ((228ULL + fpu_dst_scaled) << 32)); wp += 21; continue;
		case 25: store<uint32_t>(wp, 5372357 | ((228 + fpu_dst_scaled + vec_dst_idx) << 24)); wp += 4; continue;
		case 26:
		{
			store<uint64_t>(wp, 4683743612465348937ULL | ((192 + dst) << 16) | (static_cast<uint64_t>((imm32 | (256 << (mod >> 4))) & ~(128 << (mod >> 4))) << 24));
			store<uint64_t>(wp + 8, 9515824537657147639ULL | ((192 + dst) << 8) | (4278190080ULL << (mod >> 4)));
			store<uint32_t>(wp + 16, instrOffsets[registerUsage[dst] + 1] - (wp - jit + 20));
			wp += 20; _mm_store_si128(reinterpret_cast<__m128i*>(registerUsage), _mm_set1_epi16(i)); continue;
		}
		case 27:
		{
			store<uint32_t>(wp, 35657u | ((192 + src) << 16)); wp += 3;
			const uint32_t rot = (13 - imm32) & 63;
			store<uint32_t>(wp, 12632392u | (rot << 24)); wp += (rot != 0) << 2;
			store<uint64_t>(wp, 11511214941216440357ULL); store<uint64_t>(wp + 8, 2599895268385095680ULL); wp[16] = 88; wp += 17; continue;
		}
		case 28: genAddressRegWP<true, true>(wp, dst, mod, imm32); store<uint32_t>(wp, 100698444u | ((4 + src * 8) << 16)); wp += 4; continue;
		}
	}

	store<uint64_t>(wp, 220133886785ULL | ((196 + ((ar >> 2) & 1)) << 16) | ((198ULL + ((ar >> 3) & 1)) << 40)); wp += 6;
	inline_copy_jit(wp, randomx_program_read_dataset, randomx_size_read_dataset); wp += randomx_size_read_dataset;
	store<uint64_t>(wp, 220268104521ULL | ((192 + (ar & 1)) << 16) | ((194ULL + ((ar >> 1) & 1)) << 40)); wp += 6;
	inline_copy_jit(wp, randomx_prefetch_scratchpad, randomx_size_prefetch_scratchpad + randomx_size_loopstore); wp += randomx_size_prefetch_scratchpad + randomx_size_loopstore;
	const uint32_t codePos = wp - jit;
	store<uint64_t>(wp, 2232404991u | (static_cast<uint64_t>(randomx_size_prologue - (codePos + 8)) << 32)); wp[8] = 233;
	store<uint32_t>(wp + 9, (77824 - randomx_size_epilogue) - (codePos + 13));
	reinterpret_cast<void(*)(randomx_vm * __restrict) noexcept>(jit)(machine);
}

[[gnu::hot, clang::preserve_none, gnu::nonnull(1)]]
static void randomx_calculate_hash_first(randomx_vm* __restrict machine, const void* __restrict input, const uint8_t inputSize) noexcept
{
	[[assume(reinterpret_cast<uintptr_t>(machine) % 64 == 0)]];
	alignas(64) __m256i h[2];
	h[0] = _mm256_load_si256(reinterpret_cast<const __m256i*>(c_blake2b_IV_init<64>));
	h[1] = _mm256_load_si256(reinterpret_cast<const __m256i*>(c_blake2b_IV_init<64>) + 1);
	blake2b_compress_fin(reinterpret_cast<uint64_t*>(h), static_cast<const uint8_t*>(input), inputSize);
	_mm256_store_si256(reinterpret_cast<__m256i*>(machine->tempHash), h[0]);
	_mm256_store_si256(reinterpret_cast<__m256i*>(machine->tempHash) + 1, h[1]);
	fillAes1Rx4_hw(machine);
}

[[gnu::hot, clang::preserve_none, gnu::nonnull(1)]]
static void randomx_calculate_hash_next(randomx_vm* __restrict machine, const void* __restrict nextInput, const uint8_t nextInputSize, void* __restrict output) noexcept
{
	[[assume(reinterpret_cast<uintptr_t>(machine) % 64 == 0)]];
	_mm_setcsr(40896);
	const uint8_t* const reg_ptr = reinterpret_cast<const uint8_t*>(&machine->r);
	uint64_t* const temp_hash = machine->tempHash;
	for (int chain = 0; chain < 7; ++chain)
	{
		randomx_vm_run(machine);
		__builtin_memcpy(temp_hash, c_blake2b_IV_init<64>, 64);
		blake2b_compress_mid(temp_hash, reg_ptr);
		blake2b_compress_fin(temp_hash, reg_ptr + 128, 256);
	}
	randomx_vm_run(machine);
	__builtin_memcpy(temp_hash, c_blake2b_IV_init<64>, 64);
	blake2b_compress_fin(temp_hash, static_cast<const uint8_t*>(nextInput), nextInputSize);
	hashAndFillAes1Rx4_hw(machine);
	alignas(64) uint64_t h32[8];
	__builtin_memcpy(h32, c_blake2b_IV_init<32>, 64);
	blake2b_compress_mid(h32, reg_ptr);
	blake2b_compress_fin(h32, reg_ptr + 128, 256);
	__builtin_memcpy(output, h32, 32);
}