#include "ccv.h"
#include "case.h"

uint64_t uniqid()
{
	union {
		uint64_t u;
		uint8_t chr[8];
	} sign;
	int i;
	for (i = 0; i < 8; i++)
		sign.chr[i] = rand() & 0xff;
	return sign.u;
}

#define N (250000)

TEST_CASE("cache test")
{
	ccv_cache_t cache;
	ccv_cache_init(&cache);
	uint64_t sigs[N];
	void* mems[N];
	int i;
	for (i = 0; i < N; i++)
	{
		sigs[i] = uniqid();
		mems[i] = ccmalloc(1);
		ccv_cache_put(&cache, sigs[i], mems[i]);
	}
	uint8_t deleted[N];
	for (i = 0; i < N; i++)
	{
		deleted[i] = 1;
		if (deleted[i])
			ccv_cache_delete(&cache, sigs[i]);
	}
	for (i = 0; i < N; i++)
	{
		deleted[i] = (rand() % 3 == 0);
		if (!deleted[i])
		{
			mems[i] = ccmalloc(1);
			ccv_cache_put(&cache, sigs[i], mems[i]);
		}
	}
	for (i = 0; i < N; i++)
	{
		deleted[i] = (rand() % 3 == 0);
		if (deleted[i])
			ccv_cache_delete(&cache, sigs[i]);
		else {
			mems[i] = ccmalloc(1);
			ccv_cache_put(&cache, sigs[i], mems[i]);
		}
	}
	for (i = 0; i < N; i++)
	{
		ccv_matrix_t* x = ccv_cache_get(&cache, sigs[i]);
		if (!deleted[i])
		{
			REQUIRE((uint64_t)x, "at %d should exist", i);
			REQUIRE_EQ((uint64_t)mems[i], (uint64_t)x, "value at %d should be consistent", i);
		} else
	 		REQUIRE_EQ(0, (uint64_t)x, "at %d should not exist", i);
	}
	ccv_cache_close(&cache);
}

TEST_CASE("garbage collector test")
{
	int i;
	for (i = 0; i < N; i++)
	{
		ccv_dense_matrix_t* dmt = ccv_dense_matrix_new(1, 1, CCV_32S | CCV_C1, 0, 0);
		dmt->data.i[0] = i;
		dmt->sig = ccv_matrix_generate_signature((const char*)&i, 4, 0);
		dmt->type |= CCV_REUSABLE;
		ccv_matrix_free(dmt);
	}
	for (i = 0; i < N; i++)
	{
		uint64_t sig = ccv_matrix_generate_signature((const char*)&i, 4, 0);
		ccv_dense_matrix_t* dmt = ccv_dense_matrix_new(1, 1, CCV_32S | CCV_C1, 0, sig);
		REQUIRE_EQ(i, dmt->data.i[0], "should equal to labeled result");
		ccv_matrix_free(dmt);
	}
	ccv_garbage_collect();
}

#include "case_main.h"
